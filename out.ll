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

define void @Coord_display(ptr %0, ptr %self) {
entry:
  %fieldtmp = getelementptr inbounds %Coord, ptr %self, i32 0, i32 0
  %loadtmp = load i32, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %self, i32 0, i32 1
  %loadtmp2 = load i32, ptr %fieldtmp1, align 4
  %calltmp = call i32 @printf(ptr @strtmp, i32 %loadtmp, i32 %loadtmp2)
  store i32 0, ptr %0, align 4
  ret void
}

define void @add_Coord(ptr %0, ptr %1, ptr %2) {
entry:
  %structtmp = alloca %Coord, align 8
  %fieldtmp = getelementptr inbounds %Coord, ptr %2, i32 0, i32 0
  %loadtmp = load i32, ptr %fieldtmp, align 4
  %fieldtmp1 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 0
  store i32 %loadtmp, ptr %fieldtmp1, align 4
  %fieldtmp2 = getelementptr inbounds %Coord, ptr %2, i32 0, i32 1
  %loadtmp3 = load i32, ptr %fieldtmp2, align 4
  %fieldtmp4 = getelementptr inbounds %Coord, ptr %structtmp, i32 0, i32 1
  store i32 %loadtmp3, ptr %fieldtmp4, align 4
  %loadtmp5 = load ptr, ptr %structtmp, align 8
  store ptr %loadtmp5, ptr %0, align 8
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
  call void @add_Coord(ptr %alloctmp, ptr %structtmp)
  %calltmp2 = alloca i32, align 4
  call void @Coord_display(ptr %calltmp2, ptr %alloctmp)
  ret i32 0
}
