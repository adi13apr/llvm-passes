; RUN: opt -load-pass-plugin=%plugin -passes=inst-counter -disable-output %s 2>&1 | FileCheck %s

; A function with one block: an add and a return.
; CHECK: add: 2 instructions
define i32 @add(i32 %a, i32 %b) {
entry:
  %sum = add i32 %a, %b
  ret i32 %sum
}

; Declarations have no body, so the pass must not print anything for them.
; CHECK-NOT: printf
declare i32 @printf(ptr, ...)