#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <zlib.h>

int main() {
    printf("test zlib\n");

    // test write string to one .gz file
    int err;
    const char out[] = "foo.gz";
    const char hello[] = "hello, hello!";
    int len = strlen(hello) + 1;
    gzFile file;
    z_off_t pos;
    file = gzopen(out, "wb");
    if (file == NULL) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    if (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
        exit(1);
    }
    if (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzseek(file, 1, SEEK_CUR);  // add one zero 0 byte !(因为存入的是字符串，不是直接俄调用的gzwrite)
    gzclose(file);

    // test read from a more big file and write to one .gz file(检测是否能够默认开启压缩功能)
    FILE *hFile = NULL;
    hFile = fopen("libzlibstatic.a", "rb");
    if (hFile == NULL) {
        fprintf(stderr, "fopen error\n");
        exit(1);
    }
    fseek(hFile, 0, SEEK_END);
    int fileSize = ftell(hFile);
    fseek(hFile, 0, SEEK_SET);
    unsigned char * buf = (unsigned char*)malloc(fileSize + 8);
    memset(buf, 0, fileSize + 8);
    if (fread(buf, 1, fileSize, hFile) != fileSize) {
        fprintf(stderr, "fread error\n");
        exit(1);
    }
    fclose(hFile);
    file = gzopen(out, "wb");
    if (gzwrite(file, buf, fileSize) != fileSize) {
        fprintf(stderr, "gzwrite err: %s\n", gzerror(file, &err));
        exit(1);
    }
    gzclose(file);
    return 0;
}