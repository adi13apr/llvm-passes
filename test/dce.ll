; RUN: opt -load-pass-plugin=%dceplugin -passes=my-dce -S %s | FileCheck %s

; The multiply's result is never used, so the pass should remove it,
; while keeping the add and the return.
; CHECK: define i32 @f(i32 %a)
; CHECK-NOT: %unused
; CHECK: ret i32 %r
define i32 @f(i32 %a) {
entry:
  %unused = mul i32 %a, 2
  %r = add i32 %a, 1
  ret i32 %r
}