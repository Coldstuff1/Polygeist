// RUN: polygeist-opt --sql-lower --sql-raising --allow-unregistered-dialect %s | FileCheck %s

module {
  func.func private @run() -> i32 {
    %c0 = arith.constant 0 : index

    %col = "sql.column"() {expr = "data"} : () -> !sql.expr
    %tbl = "sql.table"() {expr = "mytable"} : () -> !sql.expr
    %bool = "sql.calc_bool"() {expr = "1 = 1"} : () -> !sql.bool
    %where = "sql.where"(%bool) : (!sql.bool) -> !sql.expr

    %q = "sql.select"(%col, %tbl, %where) {limit = 1 : si64} : (!sql.expr, !sql.expr, !sql.expr) -> !sql.expr

    %h = "sql.execute"(%c0, %q) : (index, !sql.expr) -> index
    %res = "sql.get_value"(%h, %c0, %c0) : (index, index, index) -> i32

    return %res : i32
  }
}

// CHECK-LABEL: func.func private @run() -> i32 {
// CHECK: %[[C0:.+]] = arith.constant 0 : index
// CHECK: %[[COL:.+]] = "sql.column"() {expr = "data"} : () -> !sql.expr
// CHECK: %[[TBL:.+]] = "sql.table"() {expr = "mytable"} : () -> !sql.expr
// CHECK: %[[BOOL:.+]] = "sql.calc_bool"() {expr = "1 = 1"} : () -> !sql.bool
// CHECK: %[[WHERE:.+]] = "sql.where"(%[[BOOL]]) : (!sql.bool) -> !sql.expr
// CHECK: %[[Q:.+]] = "sql.select"(%[[COL]], %[[TBL]], %[[WHERE]]) {limit = 1 : si64} : (!sql.expr, !sql.expr, !sql.expr) -> !sql.expr
// CHECK: %[[H:.+]] = "sql.execute"(%[[C0]], %[[Q]]) : (index, !sql.expr) -> index
// CHECK: %[[RES:.+]] = "sql.get_value"(%[[H]], %[[C0]], %[[C0]]) : (index, index, index) -> i32
// CHECK: return %[[RES]] : i32
// CHECK: }