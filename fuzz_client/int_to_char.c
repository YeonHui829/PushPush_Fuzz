#include <stdio.h>

int main() {
    int number = 3;
    FILE *file = fopen("example.bin", "wb"); // 이진 파일을 쓰기 모드로 열기

    if (file != NULL) {
        fwrite(&number, sizeof(int), 1, file); // 정수를 이진 형식으로 파일에 쓰기
        fclose(file); // 파일 닫기
    }

    return 0;
}