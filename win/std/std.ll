; ModuleID = 'std.bc'
source_filename = "std.c"
target datalayout = "e-m:x-p:32:32-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i686-pc-windows-msvc19.11.25507"

%struct._OVERLAPPED = type { i32, i32, %union.anon, i8* }
%union.anon = type { %struct.anon }
%struct.anon = type { i32, i32 }

$"\01??_C@_01EEMJAFIK@?6?$AA@" = comdat any

@"\01??_C@_01EEMJAFIK@?6?$AA@" = linkonce_odr unnamed_addr constant [2 x i8] c"\0A\00", comdat, align 1

; Function Attrs: noinline nounwind optnone
define void @system_exit(i32 %exitCode) #0 {
entry:
  %exitCode.addr = alloca i32, align 4
  store i32 %exitCode, i32* %exitCode.addr, align 4
  %0 = load i32, i32* %exitCode.addr, align 4
  call x86_stdcallcc void @"\01_ExitProcess@4"(i32 %0) #3
  unreachable

return:                                           ; No predecessors!
  ret void
}

; Function Attrs: noreturn
declare dllimport x86_stdcallcc void @"\01_ExitProcess@4"(i32) #1

; Function Attrs: noinline nounwind optnone
define i32 @_strlen(i8* %message) #0 {
entry:
  %message.addr = alloca i8*, align 4
  %len = alloca i32, align 4
  store i8* %message, i8** %message.addr, align 4
  store i32 0, i32* %len, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i8*, i8** %message.addr, align 4
  %1 = load i32, i32* %len, align 4
  %arrayidx = getelementptr inbounds i8, i8* %0, i32 %1
  %2 = load i8, i8* %arrayidx, align 1
  %tobool = icmp ne i8 %2, 0
  br i1 %tobool, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, i32* %len, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* %len, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %4 = load i32, i32* %len, align 4
  ret i32 %4
}

; Function Attrs: noinline nounwind optnone
define void @_i32_toString(i32 %value, i8* %buffer) #0 {
entry:
  %buffer.addr = alloca i8*, align 4
  %value.addr = alloca i32, align 4
  %index = alloca i16, align 2
  %sign = alloca i8, align 1
  %t = alloca i16, align 2
  store i8* %buffer, i8** %buffer.addr, align 4
  store i32 %value, i32* %value.addr, align 4
  store i16 0, i16* %index, align 2
  %0 = load i32, i32* %value.addr, align 4
  %cmp = icmp eq i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load i8*, i8** %buffer.addr, align 4
  %2 = load i16, i16* %index, align 2
  %inc = add i16 %2, 1
  store i16 %inc, i16* %index, align 2
  %idxprom = sext i16 %2 to i32
  %arrayidx = getelementptr inbounds i8, i8* %1, i32 %idxprom
  store i8 48, i8* %arrayidx, align 1
  br label %if.end53

if.else:                                          ; preds = %entry
  store i8 0, i8* %sign, align 1
  %3 = load i32, i32* %value.addr, align 4
  %cmp1 = icmp slt i32 %3, 0
  br i1 %cmp1, label %if.then2, label %if.end

if.then2:                                         ; preds = %if.else
  %4 = load i32, i32* %value.addr, align 4
  %sub = sub nsw i32 0, %4
  store i32 %sub, i32* %value.addr, align 4
  store i8 45, i8* %sign, align 1
  br label %if.end

if.end:                                           ; preds = %if.then2, %if.else
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.end
  %5 = load i32, i32* %value.addr, align 4
  %cmp3 = icmp ne i32 %5, 0
  br i1 %cmp3, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %6 = load i32, i32* %value.addr, align 4
  %rem = srem i32 %6, 10
  %add = add nsw i32 %rem, 48
  %conv = trunc i32 %add to i8
  %7 = load i8*, i8** %buffer.addr, align 4
  %8 = load i16, i16* %index, align 2
  %inc4 = add i16 %8, 1
  store i16 %inc4, i16* %index, align 2
  %idxprom5 = sext i16 %8 to i32
  %arrayidx6 = getelementptr inbounds i8, i8* %7, i32 %idxprom5
  store i8 %conv, i8* %arrayidx6, align 1
  %9 = load i32, i32* %value.addr, align 4
  %div = sdiv i32 %9, 10
  store i32 %div, i32* %value.addr, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %10 = load i8, i8* %sign, align 1
  %conv7 = sext i8 %10 to i32
  %cmp8 = icmp ne i32 %conv7, 0
  br i1 %cmp8, label %if.then10, label %if.end14

