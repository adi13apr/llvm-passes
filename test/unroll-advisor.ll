; RUN: opt -load-pass-plugin=%unrollplugin -passes=unroll-advisor -disable-output %s 2>&1 | FileCheck %s

; Known trip count (4) and a small body (8): 4 x 8 = 32 <= 64.
; CHECK: loop %loop in sum4: trip count 4, size 8 -> full unroll
define i32 @sum4(ptr %a) {
entry:
  br label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %sum = phi i32 [ 0, %entry ], [ %sum.next, %loop ]
  %p = getelementptr inbounds i32, ptr %a, i32 %i
  %v = load i32, ptr %p
  %sum.next = add i32 %sum, %v
  %i.next = add nsw i32 %i, 1
  %cond = icmp slt i32 %i.next, 4
  br i1 %cond, label %loop, label %exit
exit:
  ret i32 %sum.next
}

; Unknown trip count (depends on %n), small body (8 <= 16).
; CHECK: loop %loop in sumN: trip count unknown, size 8 -> partial unroll by 4
define i32 @sumN(ptr %a, i32 %n) {
entry:
  br label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %sum = phi i32 [ 0, %entry ], [ %sum.next, %loop ]
  %p = getelementptr inbounds i32, ptr %a, i32 %i
  %v = load i32, ptr %p
  %sum.next = add i32 %sum, %v
  %i.next = add nsw i32 %i, 1
  %cond = icmp slt i32 %i.next, %n
  br i1 %cond, label %loop, label %exit
exit:
  ret i32 %sum.next
}

; Unknown trip count and a large body (18 > 16): unrolling would bloat the code.
; CHECK: loop %loop in bigN: trip count unknown, size 18 -> no unroll
define i32 @bigN(i32 %x, i32 %n) {
entry:
  br label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %i.next, %loop ]
  %s = phi i32 [ 0, %entry ], [ %s13, %loop ]
  %s1 = add i32 %s, %x
  %s2 = add i32 %s1, %x
  %s3 = add i32 %s2, %x
  %s4 = add i32 %s3, %x
  %s5 = add i32 %s4, %x
  %s6 = add i32 %s5, %x
  %s7 = add i32 %s6, %x
  %s8 = add i32 %s7, %x
  %s9 = add i32 %s8, %x
  %s10 = add i32 %s9, %x
  %s11 = add i32 %s10, %x
  %s12 = add i32 %s11, %x
  %s13 = add i32 %s12, %x
  %i.next = add nsw i32 %i, 1
  %cond = icmp slt i32 %i.next, %n
  br i1 %cond, label %loop, label %exit
exit:
  ret i32 %s13
}