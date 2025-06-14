#include "common.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// 입력 버퍼 비우기
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 안전한 문자열 입력 받기
void safe_input(char *buffer, size_t size) {
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';  // 줄바꿈 제거
        } else {
            clear_input_buffer();   // 입력이 길어져 줄바꿈이 안 들어간 경우만 처리
        }
    }
}

// 날짜 형식 검증 (YYYYMMDD)
int validate_date_format(const char *date) {
    if (strlen(date) != 8) return 0;
    
    for (int i = 0; i < 8; i++) {
        if (!isdigit(date[i])) return 0;
    }
    
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + 
               (date[2] - '0') * 10 + (date[3] - '0');
    int month = (date[4] - '0') * 10 + (date[5] - '0');
    int day = (date[6] - '0') * 10 + (date[7] - '0');
    
    if (month < 1 || month > 12) return 0;
    if (day < 1 || day > 31) return 0;
    
    return 1;
}

// 날짜 입력 받기
int get_date_input(char *date, size_t size) {
    printf("날짜 (YYYYMMDD): ");
    safe_input(date, size);
    
    if (!validate_date_format(date)) {
        printf("잘못된 날짜 형식입니다. YYYYMMDD 형식으로 입력해주세요.\n");
        return 0;
    }
    
    return 1;
}

// 기간 입력 받기 (시작일-종료일)
int get_period_input(char *start_date, char *end_date, size_t size) {
    printf("시작 날짜 (YYYYMMDD): ");
    safe_input(start_date, size);
    
    if (!validate_date_format(start_date)) {
        printf("잘못된 시작 날짜 형식입니다. YYYYMMDD 형식으로 입력해주세요.\n");
        return 0;
    }
    
    printf("종료 날짜 (YYYYMMDD): ");
    safe_input(end_date, size);
    
    if (!validate_date_format(end_date)) {
        printf("잘못된 종료 날짜 형식입니다. YYYYMMDD 형식으로 입력해주세요.\n");
        return 0;
    }
    
    // 시작일이 종료일보다 늦은 경우
    if (strcmp(start_date, end_date) > 0) {
        printf("시작 날짜가 종료 날짜보다 늦을 수 없습니다.\n");
        return 0;
    }
    
    return 1;
}

// 메뉴 선택 입력 받기
int get_menu_choice(int min, int max) {
    int choice;
    char input[10];
    
    while (1) {
        safe_input(input, sizeof(input));
        choice = atoi(input);
        
        if (choice >= min && choice <= max) {
            return choice;
        }
        
        printf("잘못된 선택입니다. %d-%d 사이의 숫자를 입력해주세요: ", min, max);
    }
}

// ID 입력 받기
void get_id_input(char *id, size_t size) {
    printf("아이디: ");
    safe_input(id, size);
}

// 비밀번호 입력 받기
void get_password_input(char *password, size_t size) {
    printf("비밀번호: ");
    safe_input(password, size);
}

// 교육청 코드 입력 받기
void get_edu_office_input(char *edu_office, size_t size) {
    printf("교육청 이름: ");
    safe_input(edu_office, size);
}

// 학교 코드 입력 받기
void get_school_input(char *school, size_t size) {
    printf("학교 코드: ");
    safe_input(school, size);
}
