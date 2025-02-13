#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifdef _WIN32
#include <conio.h>  // Windows에서 비밀번호 숨기기용
#else
#include <unistd.h> // Linux의 표준 입출력
#endif

#define KEY_SIZE 32
#define IV_SIZE 16
#define BUFFER_SIZE 256
#define PASSWORD_FILE "passwords.txt"

void handle_errors(const char *msg) {
    perror(msg);
    exit(1);
}

void generate_key_iv(unsigned char *key, unsigned char *iv) {
    if (!RAND_bytes(key, KEY_SIZE) || !RAND_bytes(iv, IV_SIZE)) {
        handle_errors("키 및 IV 생성 오류");
    }
}

// 비밀번호 입력 함수 (Windows: _getch(), Linux: getchar())
void get_password(char *password, size_t size) {
    printf("비밀번호를 입력하세요: ");
    int i = 0;
    char ch;
    
#ifdef _WIN32
    while ((ch = _getch()) != '\r' && i < size - 1) {  // Enter 키 입력 시 종료
        if (ch == '\b' && i > 0) { // 백스페이스 처리
            i--;
            printf("\b \b");
        } else {
            password[i++] = ch;
            printf("*");  // '*' 표시
        }
    }
#else
    while ((ch = getchar()) != '\n' && i < size - 1) { // Enter 키 입력 시 종료
        password[i++] = ch;
        printf("*");
    }
#endif

    password[i] = '\0';  // 문자열 종료
    printf("\n");
}

void encrypt_password(const char *password, unsigned char *key, unsigned char *iv, unsigned char *ciphertext, int *ciphertext_len) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    EVP_EncryptUpdate(ctx, ciphertext, ciphertext_len, (unsigned char *)password, strlen(password));
    int len;
    EVP_EncryptFinal_ex(ctx, ciphertext + *ciphertext_len, &len);
    *ciphertext_len += len;
    EVP_CIPHER_CTX_free(ctx);
}

void decrypt_password(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    int len;
    EVP_DecryptUpdate(ctx, (unsigned char *)plaintext, &len, ciphertext, ciphertext_len);
    int final_len;
    if (EVP_DecryptFinal_ex(ctx, (unsigned char *)plaintext + len, &final_len) <= 0) {
        printf("복호화 실패: 잘못된 키 또는 IV\n");
        plaintext[0] = '\0'; // 복호화 실패 시 빈 문자열 반환
    } else {
        plaintext[len + final_len] = '\0';
    }
    EVP_CIPHER_CTX_free(ctx);
}

void save_encrypted_password(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv) {
    FILE *file = fopen(PASSWORD_FILE, "wb"); // 기존 파일 덮어쓰기
    if (!file) handle_errors("파일 저장 오류");
    fwrite(key, 1, KEY_SIZE, file);
    fwrite(iv, 1, IV_SIZE, file);
    fwrite(&ciphertext_len, sizeof(int), 1, file);
    fwrite(ciphertext, 1, ciphertext_len, file);
    fclose(file);
}

int load_encrypted_password(unsigned char *ciphertext, int *ciphertext_len, unsigned char *key, unsigned char *iv) {
    FILE *file = fopen(PASSWORD_FILE, "rb");
    if (!file) return 0;
    fread(key, 1, KEY_SIZE, file);
    fread(iv, 1, IV_SIZE, file);
    fread(ciphertext_len, sizeof(int), 1, file);
    fread(ciphertext, 1, *ciphertext_len, file);
    fclose(file);
    return 1;
}

int main() {
    unsigned char key[KEY_SIZE], iv[IV_SIZE], ciphertext[BUFFER_SIZE];
    char password[BUFFER_SIZE], decrypted_password[BUFFER_SIZE];
    int ciphertext_len, choice;

    while (1) {
        printf("\n===== 비밀번호 관리 프로그램 =====\n");
        printf("1. 비밀번호 저장\n");
        printf("2. 비밀번호 복호화\n");
        printf("3. 종료\n");
        printf("선택: ");
        scanf("%d", &choice);
        getchar(); // 개행 문자 제거

        if (choice == 1) {
            get_password(password, BUFFER_SIZE);
            generate_key_iv(key, iv);
            encrypt_password(password, key, iv, ciphertext, &ciphertext_len);
            save_encrypted_password(ciphertext, ciphertext_len, key, iv);
            printf("비밀번호가 저장되었습니다\n");
        } else if (choice == 2) {
            if (load_encrypted_password(ciphertext, &ciphertext_len, key, iv)) {
                decrypt_password(ciphertext, ciphertext_len, key, iv, decrypted_password);
                if (strlen(decrypted_password) > 0) {
                    printf("저장된 비밀번호: %s\n", decrypted_password);
                }
            } else {
                printf("저장된 비밀번호가 없습니다\n");
            }
        } else if (choice == 3) {
            printf("프로그램을 종료합니다\n");
            break;
        } else {
            printf("올바른 옵션을 선택하세요\n");
        }
    }
    return 0;
}
