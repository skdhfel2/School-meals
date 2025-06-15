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

// 허용된 교육청 이름 목록
const char *valid_edu_offices[] = {
    "서울특별시교육청", "부산광역시교육청", "대구광역시교육청", "인천광역시교육청",
    "광주광역시교육청", "대전광역시교육청", "울산광역시교육청", "세종특별자치시교육청",
    "경기도교육청", "강원도교육청", "충청북도교육청", "충청남도교육청",
    "전라북도교육청", "전라남도교육청", "경상북도교육청", "경상남도교육청", "제주특별자치도교육청"};
#define EDU_OFFICE_COUNT (sizeof(valid_edu_offices) / sizeof(valid_edu_offices[0]))

bool is_valid_edu_office(const char *name)
{
    for (int i = 0; i < EDU_OFFICE_COUNT; i++)
    {
        if (strcmp(name, valid_edu_offices[i]) == 0)
            return true;
    }
    return false;
}

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
    if (status && atoi(status) == RESP_SUCCESS)
    {
        // 사용자 정보 저장
        strncpy(current_user_id, id, MAX_ID_LEN - 1);
        current_user_id[MAX_ID_LEN - 1] = '\0';

        char *message = strtok(NULL, CMD_DELIMITER); // "로그인 성공" → 사용 X, 건너뜀
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
    if (!is_valid_edu_office(edu_office))
    {
        strcpy(response, "❌ 올바른 교육청 이름이 아닙니다.\n예시: 서울특별시교육청, 경기도교육청");
        return false;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_REGISTER_GENERAL, CMD_DELIMITER, id, CMD_DELIMITER, pw,
             CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(response, "❌ 서버와의 통신 중 문제가 발생했습니다. 다시 시도해주세요.");
        return false;
    }

    if (!receive_response(response))
    {
        strcpy(response, "❌ 서버로부터 응답을 받지 못했습니다. 네트워크를 확인해주세요.");
        return false;
    }

    // 응답 파싱
    char *status = strtok(response, CMD_DELIMITER); // 예: 1 또는 0
    char *message = strtok(NULL, CMD_DELIMITER);    // 예: REGISTER_OK, ID_DUPLICATE 등

    if (status == NULL || message == NULL)
    {
        strcpy(response, "❌ 서버 응답 형식이 잘못되었습니다.");
        return false;
    }

    int code = atoi(status);

    if (code == RESP_SUCCESS && strcmp(message, RESP_REGISTER_OK) == 0)
    {
        strcpy(response, "🎉 회원가입이 성공적으로 완료되었습니다!\n이제 로그인하여 서비스를 이용해보세요.");
        return true;
    }
    else if (code == RESP_ERROR && strcmp(message, ERR_ID_DUPLICATE) == 0)
    {
        strcpy(response, "⚠️ 입력한 아이디는 이미 사용 중입니다.\n다른 아이디를 입력해주세요.");
        return false;
    }
    else
    {
        snprintf(response, BUFFER_SIZE, "❌ 회원가입 중 오류 발생: %s", message);
        return false;
    }
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
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s-%s",
             CMD_GET_MULTI_MEAL, CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name,
             CMD_DELIMITER, start_date, end_date);

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

// TODO: 추후 부모 사용자 기능 구현 시 활성화 예정
/*
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
*/

// 사용자 관리 핸들러
bool handle_add_user(const char *id, const char *pw,
                     const char *edu_office, const char *school_name,
                     int *status, char *message)
{
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s%s%s%s%s%s%s%s%s",
             CMD_REGISTER_GENERAL, CMD_DELIMITER, id, CMD_DELIMITER, pw,
             CMD_DELIMITER, edu_office, CMD_DELIMITER, school_name);

    if (!send_data(client_socket, buffer, strlen(buffer)))
    {
        strcpy(message, "서버 통신 오류");
        *status = RESP_ERROR;
        return false;
    }

    char response[BUFFER_SIZE];
    if (!receive_response(response))
    {
        strcpy(message, "서버 응답 없음");
        *status = RESP_ERROR;
        return false;
    }

    // 응답 파싱
    char *status_str = strtok(response, CMD_DELIMITER); // "0", "1", "2"
    char *msg = strtok(NULL, CMD_DELIMITER);            // 실제 메시지

    if (!status_str || !msg)
    {
        strcpy(message, "서버 응답 파싱 오류");
        *status = RESP_ERROR;
        return false;
    }

    *status = atoi(status_str);
    strcpy(message, msg);

    return true;
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

void print_edu_office_guide()
{
    printf("※ 교육청 이름은 아래 중 하나를 정확히 입력해야 합니다.\n");
    printf("서울특별시교육청, 부산광역시교육청, 대구광역시교육청, 인천광역시교육청,\n");
    printf("광주광역시교육청, 대전광역시교육청, 울산광역시교육청, 세종특별자치시교육청,\n");
    printf("경기도교육청, 강원도교육청, 충청북도교육청, 충청남도교육청,\n");
    printf("전라북도교육청, 전라남도교육청, 경상북도교육청, 경상남도교육청, 제주특별자치도교육청\n");
}