#include <stdio.h>       // puts()
#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>
int main() {
    int number = 3;
    char buf[256];
    int fd = open("example.bin", O_RDWR| O_CREAT);

    write(fd,(void *)&number, sizeof(number));

    // close(fd);

    // fd = open("example.bin", O_RDONLY);
    int num2;
    read(fd, (void *)&num2, sizeof(number));
    fprintf(stderr, "number : %d\n", num2);
    // FILE *file = fopen("example.bin", "wb"); // 이진 파일을 쓰기 모드로 열기

    // if (file != NULL) {
    //     fwrite(&number, sizeof(int), 1, file); // 정수를 이진 형식으로 파일에 쓰기
    //     fclose(file); // 파일 닫기
    // }

    // file = fopen("example.bin", "rb"); // 이진 파일을 쓰기 모드로 열기

    // int input;
    // fread(&input, sizeof(int), 1, file);
    // fprintf(stderr, "input : %d\n", input);
    // fclose(file); // 파일 닫기

    return 0;
}