; RUN: opt -load-pass-plugin=%dceplugin -passes=my-dce -S %s | FileCheck %s

; A single unused instruction is removed; the needed code stays.
; CHECK-LABEL: define i32 @dead_mul(i32 %a)
; CHECK-NOT: %unused
; CHECK: ret i32 %r
define i32 @dead_mul(i32 %a) {
entry:
  %unused = mul i32 %a, 2
  %r = add i32 %a, 1
  ret i32 %r
}

; A chain: %y is unused, and %x is only used by %y. Both must be removed.
; CHECK-LABEL: define i32 @dead_chain(i32 %a)
; CHECK-NOT: %x
; CHECK-NOT: %y
; CHECK: ret i32 %a
define i32 @dead_chain(i32 %a) {
entry:
  %x = mul i32 %a, 2
  %y = add i32 %x, 1
  ret i32 %a
}