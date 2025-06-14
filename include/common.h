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

#define close closesocket

#define BUFFER_SIZE 4096
#define MAX_ID_LEN 32
#define MAX_PW_LEN 32
#define MAX_EDU_OFFICE_LEN 32
#define MAX_SCHOOL_NAME_LEN 32
#define MAX_ROLE_LEN 16
#define MAX_CHILD_NAME_LEN 32
#define MAX_NAME_LEN 32
#define MAX_MEAL_LEN 2048
#define MAX_PERIOD_DAYS 7 // ✅ 최대 급식 조회 기간 (7일)
#define MAX_CHILDREN 10   // ✅ 최대 자녀 수

// 성공/실패 응답 코드 (정수 값)
#define SUCCESS 1        // 성공 (논리적으로 TRUE 의미)
#define ERROR 0          // 일반 에러
#define DUPLICATE_CODE 2 // 중복 ID 등 처리 시

// 문자열 응답용 상수
#define SUCCESS_STR "SUCCESS"
#define ERROR_STR "ERROR"
#define DUPLICATE_CODE_STR "DUPLICATE"

#define CMD_DELIMITER "//"

// 에러 메시지
#define ERR_INVALID_ID "잘못된 ID입니다"
#define ERR_INVALID_PW "잘못된 비밀번호입니다"
#define ERR_USER_NOT_FOUND "사용자를 찾을 수 없습니다"
#define ERR_DUPLICATE_ID "이미 존재하는 ID입니다"
#define ERR_INVALID_CHILD_ID "잘못된 자녀 ID입니다"
#define ERR_CHILD_USER_NOT_FOUND "자녀 사용자를 찾을 수 없습니다"
#define ERR_CHILD_MEAL_NOT_FOUND "자녀의 급식 정보를 찾을 수 없습니다"

// 명령어
#define CMD_LOGIN "LOGIN"
#define CMD_REGISTER_GENERAL "REGISTER_GENERAL"
#define CMD_UPDATE "UPDATE"
#define CMD_DELETE "DELETE"
#define CMD_LOGOUT "LOGOUT"
#define CMD_QUIT "QUIT"
#define CMD_GET_CHILDREN "GET_CHILDREN"
#define CMD_ADD_CHILD "ADD_CHILD"
#define CMD_DEL_CHILD "DEL_CHILD"
#define CMD_CHILD_MEAL "CHILD_MEAL"
#define CMD_CHILD_MULTI_MEAL "CHILD_MULTI_MEAL"
#define CMD_GET_MEAL "GET_MEAL"
#define CMD_GET_MULTI_MEAL "GET_MULTI_MEAL"

// 역할
#define ROLE_PARENT "PARENT"
#define ROLE_GENERAL "GENERAL"

// 서버 설정
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

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
    char pw[MAX_PW_LEN];
    char name[MAX_NAME_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
} Child;

typedef struct
{
    char date[11];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char meal[MAX_MEAL_LEN];
} Meal;

typedef struct
{
    int status;
    char message[256];
    char data[BUFFER_SIZE];
} Response;

// 유틸리티 함수
bool validate_id(const char *id);
bool validate_password(const char *password);
bool validate_date_format(const char *date);
void trim_string(char *str);

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

#ifdef _WIN32
#endif

#endif // COMMON_H