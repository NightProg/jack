; ModuleID = 'hello.jack'
source_filename = "hello.jack"

%std_String = type { ptr }
%std_Array = type { ptr, i32 }

@strtmp = private unnamed_addr constant [2 x i8] c"r\00", align 1
@strtmp.1 = private unnamed_addr constant [21 x i8] c"Failed to open file\0A\00", align 1

declare i32 @printf(ptr %0, ...)

declare ptr @fopen(ptr %0, ptr %1)

declare i32 @fclose(ptr %0)

declare i32 @fread(ptr %0, i32 %1, i32 %2, ptr %3)

declare i32 @fseek(ptr %0, i32 %1, i32 %2)

declare i32 @ftell(ptr %0)

declare ptr @malloc(ptr %0)

declare void @free(ptr %0)

declare ptr @strcpy(ptr %0, ptr %1)

declare i32 @strlen(ptr %0)

declare ptr @strcat(ptr %0, ptr %1)

declare i32 @socket(i32 %0, i32 %1, i32 %2)

declare i32 @connect(i32 %0, ptr %1, i32 %2)

declare i32 @send(i32 %0, ptr %1, i32 %2, i32 %3)

declare i32 @recv(i32 %0, ptr %1, i32 %2, i32 %3)

declare i32 @bind(i32 %0, ptr %1, i32 %2)

declare i32 @close(i32 %0)

declare i32 @htons(i32 %0)

define ptr @std_String_append_c_str(ptr %0, ptr %1) {
entry:
  %self = alloca ptr, align 8
  store ptr %0, ptr %self, align 8
  %other = alloca ptr, align 8
  store ptr %1, ptr %other, align 8
  %fieldtmp = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  %calltmp = call i32 @strlen(ptr %loadtmp)
  %self_len = alloca i32, align 4
  store i32 %calltmp, ptr %self_len, align 4
  %loadtmp1 = load ptr, ptr %other, align 8
  %calltmp2 = call i32 @strlen(ptr %loadtmp1)
  %other_len = alloca i32, align 4
  store i32 %calltmp2, ptr %other_len, align 4
  %loadtmp3 = load i32, ptr %self_len, align 4
  %loadtmp4 = load i32, ptr %other_len, align 4
  %addtmp = add i32 %loadtmp3, %loadtmp4
  %new_len = alloca i32, align 4
  store i32 %addtmp, ptr %new_len, align 4
  %loadtmp5 = load i32, ptr %new_len, align 4
  %addtmp6 = add i32 %loadtmp5, 1
  %calltmp7 = call ptr @malloc(i32 %addtmp6)
  %new_str = alloca ptr, align 8
  store ptr %calltmp7, ptr %new_str, align 8
  %loadtmp8 = load ptr, ptr %new_str, align 8
  %fieldtmp9 = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp10 = load ptr, ptr %fieldtmp9, align 8
  %calltmp11 = call ptr @strcpy(ptr %loadtmp8, ptr %loadtmp10)
  %loadtmp12 = load ptr, ptr %new_str, align 8
  %loadtmp13 = load ptr, ptr %new_str, align 8
  %loadtmp14 = load i32, ptr %self_len, align 4
  %geptmp = getelementptr i8, ptr %loadtmp13, i32 %loadtmp14
  %loadtmp15 = load ptr, ptr %other, align 8
  %calltmp16 = call ptr @strcat(ptr %geptmp, ptr %loadtmp15)
  %loadtmp17 = load ptr, ptr %new_str, align 8
  ret ptr %loadtmp17
}

define ptr @std_String_to_c_string(ptr %0) {
entry:
  %self = alloca ptr, align 8
  store ptr %0, ptr %self, align 8
  %fieldtmp = getelementptr inbounds %std_String, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  ret ptr %loadtmp
}

define i32 @std_Array_new(ptr %0, i32 %1, i32 %2) {
entry:
  %self = alloca ptr, align 8
  store ptr %0, ptr %self, align 8
  %length = alloca i32, align 4
  store i32 %1, ptr %length, align 4
  %size = alloca i32, align 4
  store i32 %2, ptr %size, align 4
  %loadtmp = load ptr, ptr %self, align 8
  %fieldtmp = getelementptr inbounds %std_Array, ptr %loadtmp, i32 0, i32 1
  %loadtmp1 = load i32, ptr %length, align 4
  store i32 %loadtmp1, ptr %fieldtmp, align 4
  %loadtmp2 = load ptr, ptr %self, align 8
  %fieldtmp3 = getelementptr inbounds %std_Array, ptr %loadtmp2, i32 0, i32 0
  %loadtmp4 = load i32, ptr %length, align 4
  %loadtmp5 = load i32, ptr %size, align 4
  %multmp = mul i32 %loadtmp4, %loadtmp5
  %calltmp = call ptr @malloc(i32 %multmp)
  store ptr %calltmp, ptr %fieldtmp3, align 8
  ret i32 0
}

