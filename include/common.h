#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>

#pragma comment(lib, "sqlite3.lib")

#define close closesocket

#define BUFFER_SIZE 4096
#define MAX_ID_LEN 20
#define MAX_PW_LEN 20
#define MAX_NAME_LEN 50
#define MAX_ROLE_LEN 20
#define MAX_EDU_OFFICE_LEN 50
#define MAX_SCHOOL_NAME_LEN 50
#define MAX_MEAL_LEN 2000
#define MAX_DATE_LEN 11
#define MAX_PERIOD_LEN 21
#define MAX_CHILDREN 10 // ✅ 최대 자녀 수

// 성공/실패 응답 코드 (정수 값)
#define SUCCESS 1        // 성공 (논리적으로 TRUE 의미)
#define ERROR 0          // 일반 에러
#define DUPLICATE_CODE 2 // 중복 ID 등 처리 시

// 문자열 응답용 상수
#define SUCCESS_STR "SUCCESS"
#define ERROR_STR "ERROR"
#define DUPLICATE_CODE_STR "DUPLICATE"

// 에러 메시지
#define ERR_INVALID_ID "잘못된 ID입니다"
#define ERR_INVALID_PW "잘못된 비밀번호입니다"
#define ERR_USER_NOT_FOUND "사용자를 찾을 수 없습니다"
#define ERR_DUPLICATE_ID "이미 존재하는 ID입니다"

// 서버 설정
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// 데이터베이스 경로
#define DB_PATH "school_meals.db"

// 구조체 정의
typedef struct
{
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
    char name[MAX_NAME_LEN];
    char role[MAX_ROLE_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
} User;

typedef struct
{
    char id[MAX_ID_LEN];
    char name[MAX_NAME_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
} Child;

typedef struct
{
    char date[MAX_DATE_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char meal[MAX_MEAL_LEN];
} Meal;

typedef struct
{
    int status;
    char message[BUFFER_SIZE];
    char data[BUFFER_SIZE];
} Response;

// 유틸리티 함수
bool validate_id(const char *id);
bool validate_password(const char *password);
bool validate_date_format(const char *date);
void trim_string(char *str);

// 유효성 검사 함수
bool is_valid_id(const char *id);
bool is_valid_password(const char *pw);

// 공통 네트워크 함수
bool init_network(void);
void cleanup_network(void);
SOCKET create_socket(void);
bool connect_to_server(SOCKET sock, const char *ip, int port);
bool send_data(SOCKET sock, const char *data, int len);
bool receive_data(SOCKET sock, char *buffer, int len);
void close_socket(SOCKET sock);

// 공통 응답 처리 함수
bool parse_response(const char *buffer, int *status, char *message, char *data);
bool send_response(SOCKET sock, int status, const char *message, const char *data);

bool get_meal_from_neis(const char *edu_office, const char *school_name, const char *date, char *meal);

bool get_meals_period_from_neis(const char *edu_office, const char *school_name, const char *start_date, const char *end_date, char *meals_json);

// 데이터베이스 함수
bool init_db(void);
void cleanup_db(void);
bool is_user_exists(const char *id);
bool verify_user(const char *id, const char *pw);
bool get_user(const char *id, User *user);
bool add_user(const User *user);
bool update_user(const User *user);
bool delete_user(const char *id, char *response);
bool is_child_registered(const char *child_id, const char *parent_id);
bool db_add_child(const char *child_id, const char *parent_id);
bool db_delete_child(const char *child_id, const char *parent_id);
bool get_children_db(const char *parent_id, Child *children, int *count);
bool get_meal(const char *date, const char *edu_office, const char *school_name, Meal *meal);
#endif