#ifndef INPUT_UTIL_H
#define INPUT_UTIL_H

#include "common.h"

// 입력 버퍼 비우기
void clear_input_buffer(void);

// 안전한 문자열 입력 받기
void safe_input(char *buffer, size_t size);

// 날짜 형식 검증 (YYYYMMDD)
bool validate_date_format(const char *date);

// 날짜 입력 받기
bool get_date_input(char *date, size_t size);

// 기간 입력 받기 (시작일-종료일)
bool get_period_input(char *start_date, char *end_date, size_t size);

// 메뉴 선택 입력 받기
int get_menu_choice(int min, int max);

// ID 입력 받기
void get_id_input(char *id, size_t size);

// 비밀번호 입력 받기
void get_password_input(char *password, size_t size);

// 교육청 코드 입력 받기
void get_edu_office_input(char *edu_office, size_t size);

// 학교 코드 입력 받기
void get_school_input(char *school, size_t size);

// 교육청 이름-코드 매핑 테이블
typedef struct {
    const char *name;
    const char *code;
} EduOfficeMap;

EduOfficeMap edu_office_map[] = {
    {"서울특별시교육청", "B10"},
    {"부산광역시교육청", "C10"},
    {"대구광역시교육청", "D10"},
    {"인천광역시교육청", "I10"},
    {"광주광역시교육청", "F10"},
    {"대전광역시교육청", "G10"},
    {"울산광역시교육청", "H10"},
    {"세종특별자치시교육청", "M10"},
    {"경기도교육청", "J10"},
    {"강원도교육청", "K10"},
    {"충청북도교육청", "N10"},
    {"충청남도교육청", "P10"},
    {"전라북도교육청", "Q10"},
    {"전라남도교육청", "R10"},
    {"경상북도교육청", "S10"},
    {"경상남도교육청", "T10"},
    {"제주특별자치도교육청", "U10"},
    {NULL, NULL}
};

const char* get_edu_office_code(const char* name) {
    for (int i = 0; edu_office_map[i].name != NULL; i++) {
        if (strcmp(edu_office_map[i].name, name) == 0) {
            return edu_office_map[i].code;
        }
    }
    return NULL; // 못 찾으면 NULL
}

#endif // INPUT_UTIL_H 