define ptr @std_Array_get(ptr %0, i32 %1) {
entry:
  %self = alloca ptr, align 8
  store ptr %0, ptr %self, align 8
  %index = alloca i32, align 4
  store i32 %1, ptr %index, align 4
  %fieldtmp = getelementptr inbounds %std_Array, ptr %self, i32 0, i32 0
  %loadtmp = load ptr, ptr %fieldtmp, align 8
  %fieldtmp1 = getelementptr inbounds %std_Array, ptr %self, i32 0, i32 0
  %loadtmp2 = load ptr, ptr %fieldtmp1, align 8
  %loadtmp3 = load i32, ptr %index, align 4
  %geptmp = getelementptr void, ptr %loadtmp2, i32 %loadtmp3
  ret ptr %geptmp
}

define void @std_read_file(ptr %0, ptr %1) {
entry:
  %path = alloca ptr, align 8
  store ptr %1, ptr %path, align 8
  %loadtmp = load ptr, ptr %path, align 8
  %calltmp = call ptr @fopen(ptr %loadtmp, ptr @strtmp)
  %file = alloca ptr, align 8
  store ptr %calltmp, ptr %file, align 8
  %loadtmp1 = load ptr, ptr %file, align 8
  %eqtmp = icmp eq ptr %loadtmp1, null
  br i1 %eqtmp, label %then, label %merge

then:                                             ; preds = %entry
  %calltmp2 = call i32 @printf(ptr @strtmp.1)
  %structtmp = alloca %std_String, align 8
  %fieldtmp = getelementptr inbounds %std_String, ptr %structtmp, i32 0, i32 0
  store ptr null, ptr %fieldtmp, align 8
  %loadtmp3 = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp3, ptr %0, align 8
  ret void
  br label %merge

merge:                                            ; preds = %then, %entry
  %loadtmp4 = load ptr, ptr %file, align 8
  %calltmp5 = call i32 @fseek(ptr %loadtmp4, i32 0, i32 2)
  %loadtmp6 = load ptr, ptr %file, align 8
  %calltmp7 = call i32 @ftell(ptr %loadtmp6)
  %size = alloca i32, align 4
  store i32 %calltmp7, ptr %size, align 4
  %loadtmp8 = load ptr, ptr %file, align 8
  %calltmp9 = call i32 @fseek(ptr %loadtmp8, i32 0, i32 0)
  %loadtmp10 = load i32, ptr %size, align 4
  %addtmp = add i32 %loadtmp10, 1
  %calltmp11 = call ptr @malloc(i32 %addtmp)
  %buffer = alloca ptr, align 8
  store ptr %calltmp11, ptr %buffer, align 8
  %loadtmp12 = load ptr, ptr %buffer, align 8
  %loadtmp13 = load i32, ptr %size, align 4
  %loadtmp14 = load ptr, ptr %file, align 8
  %calltmp15 = call i32 @fread(ptr %loadtmp12, i32 1, i32 %loadtmp13, ptr %loadtmp14)
  %structtmp16 = alloca %std_String, align 8
  %loadtmp17 = load ptr, ptr %buffer, align 8
  %fieldtmp18 = getelementptr inbounds %std_String, ptr %structtmp16, i32 0, i32 0
  store ptr %loadtmp17, ptr %fieldtmp18, align 8
  %loadtmp19 = load ptr, ptr %structtmp16, align 8
  store ptr %loadtmp19, ptr %0, align 8
  ret void
}

define void @"char*_to_string"(ptr %0, ptr %1) {
entry:
  %self = alloca ptr, align 8
  store ptr %1, ptr %self, align 8
  %structtmp = alloca %std_String, align 8
  %loadtmp = load ptr, ptr %self, align 8
  %fieldtmp = getelementptr inbounds %std_String, ptr %structtmp, i32 0, i32 0
  store ptr %loadtmp, ptr %fieldtmp, align 8
  %loadtmp1 = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp1, ptr %0, align 8
  ret void
}

define i32 @main() {
entry:
  %structtmp = alloca %std_Array, align 8
  %fieldtmp = getelementptr inbounds %std_Array, ptr %structtmp, i32 0, i32 0
  store ptr null, ptr %fieldtmp, align 8
  %fieldtmp1 = getelementptr inbounds %std_Array, ptr %structtmp, i32 0, i32 1
  store i32 0, ptr %fieldtmp1, align 4
  %r = alloca ptr, align 8
  store ptr %structtmp, ptr %r, align 8
  %loadtmp = load ptr, ptr %r, align 8
  %loadtmp2 = load ptr, ptr %r, align 8
  %calltmp = call i32 @std_Array_new(ptr %loadtmp2, i32 10, i32 4)
  ret i32 0
}
