#ifndef INPUT_UTIL_H
#define INPUT_UTIL_H

#include "common.h"

// 입력 버퍼 비우기
void clear_input_buffer(void);

// 안전한 문자열 입력 받기
void safe_input(char *buffer, size_t size);

// 날짜 형식 검증 (YYYYMMDD)
bool validate_date_format(const char *date);

// ID와 비밀번호 유효성 검사 함수
bool is_valid_id(const char *id);
bool is_valid_password(const char *pw);

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

// 교육청 코드 매핑 구조체
typedef struct
{
    const char *name;
    const char *code;
} EduOfficeMap;

// 함수 선언
const char *get_edu_office_code(const char *edu_office_name);
const char *get_edu_office_name(const char *code);

#endif // INPUT_UTIL_H