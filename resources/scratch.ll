target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-suse-linux-gnu"

@A.b = global i32 1
@a = global i32 1
@d = global i32 2
@A.c = global i32 2
@A.e = global i32 3
@A.a = global i32 3
@B.g = global i32 4
@A.f = global i32 4
@A.B.i = global i32 5
@A.h = global i32 5
@A.B.k = global i32 6
@A.j = global i32 6
@A.m = global i32 7
@A.B.l = global i32 7
@B.o = global i32 8
@A.B.n = global i32 8
@q = global i32 9
@A.p = global i32 9
@A.s = global i32 10
@A.r = global i32 10
@B.u = global i32 11
@A.t = global i32 11
@B.C.w = global i32 12
@B.v = global i32 12
@B.z = global i32 13
@B.C.x = global i32 13
@B.C.a = global i32 14
@B.C.y = global i32 14
@f = global ptr @l0
@g = global ptr @l1

define i32 @l0(i32 %0, i32 %1) {
  %add = add i32 %0, %1
  ret i32 %add
}

define i32 @l1(i32 %0) {
  %call = call i32 @f(i32 %0, i32 %0)
  ret i32 %call
}
