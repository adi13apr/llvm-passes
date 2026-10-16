; RUN: opt -load-pass-plugin=%plugin -passes=inst-counter -disable-output %s 2>&1 | FileCheck %s
   ; CHECK: max: 4 instructions
   define i32 @max(i32 %a, i32 %b) {
entry:
  %cmp = icmp sgt i32 %a, %b
  br i1 %cmp, label %then, label %else
then:
  ret i32 %a
else:
  ret i32 %b
}