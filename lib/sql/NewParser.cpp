#include <algorithm>
#include <regex>
#include <string>
#include <vector>

#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/Value.h"
#include "llvm/ADT/SmallVector.h"

#include "sql/SQLDialect.h"
#include "sql/SQLOps.h"
#include "sql/SQLTypes.h"

// hsql stuff
#include "SQLParser.h"
#include "util/sqlhelper.h"

using namespace mlir;
using namespace sql;
using namespace hsql;

static const std::map<const hsql::OperatorType, const std::string> boolOperatorsToToken = {
      {kOpEquals, "="},      {kOpNotEquals, "!="},
      {kOpLess, "<"},        {kOpLessEq, "<="},
      {kOpGreater, ">"},     {kOpGreaterEq, ">="}};

static const std::map<const hsql::OperatorType, const std::string> arithOperatorsToToken = {
      {kOpPlus, "+"},        {kOpMinus, "-"},
      {kOpAsterisk, "*"},    {kOpSlash, "/"},
      {kOpPercentage, "%"},  {kOpCaret, "^"}};

mlir::Value parseLiteral(mlir::Location loc, mlir::OpBuilder &builder, hsql::Expr* expr){
    // if (expr->isType(hsql::kExprLiteralInt)) {
    //     return builder.create<IntOp>(loc, IntType::get(builder.getContext()), expr->ival).getResult();
    // } else if (expr->isType(hsql::kExprLiteralFloat)) {
    //     return builder.create<FloatOp>(loc, FloatType::get(builder.getContext()), expr->fval).getResult();
    // } else if (expr->isType(hsql::kExprLiteralString)) {
    //     return builder.create<ConstantStringOp>(loc, StringType::get(builder.getContext()), std::string(expr->name)).getResult();
    // } else {
    //     assert(false && "other literals not implemented");
    // }
    return nullptr;
}

mlir::Value parseClause(mlir::Location loc, mlir::OpBuilder &builder, hsql::Expr* expr) {
    if (expr == nullptr) {
        return nullptr;
    }
    if (expr->isType(hsql::kExprOperator)) {
        auto op = expr->opType;
        auto left = parseClause(loc, builder, expr->expr);
        if (expr->expr2 != nullptr) {
            auto right = parseClause(loc, builder, expr->expr2);
            if (op == hsql::kOpAnd) {
                return builder.create<AndOp>(loc, BoolType::get(builder.getContext()), left, right).getResult();
            } else if (op == hsql::kOpOr) {
                return builder.create<OrOp>(loc, BoolType::get(builder.getContext()), left, right).getResult();
            } else if (boolOperatorsToToken.find(op) != boolOperatorsToToken.cend()){ 
                const auto found = boolOperatorsToToken.find(op); 
                return builder.create<BoolArithOp>(loc, BoolType::get(builder.getContext()), left, right, found->second).getResult();
            } else if (arithOperatorsToToken.find(op) != arithOperatorsToToken.cend()){
                const auto found = arithOperatorsToToken.find(op);
                return builder.create<ArithOp>(loc, sql::ExprType::get(builder.getContext()), left, right, found->second).getResult();
            } else { 
                assert(false && "other operations not implemented");
            }
        } else if (expr->exprList != nullptr) { 
            assert(false && "other list operations not implemented");
        } else {
            assert(false && "other unitary operations not implemented");
        }
    } else if (expr->isLiteral()) { 
        // should they be an op or just constant
        return parseLiteral(loc, builder, expr);
    } else if (expr->isType(hsql::kExprColumnRef)) {
        return builder.create<ColumnOp>(loc, sql::ExprType::get(builder.getContext()), std::string(expr->name)).getResult();
    } else {
        assert(false && "other clauses not implemented");
    }
   
}


mlir::Value parseSQL(mlir::Location loc, mlir::OpBuilder &builder,
                     std::string query) {

  hsql::SQLParserResult result;
  hsql::SQLParser::parse(query, &result);
  auto statement = result.getStatement(0); 

  if (statement->isType(hsql::kStmtSelect)) {
    auto selectStatement = (const hsql::SelectStatement*) statement;
    llvm::SmallVector<Value> columns;
    
    // parse columns
    for (auto expr : *selectStatement->selectList) {
        mlir::Value columnOp = nullptr;
        if (expr->isType(hsql::kExprStar)) {
            columnOp = builder
                .create<AllColumnsOp>(loc, sql::ExprType::get(builder.getContext()))
                .getResult();
        } else if (expr->isType(hsql::kExprColumnRef)) { 
            columnOp = builder
              .create<ColumnOp>(loc, sql::ExprType::get(builder.getContext()),
                                std::string(expr->getName()))
              .getResult();
        } else if (expr->isLiteral()) {
            columnOp = parseLiteral(loc, builder, expr);
        } else { 
            assert(false && "other columns not implemented");
        }
        if (columnOp != nullptr)
            columns.push_back(columnOp);
    }

    // parse table
    auto fromTable = selectStatement->fromTable;
    mlir::Value tableOp = nullptr;
    if (fromTable == NULL) {
        tableOp = builder
            .create<TableOp>(loc, sql::ExprType::get(builder.getContext()),
                             std::string(""))
            .getResult();
    } else if (fromTable != NULL && fromTable->type == hsql::kTableName) {
        tableOp = builder
            .create<TableOp>(loc, sql::ExprType::get(builder.getContext()),
                             std::string(fromTable->getName()))
            .getResult();
    } else {
        assert (false && "other tables not implemented");
    }

    // parse limit
    int64_t limit = -1;
    if (selectStatement->limit != NULL) {
        auto selectLimit = selectStatement->limit;
        if (selectLimit->limit != NULL && selectLimit->limit->isType(hsql::kExprLiteralInt)) {
            limit = selectLimit->limit->ival;
            // auto limit = builder
            //     .create<LimitOp>(loc, LimitType::get(builder.getContext()), value)
            //     .getResult();
        } else {
            assert(false && "other limits not implemented");
        }
    }
    // parse where clause
    mlir::Value whereOp = nullptr; 
    if (selectStatement->whereClause != NULL) {
        auto whereClause = selectStatement->whereClause;
        auto whereOp = parseClause(loc, builder, whereClause);
    }
    
    return builder.create<sql::SelectOp>(loc, sql::ExprType::get(builder.getContext()),
                                     columns, tableOp, whereOp, limit).getResult();
  }

}
