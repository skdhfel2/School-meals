#include "common.h"
#include <stdio.h>
#include <string.h>
#include "protocol.h"
#include "db.h"
#include <winsock2.h>
#include "neis_api.h"
// 충분한 응답 버퍼 확보
#define RESPONSE_SIZE 2048

bool is_valid_id(const char *id)
{
    int len = strlen(id);
    if (len < 4 || len > 16)
        return false;
    for (int i = 0; i < len; i++)
    {
        if (!isalnum(id[i]))
            return false;
    }
    return true;
}

bool is_valid_password(const char *pw)
{
    if (strlen(pw) != 4)
        return false;
    for (int i = 0; i < 4; i++)
    {
        if (!isdigit(pw[i]))
            return false;
    }
    return true;
}

void handle_register_parent(int client_socket, char *id, char *pw)
{
    char response[RESPONSE_SIZE] = {0};

    //  1. 입력값 유효성 검사 추가
    if (!is_valid_id(id))
    {
        sprintf(response, "%s//%s", RESP_ERROR, "유효하지 않은 아이디입니다.");
    }
    else if (!is_valid_password(pw))
    {
        sprintf(response, "%s//%s", RESP_ERROR, "비밀번호는 숫자 4자리여야 합니다.");
    }
    else if (is_user_exists(id))
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_ID_DUPLICATE);
    }
    else
    {
        User user;
        strncpy(user.id, id, MAX_ID_LEN - 1);
        user.id[MAX_ID_LEN - 1] = '\0';
        strncpy(user.pw, pw, MAX_PW_LEN - 1);
        user.pw[MAX_PW_LEN - 1] = '\0';
        strncpy(user.role, ROLE_PARENT, sizeof(user.role) - 1);
        user.role[sizeof(user.role) - 1] = '\0';
        user.edu_office[0] = '\0';
        user.school_name[0] = '\0';

        if (add_user(&user))
            sprintf(response, "%s//%s", RESP_SUCCESS, CMD_REGISTER_PARENT);
        else
            sprintf(response, "%s//%s", RESP_ERROR, "DB_ERROR");
    }

    if (send(client_socket, response, strlen(response), 0) < 0)
    {
        perror("send failed (register_parent)");
    }
}

void handle_login(int client_socket, char *id, char *pw) // 로그인 요청을 처리하고, 성공 또는 실패 메시지를 클라이언트에게 보내는 함수
{
    char response[RESPONSE_SIZE] = {0};

    if (verify_user(id, pw)) // ID와 비밀번호가 일치하는지 확인
    {
        User user;               // 사용자 정보를 저장할 구조체
        if (get_user(id, &user)) // 사용자 정보를 가져오는 함수
        {
            snprintf(response, RESPONSE_SIZE, RESP_LOGIN_FORMAT,
                     RESP_SUCCESS, user.edu_office, user.school_name, user.role);
        }
        else // 사용자 정보를 가져오지 못하면 오류 응답
        {
            sprintf(response, "%s//%s", RESP_ERROR, "DB_ERROR");
        }
    }
    else // ID와 비밀번호가 일치하지 않으면 오류 응답
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_INVALID_LOGIN);
    }

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (login)");
    }
}

void handle_meal(int client_socket, char *edu_office, char *school_name, char *date) // 단일 급식 정보 요청을 처리하는 함수
{
    char response[RESPONSE_SIZE] = {0};
    char meal[MAX_MEAL_LEN] = {0};

    if (get_meal_from_neis(edu_office, school_name, date, meal)) // NEIS API를 통해 급식 정보를 조회
    {
        snprintf(response, RESPONSE_SIZE, RESP_MEAL_FORMAT, RESP_SUCCESS, meal);
    }
    else // 급식 정보를 조회하지 못하면 오류 응답
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_MEAL_NOT_FOUND);
    }

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (meal)");
    }
}

void handle_multi_meal(int client_socket, char *edu_office, char *school_name,
                       char *start_date, char *end_date) // 여러 날의 급식 정보를 요청하는 함수
{
    char response[RESPONSE_SIZE] = {0};
    char meals_json[MAX_MEAL_LEN] = {0};

    if (get_meals_period_from_neis(edu_office, school_name, start_date, end_date, meals_json)) // NEIS API를 통해 여러 날의 급식 정보를 조회
    {
        snprintf(response, RESPONSE_SIZE, RESP_MULTI_MEAL_FORMAT, RESP_SUCCESS, meals_json); // 여러 날의 급식 정보를 포함한 응답
    }
    else // 급식 정보를 조회하지 못하면 오류 응답
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_TOO_LONG_PERIOD);
    }

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (multi_meal)");
    }
}

void handle_other_meal(int client_socket, char *edu_office, char *school_name, char *date) // 다른 학교 급식 정보를 요청하는 함수
{
    char response[RESPONSE_SIZE] = {0};
    char meal[MAX_MEAL_LEN] = {0};

    // 학교 코드 조회
    char edu_code[10] = {0};
    char school_code[20] = {0};

    if (!resolve_school_code(school_name, edu_code, school_code))
    {
        printf("❌ 학교 정보를 찾을 수 없습니다: %s\n", school_name);
        sprintf(response, "%s//%s", RESP_ERROR, "학교 정보를 찾을 수 없습니다.");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    if (get_meal_from_neis(edu_code, school_code, date, meal))
    {
        snprintf(response, RESPONSE_SIZE, RESP_OTHER_MEAL_FORMAT, RESP_SUCCESS, meal); // 다른 학교 급식 정보를 포함한 응답
    }
    else // 급식 정보를 조회하지 못하면 오류 응답
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_MEAL_NOT_FOUND);
    }

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (other_meal)");
    }
}

void handle_multi_other_meal(int client_socket, char *edu_office, char *school_name,
                             char *start_date, char *end_date) // 여러 날의 다른 급식 정보를 요청하는 함수
{
    char response[RESPONSE_SIZE] = {0};
    char meals_json[MAX_MEAL_LEN] = {0};

    // 학교 코드 조회
    char edu_code[10] = {0};
    char school_code[20] = {0};

    if (!resolve_school_code(school_name, edu_code, school_code))
    {
        printf("❌ 학교 정보를 찾을 수 없습니다: %s\n", school_name);
        sprintf(response, "%s//%s", RESP_ERROR, "학교 정보를 찾을 수 없습니다.");
        send(client_socket, response, strlen(response), 0);
        return;
    }

    if (get_meals_period_from_neis(edu_code, school_code, start_date, end_date, meals_json)) // NEIS API를 통해 여러 날의 급식 정보를 조회
    {
        snprintf(response, RESPONSE_SIZE, RESP_MULTI_OTHER_MEAL_FORMAT, RESP_SUCCESS, meals_json); // 여러 날의 다른 급식 정보를 포함한 응답
    }
    else
    {
        sprintf(response, "%s//%s", RESP_ERROR, ERR_TOO_LONG_PERIOD); // 급식 정보를 조회하지 못하면 오류 응답
    }

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (multi_other_meal)");
    }
}

void handle_quit(int client_socket) // 클라이언트가 연결을 종료할 때 호출되는 함수
{
    char response[RESPONSE_SIZE] = {0};
    sprintf(response, "%s//%s", RESP_SUCCESS, CMD_QUIT);

    if (send(client_socket, response, strlen(response), 0) < 0) // 응답 메시지를 클라이언트에게 전송
    {
        perror("send failed (quit)");
    }
}