if.then10:                                        ; preds = %while.end
  %11 = load i8, i8* %sign, align 1
  %12 = load i8*, i8** %buffer.addr, align 4
  %13 = load i16, i16* %index, align 2
  %inc11 = add i16 %13, 1
  store i16 %inc11, i16* %index, align 2
  %idxprom12 = sext i16 %13 to i32
  %arrayidx13 = getelementptr inbounds i8, i8* %12, i32 %idxprom12
  store i8 %11, i8* %arrayidx13, align 1
  br label %if.end14

if.end14:                                         ; preds = %if.then10, %while.end
  store i16 0, i16* %t, align 2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end14
  %14 = load i16, i16* %t, align 2
  %conv15 = sext i16 %14 to i32
  %15 = load i16, i16* %index, align 2
  %conv16 = sext i16 %15 to i32
  %div17 = sdiv i32 %conv16, 2
  %cmp18 = icmp slt i32 %conv15, %div17
  br i1 %cmp18, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %16 = load i8*, i8** %buffer.addr, align 4
  %17 = load i16, i16* %index, align 2
  %conv20 = sext i16 %17 to i32
  %18 = load i16, i16* %t, align 2
  %conv21 = sext i16 %18 to i32
  %sub22 = sub nsw i32 %conv20, %conv21
  %sub23 = sub nsw i32 %sub22, 1
  %arrayidx24 = getelementptr inbounds i8, i8* %16, i32 %sub23
  %19 = load i8, i8* %arrayidx24, align 1
  %conv25 = sext i8 %19 to i32
  %20 = load i8*, i8** %buffer.addr, align 4
  %21 = load i16, i16* %t, align 2
  %idxprom26 = sext i16 %21 to i32
  %arrayidx27 = getelementptr inbounds i8, i8* %20, i32 %idxprom26
  %22 = load i8, i8* %arrayidx27, align 1
  %conv28 = sext i8 %22 to i32
  %xor = xor i32 %conv28, %conv25
  %conv29 = trunc i32 %xor to i8
  store i8 %conv29, i8* %arrayidx27, align 1
  %23 = load i8*, i8** %buffer.addr, align 4
  %24 = load i16, i16* %t, align 2
  %idxprom30 = sext i16 %24 to i32
  %arrayidx31 = getelementptr inbounds i8, i8* %23, i32 %idxprom30
  %25 = load i8, i8* %arrayidx31, align 1
  %conv32 = sext i8 %25 to i32
  %26 = load i8*, i8** %buffer.addr, align 4
  %27 = load i16, i16* %index, align 2
  %conv33 = sext i16 %27 to i32
  %28 = load i16, i16* %t, align 2
  %conv34 = sext i16 %28 to i32
  %sub35 = sub nsw i32 %conv33, %conv34
  %sub36 = sub nsw i32 %sub35, 1
  %arrayidx37 = getelementptr inbounds i8, i8* %26, i32 %sub36
  %29 = load i8, i8* %arrayidx37, align 1
  %conv38 = sext i8 %29 to i32
  %xor39 = xor i32 %conv38, %conv32
  %conv40 = trunc i32 %xor39 to i8
  store i8 %conv40, i8* %arrayidx37, align 1
  %30 = load i8*, i8** %buffer.addr, align 4
  %31 = load i16, i16* %index, align 2
  %conv41 = sext i16 %31 to i32
  %32 = load i16, i16* %t, align 2
  %conv42 = sext i16 %32 to i32
  %sub43 = sub nsw i32 %conv41, %conv42
  %sub44 = sub nsw i32 %sub43, 1
  %arrayidx45 = getelementptr inbounds i8, i8* %30, i32 %sub44
  %33 = load i8, i8* %arrayidx45, align 1
  %conv46 = sext i8 %33 to i32
  %34 = load i8*, i8** %buffer.addr, align 4
  %35 = load i16, i16* %t, align 2
  %idxprom47 = sext i16 %35 to i32
  %arrayidx48 = getelementptr inbounds i8, i8* %34, i32 %idxprom47
  %36 = load i8, i8* %arrayidx48, align 1
  %conv49 = sext i8 %36 to i32
  %xor50 = xor i32 %conv49, %conv46
  %conv51 = trunc i32 %xor50 to i8
  store i8 %conv51, i8* %arrayidx48, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %37 = load i16, i16* %t, align 2
  %inc52 = add i16 %37, 1
  store i16 %inc52, i16* %t, align 2
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %if.end53

