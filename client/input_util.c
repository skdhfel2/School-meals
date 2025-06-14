#include "common.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// 교육청 코드 매핑
static const struct {
    const char *name;
    const char *code;
} edu_office_map[] = {
    {"서울특별시교육청", "B10"},
    {"부산광역시교육청", "C10"},
    {"대구광역시교육청", "D10"},
    {"인천광역시교육청", "E10"},
    {"광주광역시교육청", "F10"},
    {"대전광역시교육청", "G10"},
    {"울산광역시교육청", "H10"},
    {"세종특별자치시교육청", "I10"},
    {"경기도교육청", "J10"},
    {"강원도교육청", "K10"},
    {"충청북도교육청", "M10"},
    {"충청남도교육청", "N10"},
    {"전라북도교육청", "P10"},
    {"전라남도교육청", "Q10"},
    {"경상북도교육청", "R10"},
    {"경상남도교육청", "S10"},
    {"제주특별자치도교육청", "T10"},
    {NULL, NULL}
};

// 교육청 이름으로 코드 조회
const char* get_edu_office_code(const char* edu_office_name) {
    for (int i = 0; edu_office_map[i].name != NULL; i++) {
        if (strcmp(edu_office_map[i].name, edu_office_name) == 0) {
            return edu_office_map[i].code;
        }
    }
    return NULL;
}

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
bool validate_date_format(const char *date) {
    if (strlen(date) != 8) return false;
    
    for (int i = 0; i < 8; i++) {
        if (!isdigit(date[i])) return false;
    }
    
    int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + 
               (date[2] - '0') * 10 + (date[3] - '0');
    int month = (date[4] - '0') * 10 + (date[5] - '0');
    int day = (date[6] - '0') * 10 + (date[7] - '0');
    
    // 연도 검증 (1900년 이후)
    if (year < 1900) return false;
    
    // 월 검증
    if (month < 1 || month > 12) return false;
    
    // 일 검증
    if (day < 1 || day > 31) return false;
    
    // 2월의 경우 윤년 체크
    if (month == 2) {
        bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (is_leap && day > 29) return false;
        if (!is_leap && day > 28) return false;
    }
    
    // 30일까지 있는 달 체크
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) return false;
    
    return true;
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
    printf("\n=== 기간별 급식 조회 ===\n");
    printf("※ 조회 가능 기간: 최대 7일\n");
    printf("※ 날짜 형식: YYYYMMDD (예: 20240301)\n\n");
    
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

    // 날짜 차이 계산
    int year1, month1, day1, year2, month2, day2;
    sscanf(start_date, "%4d%2d%2d", &year1, &month1, &day1);
    sscanf(end_date, "%4d%2d%2d", &year2, &month2, &day2);
    
    // 날짜를 일수로 변환
    int days1 = year1 * 365 + month1 * 30 + day1;
    int days2 = year2 * 365 + month2 * 30 + day2;
    
    if (days2 - days1 > 6) {
        printf("최대 7일까지만 조회 가능합니다.\n");
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
