// RUN: polygeist-opt --allow-unregistered-dialect %s | FileCheck %s

module {
  func.func @test_int() -> !sql.expr {
    %0 = "sql.int"() <{expr = "42"}> : () -> !sql.expr
    return %0 : !sql.expr
  }
}

// CHECK-LABEL: func.func @test_int()
// CHECK: %[[VAL:.+]] = "sql.int"() <{expr = "42"}> : () -> !sql.expr
// CHECK: return %[[VAL]] : !sql.expr