if.end53:                                         ; preds = %for.end, %if.then
  %38 = load i8*, i8** %buffer.addr, align 4
  %39 = load i16, i16* %index, align 2
  %idxprom54 = sext i16 %39 to i32
  %arrayidx55 = getelementptr inbounds i8, i8* %38, i32 %idxprom54
  store i8 0, i8* %arrayidx55, align 1
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @print(i8* %message) #0 {
entry:
  %message.addr = alloca i8*, align 4
  %out = alloca i8*, align 4
  store i8* %message, i8** %message.addr, align 4
  %call = call x86_stdcallcc i8* @"\01_GetStdHandle@4"(i32 -11)
  store i8* %call, i8** %out, align 4
  %0 = load i8*, i8** %message.addr, align 4
  %call1 = call i32 @_strlen(i8* %0)
  %1 = load i8*, i8** %message.addr, align 4
  %2 = load i8*, i8** %out, align 4
  %call2 = call x86_stdcallcc i32 @"\01_WriteFile@20"(i8* %2, i8* %1, i32 %call1, i32* null, %struct._OVERLAPPED* null)
  ret void
}

declare dllimport x86_stdcallcc i8* @"\01_GetStdHandle@4"(i32) #2

declare dllimport x86_stdcallcc i32 @"\01_WriteFile@20"(i8*, i8*, i32, i32*, %struct._OVERLAPPED*) #2

; Function Attrs: noinline nounwind optnone
define void @println() #0 {
entry:
  %out = alloca i8*, align 4
  %call = call x86_stdcallcc i8* @"\01_GetStdHandle@4"(i32 -11)
  store i8* %call, i8** %out, align 4
  %0 = load i8*, i8** %out, align 4
  %call1 = call x86_stdcallcc i32 @"\01_WriteFile@20"(i8* %0, i8* getelementptr inbounds ([2 x i8], [2 x i8]* @"\01??_C@_01EEMJAFIK@?6?$AA@", i32 0, i32 0), i32 1, i32* null, %struct._OVERLAPPED* null)
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @print_i32(i32 %value) #0 {
entry:
  %value.addr = alloca i32, align 4
  %buffer = alloca [15 x i8], align 1
  %addr = alloca i8*, align 4
  store i32 %value, i32* %value.addr, align 4
  %arrayidx = getelementptr inbounds [15 x i8], [15 x i8]* %buffer, i32 0, i32 0
  store i8* %arrayidx, i8** %addr, align 4
  %0 = load i8*, i8** %addr, align 4
  %1 = load i32, i32* %value.addr, align 4
  call void @_i32_toString(i32 %1, i8* %0)
  %arraydecay = getelementptr inbounds [15 x i8], [15 x i8]* %buffer, i32 0, i32 0
  call void @print(i8* %arraydecay)
  ret void
}

; Function Attrs: noinline nounwind optnone
define void @println_i32(i32 %value) #0 {
entry:
  %value.addr = alloca i32, align 4
  store i32 %value, i32* %value.addr, align 4
  %0 = load i32, i32* %value.addr, align 4
  call void @print_i32(i32 %0)
  call void @println()
  ret void
}

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn }

!llvm.linker.options = !{!0, !0}
!llvm.module.flags = !{!1, !2}
!llvm.ident = !{!3}

!0 = !{!"/DEFAULTLIB:uuid.lib"}
!1 = !{i32 1, !"NumRegisterParameters", i32 0}
!2 = !{i32 1, !"wchar_size", i32 2}
!3 = !{!"clang version 6.0.0 (trunk)"}
