target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-suse-linux-gnu"

@a = global i32 4

define i32 @f(i32 %0, i32 %1) {
  %add = add i32 %0, %1
  ret i32 %add
}

define i32 @g(i32 %0) {
  %call = call i32 @f(i32 %0, i32 %0)
  ret i32 %call
}
