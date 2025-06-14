#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"
#include "protocol.h"
#include "db_handler.h"
#include "neis_api.h"

#define MAX_CLIENTS 10
#define PORT 8080

static SOCKET server_fd;
static SOCKET client_sockets[MAX_CLIENTS];
static CRITICAL_SECTION clients_mutex;
static volatile sig_atomic_t running = 1;

// 함수 선언
bool handle_register_general(SOCKET client_socket, const char *id, const char *pw, const char *edu_office, const char *school_name);
bool send_response(SOCKET client_socket, int status, const char *message, const char *data);

void signal_handler(int sig)
{
    running = 0;
}

DWORD WINAPI handle_client(LPVOID arg)
{
    SOCKET client_socket = *(SOCKET *)arg;
    char buffer[BUFFER_SIZE] = {0};

    while (running)
    {
        // 클라이언트 요청 수신
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("클라이언트 연결 종료 또는 오류\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("수신된 요청: %s\n", buffer);

        // 프로토콜 파싱
        char *cmd = strtok(buffer, CMD_DELIMITER);
        if (cmd == NULL)
        {
            printf("잘못된 요청 형식\n");
            send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            continue;
        }

        printf("명령어: %s\n", cmd);

        // 파라미터 파싱
        char *id = NULL;
        char *pw = NULL;
        char *edu_office = NULL;
        char *school_name = NULL;
        char *date = NULL;
        char *child_id = NULL;
        char *period = NULL;

        if (strcmp(cmd, CMD_GET_MEAL) == 0)
        {
            edu_office = strtok(NULL, CMD_DELIMITER);
            school_name = strtok(NULL, CMD_DELIMITER);
            date = strtok(NULL, CMD_DELIMITER);

            printf("파싱된 파라미터:\n");
            printf("교육청: %s\n", edu_office);
            printf("학교: %s\n", school_name);
            printf("날짜: %s\n", date);
        }
        else if (strcmp(cmd, CMD_GET_MULTI_MEAL) == 0)
        {
            edu_office = strtok(NULL, CMD_DELIMITER);
            school_name = strtok(NULL, CMD_DELIMITER);
            period = strtok(NULL, CMD_DELIMITER);

            printf("파싱된 파라미터:\n");
            printf("교육청: %s\n", edu_office);
            printf("학교: %s\n", school_name);
            printf("기간: %s\n", period);
        }
        else
        {
            id = strtok(NULL, CMD_DELIMITER);
            pw = strtok(NULL, CMD_DELIMITER);
            edu_office = strtok(NULL, CMD_DELIMITER);
            school_name = strtok(NULL, CMD_DELIMITER);
            date = strtok(NULL, CMD_DELIMITER);
            child_id = strtok(NULL, CMD_DELIMITER);
            period = strtok(NULL, CMD_DELIMITER);
        }

        if (strcmp(cmd, CMD_LOGIN) == 0)
        {
            if (id && pw)
            {
                if (verify_user(id, pw))
                {
                    User user;
                    if (get_user(id, &user))
                    {
                        char data[256];
                        snprintf(data, sizeof(data), "%s//%s//%s", user.edu_office, user.school_name, user.role);
                        send_response(client_socket, RESP_SUCCESS, RESP_LOGIN_OK, data);
                    }
                    else
                    {
                        send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                    }
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, ERR_INVALID_LOGIN, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_REGISTER_GENERAL) == 0)
        {
            if (id && pw && edu_office && school_name)
            {
                if (is_user_exists(id))
                {
                    send_response(client_socket, RESP_ERROR, ERR_ID_DUPLICATE, "");
                }
                else
                {
                    if (handle_register_general(client_socket, id, pw, edu_office, school_name))
                    {
                        send_response(client_socket, RESP_SUCCESS, RESP_REGISTER_OK, "");
                    }
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_UPDATE) == 0)
        {
            if (id && edu_office && school_name)
            {
                User user = {0};
                strncpy(user.id, id, MAX_ID_LEN - 1);
                strncpy(user.edu_office, edu_office, MAX_EDU_OFFICE_LEN - 1);
                strncpy(user.school_name, school_name, MAX_SCHOOL_NAME_LEN - 1);

                if (update_user(&user))
                {
                    send_response(client_socket, RESP_SUCCESS, RESP_UPDATE_OK, "");
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_DELETE) == 0)
        {
            if (id)
            {
                char response[BUFFER_SIZE] = {0};
                if (delete_user(id, response))
                {
                    send_response(client_socket, RESP_SUCCESS, RESP_DELETE_OK, "");
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_GET_CHILDREN) == 0)
        {
            if (id)
            {
                Child children[MAX_CHILDREN];
                int count = 0;

                if (get_children_db(id, children, &count))
                {
                    // JSON 형식으로 변환
                    char json[BUFFER_SIZE] = "[";
                    for (int i = 0; i < count; i++)
                    {
                        char child_json[256];
                        snprintf(child_json, sizeof(child_json),
                                 "%s{\"id\":\"%s\",\"name\":\"%s\"}",
                                 i > 0 ? "," : "", children[i].id, children[i].name);
                        strncat(json, child_json, sizeof(json) - strlen(json) - 1);
                    }
                    strncat(json, "]", sizeof(json) - strlen(json) - 1);

                    send_response(client_socket, RESP_SUCCESS, RESP_GET_CHILDREN_OK, json);
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_ADD_CHILD) == 0)
        {
            if (id && child_id)
            {
                if (!is_user_exists(child_id))
                {
                    send_response(client_socket, RESP_ERROR, ERR_CHILD_USER_NOT_FOUND, "");
                }
                else if (db_add_child(child_id, id))
                {
                    send_response(client_socket, RESP_SUCCESS, RESP_ADD_CHILD_OK, "");
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_DEL_CHILD) == 0)
        {
            if (id && child_id)
            {
                if (!is_child_registered(child_id, id))
                {
                    send_response(client_socket, RESP_ERROR, ERR_INVALID_CHILD_ID, "");
                }
                else if (db_delete_child(child_id, id))
                {
                    send_response(client_socket, RESP_SUCCESS, RESP_DEL_CHILD_OK, "");
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_CHILD_MEAL) == 0)
        {
            if (id && child_id && date)
            {
                if (!is_child_registered(child_id, id))
                {
                    send_response(client_socket, RESP_ERROR, ERR_INVALID_CHILD_ID, "");
                }
                else
                {
                    User child;
                    if (get_user(child_id, &child))
                    {
                        Meal meal_data = {0};
                        if (get_meal(date, child.edu_office, child.school_name, &meal_data))
                        {
                            send_response(client_socket, RESP_SUCCESS, RESP_GET_CHILD_MEAL_OK, meal_data.meal);
                        }
                        else
                        {
                            send_response(client_socket, RESP_ERROR, ERR_CHILD_MEAL_NOT_FOUND, "");
                        }
                    }
                    else
                    {
                        send_response(client_socket, RESP_ERROR, ERR_CHILD_USER_NOT_FOUND, "");
                    }
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_CHILD_MULTI_MEAL) == 0)
        {
            if (id && child_id && period)
            {
                if (!is_child_registered(child_id, id))
                {
                    send_response(client_socket, RESP_ERROR, ERR_INVALID_CHILD_ID, "");
                }
                else
                {
                    char start_date[11], end_date[11];
                    if (sscanf(period, "%10[^-]-%10s", start_date, end_date) != 2)
                    {
                        send_response(client_socket, RESP_ERROR, ERR_INVALID_DATE_FORMAT, "");
                    }
                    else
                    {
                        // TODO: NEIS API 연동
                        send_response(client_socket, RESP_SUCCESS, RESP_GET_CHILD_MULTI_MEAL_OK, "");
                    }
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_GET_MEAL) == 0)
        {
            if (edu_office && school_name && date)
            {
                char meal[MAX_MEAL_LEN] = {0};
                if (get_meal_from_neis(edu_office, school_name, date, meal))
                {
                    send_response(client_socket, RESP_SUCCESS, "급식 정보 조회 성공", meal);
                }
                else
                {
                    send_response(client_socket, RESP_ERROR, ERR_MEAL_NOT_FOUND, "");
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else if (strcmp(cmd, CMD_GET_MULTI_MEAL) == 0)
        {
            if (edu_office && school_name && period)
            {
                char start_date[11], end_date[11];
                if (sscanf(period, "%10[^-]-%10s", start_date, end_date) != 2)
                {
                    send_response(client_socket, RESP_ERROR, ERR_INVALID_DATE_FORMAT, "");
                }
                else
                {
                    char meals[MAX_MEAL_LEN] = {0};
                    if (get_meals_period_from_neis(edu_office, school_name, start_date, end_date, meals))
                    {
                        send_response(client_socket, RESP_SUCCESS, "기간 급식 정보 조회 성공", meals);
                    }
                    else
                    {
                        send_response(client_socket, RESP_ERROR, ERR_MEAL_NOT_FOUND, "");
                    }
                }
            }
            else
            {
                send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            }
        }
        else
        {
            send_response(client_socket, RESP_ERROR, RESP_UNKNOWN_COMMAND, "");
        }
    }

    // 클라이언트 소켓 정리
    EnterCriticalSection(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] == client_socket)
        {
            client_sockets[i] = 0;
            break;
        }
    }
    LeaveCriticalSection(&clients_mutex);

    close_socket(client_socket);
    return 0;
}

bool handle_register_general(SOCKET client_socket, const char *id, const char *pw, const char *edu_office, const char *school_name)
{
    printf("회원가입 처리 시작: ID=%s\n", id);

    // 학교 코드 조회
    char edu_code[10] = {0};
    char school_code[20] = {0};
    
    if (!resolve_school_code(school_name, edu_code, school_code))
    {
        printf("❌ 학교 정보를 찾을 수 없습니다: %s\n", school_name);
        send_response(client_socket, RESP_ERROR, "학교 정보를 찾을 수 없습니다.", "");
        return false;
    }

    // 사용자 정보 생성
    User user = {0};
    strncpy(user.id, id, MAX_ID_LEN - 1);
    strncpy(user.pw, pw, MAX_PW_LEN - 1);
    strncpy(user.name, "", MAX_NAME_LEN - 1);
    strncpy(user.role, ROLE_GENERAL, MAX_ROLE_LEN - 1);
    strncpy(user.edu_office, edu_code, MAX_EDU_OFFICE_LEN - 1);
    strncpy(user.school_name, school_code, MAX_SCHOOL_NAME_LEN - 1);

    printf("사용자 정보 생성 완료\n");

    // 데이터베이스에 사용자 추가
    if (add_user(&user))
    {
        printf("회원가입 성공\n");
        return true;
    }
    else
    {
        printf("회원가입 실패: 데이터베이스 오류\n");
        return false;
    }
}

bool send_response(SOCKET client_socket, int status, const char *message, const char *data)
{
    char response[BUFFER_SIZE];

    if (data && strlen(data) > 0)
    {
        // 줄바꿈 제거 없이 그대로 보냄
        snprintf(response, sizeof(response), "%d%s%s%s%s", status, CMD_DELIMITER, message, CMD_DELIMITER, data);
    }
    else
    {
        snprintf(response, sizeof(response), "%d%s%s", status, CMD_DELIMITER, message);
    }

    int bytes_sent = send(client_socket, response, strlen(response), 0);
    return bytes_sent >= 0;
}

int main()
{
    // SIGINT 핸들러 설정
    signal(SIGINT, signal_handler);

    // 뮤텍스 초기화
    InitializeCriticalSection(&clients_mutex);

    // 클라이언트 소켓 배열 초기화
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    // 네트워크 초기화
    if (!init_network())
    {
        fprintf(stderr, "네트워크 초기화 실패\n");
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    // DB 초기화
    if (!init_db())
    {
        fprintf(stderr, "DB 초기화 실패\n");
        cleanup_network();
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    // 소켓 생성
    server_fd = create_socket();
    if (server_fd == INVALID_SOCKET)
    {
        fprintf(stderr, "소켓 생성 실패, 오류: %d\n", WSAGetLastError());
        cleanup_network();
        close_db();
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    // 소켓 옵션 설정
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        fprintf(stderr, "소켓 옵션 설정 실패, 오류: %d\n", WSAGetLastError());
        close_socket(server_fd);
        cleanup_network();
        close_db();
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    // 주소 설정
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
    {
        fprintf(stderr, "바인딩 실패, 오류: %d\n", WSAGetLastError());
        close_socket(server_fd);
        cleanup_network();
        close_db();
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    // 리스닝
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        fprintf(stderr, "리스닝 실패, 오류: %d\n", WSAGetLastError());
        close_socket(server_fd);
        cleanup_network();
        close_db();
        DeleteCriticalSection(&clients_mutex);
        return 1;
    }

    printf("서버가 포트 %d에서 실행 중입니다...\n", PORT);

    // 클라이언트 연결 처리
    while (running)
    {
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET)
        {
            if (!running)
                break;
            fprintf(stderr, "클라이언트 연결 수락 실패, 오류: %d\n", WSAGetLastError());
            continue;
        }

        printf("클라이언트 연결됨: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 클라이언트 소켓 저장
        EnterCriticalSection(&clients_mutex);
        int i;
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_sockets[i] == 0)
            {
                client_sockets[i] = client_socket;
                break;
            }
        }
        LeaveCriticalSection(&clients_mutex);

        if (i == MAX_CLIENTS)
        {
            fprintf(stderr, "최대 클라이언트 수 초과\n");
            close_socket(client_socket);
            continue;
        }

        // 클라이언트 처리 스레드 생성
        HANDLE thread = CreateThread(NULL, 0, handle_client, &client_sockets[i], 0, NULL);
        if (thread == NULL)
        {
            fprintf(stderr, "스레드 생성 실패\n");
            close_socket(client_sockets[i]);
            client_sockets[i] = 0;
        }
        else
        {
            CloseHandle(thread);
        }
    }

    // 정리
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] != 0)
        {
            close_socket(client_sockets[i]);
        }
    }

    close_socket(server_fd);
    cleanup_network();
    close_db();
    DeleteCriticalSection(&clients_mutex);

    return 0;
}