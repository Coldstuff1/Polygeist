// -----
// RUN: polygeist-opt --allow-unregistered-dialect %s | FileCheck %s
module {
  func.func @test_arith() -> !sql.expr {
    %a = "sql.column"() <{expr = "a"}> : () -> !sql.expr
    %b = "sql.column"() <{expr = "b"}> : () -> !sql.expr
    %0 = "sql.arith"(%a, %b) <{op = "+"}> : (!sql.expr, !sql.expr) -> !sql.expr
    return %0 : !sql.expr
  }
}

// CHECK-LABEL: func.func @test_arith()
// CHECK: %[[A:.+]] = "sql.column"() <{expr = "a"}> : () -> !sql.expr
// CHECK: %[[B:.+]] = "sql.column"() <{expr = "b"}> : () -> !sql.expr
// CHECK: %[[ARITH:.+]] = "sql.arith"(%[[A]], %[[B]]) <{op = "+"}> : (!sql.expr, !sql.expr) -> !sql.expr
// CHECK: return %[[ARITH]] : !sql.expr
