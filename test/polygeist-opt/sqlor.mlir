// -----
// RUN: polygeist-opt --allow-unregistered-dialect %s | FileCheck %s
module {
  func.func @test_or() -> !sql.bool {
    %a = "sql.calc_bool"() <{expr = "x > 0"}> : () -> !sql.bool
    %b = "sql.calc_bool"() <{expr = "y < 5"}> : () -> !sql.bool
    %0 = "sql.or"(%a, %b) : (!sql.bool, !sql.bool) -> !sql.bool
    return %0 : !sql.bool
  }
}

// CHECK-LABEL: func.func @test_or()
// CHECK: %[[OR:.+]] = "sql.or"(%[[A:.+]], %[[B:.+]]) : (!sql.bool, !sql.bool) -> !sql.bool
// CHECK: return %[[OR]] : !sql.bool
