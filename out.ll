; ModuleID = 'hello.jack'
source_filename = "hello.jack"

%Coord = type { i32, i32 }

@strtmp = private unnamed_addr constant [10 x i8] c"(%d, %d)\0A\00", align 1

declare i32 @printf(ptr %0, ...)

declare ptr @fopen(ptr %0, ptr %1)

declare i32 @fclose(ptr %0)

declare i32 @fread(ptr %0, i32 %1, i32 %2, ptr %3)

declare ptr @malloc(i32 %0)

declare void @free(ptr %0)

declare ptr @strcpy(ptr %0, ptr %1)

declare i32 @strlen(ptr %0)

declare ptr @strcat(ptr %0, ptr %1)

define i32 @Coord_display(ptr %self) {
entry:
  %fieldtmp = getelementptr inbounds %Coord, ptr %self, i32 0, i32 0
  %loadtmp = load i32, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %self, i32 0, i32 1
  %loadtmp2 = load i32, ptr %fieldtmp1, align 4
  %calltmp = call i32 @printf(ptr @strtmp, i32 %loadtmp, i32 %loadtmp2)
  ret i32 0
}

define void @add_Coord(ptr %0, ptr %self, ptr %other) {
entry:

  %structtmp = alloca %Coord, align 8
  %fieldtmp = getelementptr inbounds %Coord, ptr %self, i32 0, i32 0
  %loadtmp = load i32, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %other, i32 0, i32 0
  %loadtmp2 = load i32, ptr %fieldtmp1, align 4
  %addtmp = add i32 %loadtmp, %loadtmp2
  %fieldtmp3 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 0
  store i32 %addtmp, ptr %fieldtmp3, align 4
  %fieldtmp4 = getelementptr inbounds %Coord, ptr %self, i32 0, i32 1
  %loadtmp5 = load i32, ptr %fieldtmp4, align 4
  %fieldtmp6 = getelementptr inbounds %Coord, ptr %other, i32 0, i32 1
  %loadtmp7 = load i32, ptr %fieldtmp6, align 4
  %addtmp8 = add i32 %loadtmp5, %loadtmp7
  %fieldtmp9 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 1
  store i32 %addtmp8, ptr %fieldtmp9, align 4
  %loadtmp10 = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp10, ptr %0, align 8
  ret void
}

define void @sub_Coord(ptr %0, ptr %self, ptr %other) {
entry:
  %allocatmp = alloca %Coord, align 8
  store ptr %other, ptr %allocatmp, align 8
  %structtmp = alloca %Coord, align 8
  %fieldtmp = getelementptr inbounds %Coord, ptr %self, i32 0, i32 0
  %loadtmp = load i32, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %allocatmp, i32 0, i32 0
  %loadtmp2 = load i32, ptr %fieldtmp1, align 4
  %subtmp = sub i32 %loadtmp, %loadtmp2
  %fieldtmp3 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 0
  store i32 %subtmp, ptr %fieldtmp3, align 4
  %fieldtmp4 = getelementptr inbounds %Coord, ptr %self, i32 0, i32 1
  %loadtmp5 = load i32, ptr %fieldtmp4, align 4
  %fieldtmp6 = getelementptr inbounds %Coord, ptr %allocatmp, i32 0, i32 1
  %loadtmp7 = load i32, ptr %fieldtmp6, align 4
  %subtmp8 = sub i32 %loadtmp5, %loadtmp7
  %fieldtmp9 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 1
  store i32 %subtmp8, ptr %fieldtmp9, align 4
  %loadtmp10 = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp10, ptr %0, align 8
  ret void
}

define void @yo(ptr %0) {
entry:
  %structtmp = alloca %Coord, align 8
  %fieldtmp = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 0
  store i32 33, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 1
  store i32 44, ptr %fieldtmp1, align 4
  %loadtmp = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp, ptr %0, align 8
  ret void
}

define i32 @main() {
entry:
  %structtmp = alloca %Coord, align 8
  %fieldtmp = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 0
  store i32 3, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 1
  store i32 4, ptr %fieldtmp1, align 4
  %calltmp = alloca ptr, align 8
  call void @yo(ptr %calltmp)
  %alloctmp = alloca ptr, align 8
  call void @add_Coord(ptr %alloctmp, ptr %structtmp, ptr %calltmp)
  %alloctmp2 = alloca ptr, align 8
  call void @sub_Coord(ptr %alloctmp2, ptr %alloctmp, ptr %calltmp)
  %calltmp3 = call i32 @Coord_display(ptr %alloctmp)
  %calltmp4 = call i32 @Coord_display(ptr %alloctmp2)
  ret i32 0
}
