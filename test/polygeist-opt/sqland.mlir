// -----
// RUN: polygeist-opt --allow-unregistered-dialect %s | FileCheck %s
module {
  func.func @test_and() -> !sql.bool {
    %a = "sql.calc_bool"() <{expr = "x > 0"}> : () -> !sql.bool
    %b = "sql.calc_bool"() <{expr = "y < 5"}> : () -> !sql.bool
    %0 = "sql.and"(%a, %b) : (!sql.bool, !sql.bool) -> !sql.bool
    return %0 : !sql.bool
  }
}

// CHECK-LABEL: func.func @test_and()
// CHECK: %0 = "sql.and"
// CHECK: return %0
