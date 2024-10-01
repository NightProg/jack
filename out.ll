; ModuleID = 'hello.jack'
source_filename = "hello.jack"

%String = type { ptr }

@strtmp = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@strtmp.1 = private unnamed_addr constant [7 x i8] c"world!\00", align 1
@strtmp.2 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

declare i32 @printf(ptr %0, ...)

declare ptr @fopen(ptr %0, ptr %1)

declare i32 @fclose(ptr %0)

declare i32 @fread(ptr %0, i32 %1, i32 %2, ptr %3)

declare ptr @malloc(i32 %0)

declare void @free(ptr %0)

declare ptr @strcpy(ptr %0, ptr %1)

declare i32 @strlen(ptr %0)

declare ptr @strcat(ptr %0, ptr %1)

define ptr @String_add(ptr %self, ptr %other) {
entry:
  %fieldtmp = getelementptr inbounds %String, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  %calltmp = call i32 @strlen(ptr %loadtmp)
  %calltmp1 = call i32 @strlen(ptr %other)
  %addtmp = add i32 %calltmp, %calltmp1
  %new_len = alloca i32, align 4
  store i32 %addtmp, ptr %new_len, align 4
  %loadtmp2 = load i32, ptr %new_len, align 4
  %addtmp3 = add i32 %loadtmp2, 1
  %calltmp4 = call ptr @malloc(i32 %addtmp3)
  %new_str = alloca ptr, align 8
  store ptr %calltmp4, ptr %new_str, align 8
  %loadtmp5 = load ptr, ptr %new_str, align 8
  %fieldtmp6 = getelementptr inbounds %String, ptr %self, i32 0, i32 0
  %loadtmp7 = load ptr, ptr %fieldtmp6, align 8
  %calltmp8 = call ptr @strcpy(ptr %loadtmp5, ptr %loadtmp7)
  %loadtmp9 = load ptr, ptr %new_str, align 8
  %calltmp10 = call ptr @strcat(ptr %loadtmp9, ptr %other)
  %loadtmp11 = load ptr, ptr %new_str, align 8
  ret ptr %loadtmp11
}

define i32 @hello() {
entry:
  ret i32 0
}

define i32 @main() {
entry:
  %structtmp = alloca %String, align 8
  %fieldtmp = getelementptr inbounds %String, ptr %structtmp, i32 0, i32 0
  store ptr @strtmp, ptr %fieldtmp, align 8
  %calltmp = call ptr @String_add(ptr %structtmp, ptr @strtmp.1)
  %fieldtmp1 = getelementptr inbounds %String, ptr %structtmp, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp1, align 8
  %calltmp2 = call i32 (ptr, ...) @printf(ptr @strtmp.2, ptr %loadtmp)
  ret i32 0
}
