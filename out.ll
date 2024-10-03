; ModuleID = 'hello.jack'
source_filename = "hello.jack"

%std_String = type { ptr }

@strtmp = private unnamed_addr constant [7 x i8] c"hello \00", align 1
@strtmp.1 = private unnamed_addr constant [6 x i8] c"world\00", align 1
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

define ptr @std_String_add(ptr %self, ptr %other) {
entry:
  %fieldtmp = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  %calltmp = call i32 @strlen(ptr %loadtmp)
  %self_len = alloca i32, align 4
  store i32 %calltmp, ptr %self_len, align 4
  %calltmp1 = call i32 @strlen(ptr %other)
  %other_len = alloca i32, align 4
  store i32 %calltmp1, ptr %other_len, align 4
  %loadtmp2 = load i32, ptr %self_len, align 4
  %loadtmp3 = load i32, ptr %other_len, align 4
  %addtmp = add i32 %loadtmp2, %loadtmp3
  %new_len = alloca i32, align 4
  store i32 %addtmp, ptr %new_len, align 4
  %loadtmp4 = load i32, ptr %new_len, align 4
  %addtmp5 = add i32 %loadtmp4, 1
  %calltmp6 = call ptr @malloc(i32 %addtmp5)
  %new_str = alloca ptr, align 8
  store ptr %calltmp6, ptr %new_str, align 8
  %loadtmp7 = load ptr, ptr %new_str, align 8
  %fieldtmp8 = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp9 = load ptr, ptr %fieldtmp8, align 8
  %calltmp10 = call ptr @strcpy(ptr %loadtmp7, ptr %loadtmp9)
  %loadtmp11 = load ptr, ptr %new_str, align 8
  %loadtmp12 = load ptr, ptr %new_str, align 8
  %loadtmp13 = load i32, ptr %self_len, align 4
  %geptmp = getelementptr i8, ptr %loadtmp12, i32 %loadtmp13
  %calltmp14 = call ptr @strcat(ptr %geptmp, ptr %other)
  %fieldtmp15 = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp16 = load ptr, ptr %new_str, align 8
  store ptr %loadtmp16, ptr %fieldtmp15, align 8
  %loadtmp17 = load ptr, ptr %new_str, align 8
  ret ptr %loadtmp17
}

define ptr @std_String_to_c_string(ptr %self) {
entry:
  %fieldtmp = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  ret ptr %loadtmp
}

define i32 @main() {
entry:
  %structtmp = alloca %std_String, align 8
  %fieldtmp = getelementptr inbounds %std_String, ptr %structtmp, i32 0, i32 0
  store ptr @strtmp, ptr %fieldtmp, align 8
  %calltmp = call ptr @std_String_add(ptr %structtmp, ptr @strtmp.1)
  %calltmp1 = call ptr @std_String_to_c_string(ptr %structtmp)
  %calltmp2 = call i32 @printf(ptr @strtmp.2, ptr %calltmp1)
  ret i32 0
}
