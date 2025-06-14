#include "handlers.h"
#include "menu.h"
#include "network.h"
#include <stdio.h>
#include <string.h>

// 전역 변수
extern SOCKET client_socket;
extern char current_user_id[MAX_ID_LEN];
extern char current_user_role[MAX_ROLE_LEN];
extern char current_user_edu_office[MAX_EDU_OFFICE_LEN];
extern char current_user_school[MAX_SCHOOL_NAME_LEN];

// 로그인 관련 핸들러
bool handle_login(const char *id, const char *pw, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s",
             CMD_LOGIN, CMD_DELIMITER, id, CMD_DELIMITER, pw);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    if (!receive_response(response))
    {
        strcpy(response, "서버 응답 수신 실패");
        return false;
    }

    // 응답 파싱
    char *status = strtok(response, CMD_DELIMITER);
    if (strcmp(status, RESP_SUCCESS_STR) == 0)
    {
        // 사용자 정보 저장
        strncpy(current_user_id, id, MAX_ID_LEN - 1);
        current_user_id[MAX_ID_LEN - 1] = '\0';

        char *edu_office = strtok(NULL, CMD_DELIMITER);
        char *school_name = strtok(NULL, CMD_DELIMITER);
        char *role = strtok(NULL, CMD_DELIMITER);

        if (edu_office && school_name && role)
        {
            strncpy(current_user_edu_office, edu_office, MAX_EDU_OFFICE_LEN - 1);
            current_user_edu_office[MAX_EDU_OFFICE_LEN - 1] = '\0';

            strncpy(current_user_school, school_name, MAX_SCHOOL_NAME_LEN - 1);
            current_user_school[MAX_SCHOOL_NAME_LEN - 1] = '\0';

            strncpy(current_user_role, role, MAX_ROLE_LEN - 1);
            current_user_role[MAX_ROLE_LEN - 1] = '\0';

            return true;
        }
    }

    return false;
}

bool handle_register(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_REGISTER_GENERAL, CMD_DELIMITER, id, CMD_DELIMITER, pw,
             CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    if (!receive_response(response))
    {
        strcpy(response, "서버 응답 수신 실패");
        return false;
    }

    // 응답 파싱
    char *status = strtok(response, CMD_DELIMITER);
    if (strcmp(status, RESP_SUCCESS_STR) == 0)
    {
        return true;
    }

    return false;
}

void handle_logout(void)
{
    memset(current_user_id, 0, sizeof(current_user_id));
    memset(current_user_role, 0, sizeof(current_user_role));
    memset(current_user_edu_office, 0, sizeof(current_user_edu_office));
    memset(current_user_school, 0, sizeof(current_user_school));
}

// 급식 조회 핸들러
bool get_meal_from_neis(const char *edu_office, const char *school_name, const char *date, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s",
             CMD_GET_MEAL, CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name, CMD_DELIMITER, date);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool get_meals_period_from_neis(const char *edu_office, const char *school_name, const char *start_date, const char *end_date, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_GET_MULTI_MEAL, CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name,
             CMD_DELIMITER, start_date, CMD_DELIMITER, end_date);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool get_child_meal(const char *parent_id, const char *child_id, const char *date, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s",
             CMD_CHILD_MEAL, CMD_DELIMITER, parent_id, CMD_DELIMITER, child_id, CMD_DELIMITER, date);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool get_child_meals_period(const char *parent_id, const char *child_id, const char *start_date, const char *end_date, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_CHILD_MULTI_MEAL, CMD_DELIMITER, parent_id, CMD_DELIMITER, child_id,
             CMD_DELIMITER, start_date, CMD_DELIMITER, end_date);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

// 자녀 관리 핸들러
bool add_child(const char *parent_id, const char *child_id, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s",
             CMD_ADD_CHILD, CMD_DELIMITER, parent_id, CMD_DELIMITER, child_id);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool delete_child(const char *parent_id, const char *child_id, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s",
             CMD_DEL_CHILD, CMD_DELIMITER, parent_id, CMD_DELIMITER, child_id);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool get_children(const char *parent_id, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s",
             CMD_GET_CHILDREN, CMD_DELIMITER, parent_id);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

// 사용자 관리 핸들러
bool handle_add_user(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_REGISTER_GENERAL, CMD_DELIMITER, id, CMD_DELIMITER, pw,
             CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool handle_update_user(const char *id, const char *pw, const char *edu_office, const char *school_name, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_UPDATE, CMD_DELIMITER, id, CMD_DELIMITER, pw,
             CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}

bool handle_delete_user(const char *id, char *response)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s",
             CMD_DELETE, CMD_DELIMITER, id);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "서버 통신 오류");
        return false;
    }

    return receive_response(response);
}