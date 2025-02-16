#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <openssl/evp.h>

void test_openssl_static() {
    char sourceData[] = "Welcome to openssl Wikiwiki";
    char encodedData[100] = {0};
    char decodedData[100] = {0};
    EVP_EncodeBlock((unsigned char*)encodedData, (unsigned char*)sourceData, strlen(sourceData));
    printf("%s\n", encodedData);
    EVP_DecodeBlock((unsigned char*)decodedData, (unsigned char*)encodedData, strlen(encodedData));
    printf("%s\n", decodedData);
}

int main() {
    printf("test openssl\n");
    test_openssl_static();
    return 0;
}