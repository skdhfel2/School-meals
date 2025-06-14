#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <signal.h>
#include "protocol.h"
#include "db_handler.h"
#include "db.h"

#define MAX_CLIENTS 10
#define PORT 8080

static SOCKET server_fd;
static SOCKET client_sockets[MAX_CLIENTS];
static CRITICAL_SECTION clients_mutex;
static volatile sig_atomic_t running = 1;

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
        if (!receive_data(client_socket, buffer, BUFFER_SIZE - 1))
        {
            break;
        }

        printf("수신된 요청: %s\n", buffer);

        // 프로토콜 파싱
        char *cmd = strtok(buffer, CMD_DELIMITER);
        if (cmd == NULL)
        {
            printf("잘못된 요청 형식\n");
            send_response(client_socket, RESP_ERROR, RESP_INVALID_REQUEST, "");
            break;
        }

        printf("명령어: %s\n", cmd);

        char *id = strtok(NULL, CMD_DELIMITER);
        char *pw = strtok(NULL, CMD_DELIMITER); // 비밀번호
        char *edu_office = strtok(NULL, CMD_DELIMITER);
        char *school_name = strtok(NULL, CMD_DELIMITER);
        char *date = strtok(NULL, CMD_DELIMITER);
        char *child_id = strtok(NULL, CMD_DELIMITER);
        char *period = strtok(NULL, CMD_DELIMITER);

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
                    User new_user = {0};
                    strncpy(new_user.id, id, MAX_ID_LEN - 1);
                    strncpy(new_user.pw, pw, MAX_PW_LEN - 1); // 평문 비밀번호
                    strncpy(new_user.name, "", MAX_NAME_LEN - 1);
                    strncpy(new_user.role, ROLE_GENERAL, MAX_ROLE_LEN - 1);
                    strncpy(new_user.edu_office, edu_office, MAX_EDU_OFFICE_LEN - 1);
                    strncpy(new_user.school_name, school_name, MAX_SCHOOL_NAME_LEN - 1);

                    if (add_user(&new_user))
                    {
                        send_response(client_socket, RESP_SUCCESS, RESP_REGISTER_OK, "");
                    }
                    else
                    {
                        send_response(client_socket, RESP_ERROR, RESP_DB_ERROR, "");
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
                Meal meal_data = {0};
                if (get_meal(date, edu_office, school_name, &meal_data))
                {
                    send_response(client_socket, RESP_SUCCESS, RESP_GET_MEAL_OK, meal_data.meal);
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
                    // TODO: NEIS API 연동
                    send_response(client_socket, RESP_SUCCESS, RESP_GET_MULTI_MEAL_OK, "");
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

    // 클라이언트 연결 종료
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

    closesocket(client_socket);
    free(arg);
    return 0;
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

        // 클라이언트 요청 수신
        char buffer[BUFFER_SIZE] = {0};
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0)
        {
            printf("클라이언트 연결 종료 또는 오류\n");
            close_socket(client_socket);
            continue;
        }

        printf("수신된 요청: %s\n", buffer);

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
        SOCKET *client_socket_ptr = malloc(sizeof(SOCKET));
        *client_socket_ptr = client_socket;
        HANDLE thread = CreateThread(NULL, 0, handle_client, client_socket_ptr, 0, NULL);
        if (thread == NULL)
        {
            fprintf(stderr, "스레드 생성 실패, 오류: %lu\n", GetLastError());
            free(client_socket_ptr);
            close_socket(client_socket);
            continue;
        }
        CloseHandle(thread); // 스레드 핸들 닫기 (스레드 종료 기다리지 않음)
    }

    // 정리
    EnterCriticalSection(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (client_sockets[i] != 0)
        {
            close_socket(client_sockets[i]);
        }
    }
    LeaveCriticalSection(&clients_mutex);

    close_socket(server_fd);
    close_db();
    cleanup_network();
    DeleteCriticalSection(&clients_mutex);
    return 0;
}