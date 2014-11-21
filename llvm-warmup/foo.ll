; ModuleID = 'foo.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @quicksort(i32* %array, i32 %size) #0 {
  %1 = alloca i32*, align 8
  %2 = alloca i32, align 4
  store i32* %array, i32** %1, align 8
  store i32 %size, i32* %2, align 4
  %3 = load i32** %1, align 8
  %4 = load i32* %2, align 4
  %5 = sub nsw i32 %4, 1
  call void @_qs(i32* %3, i32 0, i32 %5)
  ret void
}

; Function Attrs: nounwind uwtable
define internal void @_qs(i32* %array, i32 %l, i32 %r) #0 {
  %1 = alloca i32*, align 8
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %p = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp = alloca i32, align 4
  store i32* %array, i32** %1, align 8
  store i32 %l, i32* %2, align 4
  store i32 %r, i32* %3, align 4
  store i32 0, i32* %p, align 4
  %4 = load i32* %2, align 4
  store i32 %4, i32* %i, align 4
  %5 = load i32* %3, align 4
  store i32 %5, i32* %j, align 4
  br label %6

; <label>:6                                       ; preds = %62, %0
  %7 = load i32* %i, align 4
  %8 = load i32* %j, align 4
  %9 = icmp sle i32 %7, %8
  br i1 %9, label %10, label %63

; <label>:10                                      ; preds = %6
  br label %11

; <label>:11                                      ; preds = %19, %10
  %12 = load i32* %j, align 4
  %13 = sext i32 %12 to i64
  %14 = load i32** %1, align 8
  %15 = getelementptr inbounds i32* %14, i64 %13
  %16 = load i32* %15, align 4
  %17 = load i32* %p, align 4
  %18 = icmp sgt i32 %16, %17
  br i1 %18, label %19, label %22

; <label>:19                                      ; preds = %11
  %20 = load i32* %j, align 4
  %21 = add nsw i32 %20, -1
  store i32 %21, i32* %j, align 4
  br label %11

; <label>:22                                      ; preds = %11
  br label %23

; <label>:23                                      ; preds = %31, %22
  %24 = load i32* %i, align 4
  %25 = sext i32 %24 to i64
  %26 = load i32** %1, align 8
  %27 = getelementptr inbounds i32* %26, i64 %25
  %28 = load i32* %27, align 4
  %29 = load i32* %j, align 4
  %30 = icmp slt i32 %28, %29
  br i1 %30, label %31, label %34

; <label>:31                                      ; preds = %23
  %32 = load i32* %i, align 4
  %33 = add nsw i32 %32, 1
  store i32 %33, i32* %i, align 4
  br label %23

; <label>:34                                      ; preds = %23
  %35 = load i32* %i, align 4
  %36 = load i32* %j, align 4
  %37 = icmp sle i32 %35, %36
  br i1 %37, label %38, label %62

; <label>:38                                      ; preds = %34
  %39 = load i32* %i, align 4
  %40 = sext i32 %39 to i64
  %41 = load i32** %1, align 8
  %42 = getelementptr inbounds i32* %41, i64 %40
  %43 = load i32* %42, align 4
  store i32 %43, i32* %tmp, align 4
  %44 = load i32* %j, align 4
  %45 = sext i32 %44 to i64
  %46 = load i32** %1, align 8
  %47 = getelementptr inbounds i32* %46, i64 %45
  %48 = load i32* %47, align 4
  %49 = load i32* %i, align 4
  %50 = sext i32 %49 to i64
  %51 = load i32** %1, align 8
  %52 = getelementptr inbounds i32* %51, i64 %50
  store i32 %48, i32* %52, align 4
  %53 = load i32* %tmp, align 4
  %54 = load i32* %j, align 4
  %55 = sext i32 %54 to i64
  %56 = load i32** %1, align 8
  %57 = getelementptr inbounds i32* %56, i64 %55
  store i32 %53, i32* %57, align 4
  %58 = load i32* %i, align 4
  %59 = add nsw i32 %58, 1
  store i32 %59, i32* %i, align 4
  %60 = load i32* %j, align 4
  %61 = add nsw i32 %60, -1
  store i32 %61, i32* %j, align 4
  br label %62

; <label>:62                                      ; preds = %38, %34
  br label %6

; <label>:63                                      ; preds = %6
  %64 = load i32* %i, align 4
  %65 = load i32* %3, align 4
  %66 = icmp slt i32 %64, %65
  br i1 %66, label %67, label %71

; <label>:67                                      ; preds = %63
  %68 = load i32** %1, align 8
  %69 = load i32* %i, align 4
  %70 = load i32* %3, align 4
  call void @_qs(i32* %68, i32 %69, i32 %70)
  br label %71

; <label>:71                                      ; preds = %67, %63
  %72 = load i32* %2, align 4
  %73 = load i32* %j, align 4
  %74 = icmp slt i32 %72, %73
  br i1 %74, label %75, label %79

; <label>:75                                      ; preds = %71
  %76 = load i32** %1, align 8
  %77 = load i32* %2, align 4
  %78 = load i32* %j, align 4
  call void @_qs(i32* %76, i32 %77, i32 %78)
  br label %79

; <label>:79                                      ; preds = %75, %71
  ret void
}

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = metadata !{metadata !"clang version 3.5.0 (tags/RELEASE_350/final)"}
