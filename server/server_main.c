#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "protocol.h"
#include "db.h"
#include "neis_api.c"

#define PORT 8080

volatile sig_atomic_t running = 1; // 서버가 실행 중인지 여부를 나타내는 플래그

void signal_handler(int sig)
{
    running = 0;
}

void handle_client(SOCKET client_socket) // 클라이언트와의 연결을 처리하는 함수
{
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};

    while (1) // 클라이언트로부터 명령어를 수신하고 처리하는 루프
    {
        memset(response, 0, sizeof(response)); // 응답 버퍼 초기화

        if (!receive_data(client_socket, buffer, BUFFER_SIZE - 1)) // 클라이언트로부터 데이터를 수신
        {
            printf("클라이언트와의 연결이 끊겼습니다.\n");
            break;
        }

        // 명령어 파싱 및 처리
        char *cmd = strtok(buffer, CMD_DELIMITER);
        if (cmd == NULL)
        {
            sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
            if (!send_data(client_socket, response, strlen(response)))
            {
                printf("클라이언트에게 응답을 보내는 데 실패했습니다.\n");
                break;
            }
            continue;
        }

        // 명령어 처리
        if (strcmp(cmd, CMD_REGISTER_GENERAL) == 0)
        {
            // 일반 회원가입 처리
            char *id = strtok(NULL, CMD_DELIMITER);
            char *pw = strtok(NULL, CMD_DELIMITER);
            char *name = strtok(NULL, CMD_DELIMITER); // 이름 추가
            char *edu_office_name = strtok(NULL, CMD_DELIMITER);
            char *school_name = strtok(NULL, CMD_DELIMITER);

            if (id && pw && name && edu_office_name && school_name) // 모든 필수 정보가 제공되었는지 확인
            {
                if (is_user_exists(id)) // 사용자 ID가 이미 존재하는지 확인
                {
                    sprintf(response, "%s//%s", RESP_ERROR, ERR_ID_DUPLICATE);
                }
                else // 사용자 ID가 존재하지 않으면 새 사용자 등록
                {
                    // 1. 교육청 이름 → 코드 변환
                    const char* edu_code = get_edu_office_code(edu_office_name);
                    if (!edu_code) {
                        sprintf(response, "%s//%s", RESP_ERROR, "EDU_OFFICE_CODE_FAIL");
                        send_data(client_socket, response, strlen(response));
                        break;
                    }
                    // 2. 학교 이름 → 학교 코드 변환
                    char school_code[16];
                    if (!get_school_code(edu_code, school_name, school_code, sizeof(school_code), NEIS_API_KEY)) {
                        sprintf(response, "%s//%s", RESP_ERROR, "SCHOOL_CODE_FAIL");
                        send_data(client_socket, response, strlen(response));
                        break;
                    }
                    User user;
                    strncpy(user.id, id, MAX_ID_LEN - 1);
                    user.id[MAX_ID_LEN - 1] = '\0';

                    strncpy(user.pw, pw, MAX_PW_LEN - 1);
                    user.pw[MAX_PW_LEN - 1] = '\0';

                    strncpy(user.name, name, MAX_NAME_LEN - 1); // 이름 설정
                    user.name[MAX_NAME_LEN - 1] = '\0';

                    strncpy(user.role, ROLE_GENERAL, MAX_ROLE_LEN - 1);
                    user.role[MAX_ROLE_LEN - 1] = '\0';

                    strncpy(user.edu_office, edu_code, MAX_EDU_OFFICE_LEN - 1);
                    user.edu_office[MAX_EDU_OFFICE_LEN - 1] = '\0';

                    strncpy(user.school_name, school_code, MAX_SCHOOL_NAME_LEN - 1);
                    user.school_name[MAX_SCHOOL_NAME_LEN - 1] = '\0';

                    if (add_user(&user))
                    {
                        sprintf(response, "%s//%s", RESP_SUCCESS, RESP_REGISTER_OK);
                    }
                    else
                    {
                        sprintf(response, "%s//%s", RESP_ERROR, RESP_DB_ERROR);
                    }
                }
            }
            else
            {
                sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
            }
        }
        else if (strcmp(cmd, CMD_REGISTER_PARENT) == 0)
        {
            // 부모 회원가입 처리
            char *id = strtok(NULL, CMD_DELIMITER);
            char *pw = strtok(NULL, CMD_DELIMITER);
            char *name = strtok(NULL, CMD_DELIMITER); // 이름 추가

            if (id && pw && name)
            {
                if (is_user_exists(id))
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

                    strncpy(user.name, name, MAX_NAME_LEN - 1); // 이름 설정
                    user.name[MAX_NAME_LEN - 1] = '\0';

                    strncpy(user.role, ROLE_PARENT, MAX_ROLE_LEN - 1);
                    user.role[MAX_ROLE_LEN - 1] = '\0';

                    strncpy(user.edu_office, "", MAX_EDU_OFFICE_LEN - 1);
                    user.edu_office[MAX_EDU_OFFICE_LEN - 1] = '\0';

                    strncpy(user.school_name, "", MAX_SCHOOL_NAME_LEN - 1);
                    user.school_name[MAX_SCHOOL_NAME_LEN - 1] = '\0';

                    if (add_user(&user))
                    {
                        sprintf(response, "%s//%s", RESP_SUCCESS, RESP_REGISTER_OK);
                    }
                    else
                    {
                        sprintf(response, "%s//%s", RESP_ERROR, RESP_DB_ERROR);
                    }
                }
            }
            else
            {
                sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
            }
        }
        else if (strcmp(cmd, CMD_GET_MEAL) == 0)
        {
            char *edu_office = strtok(NULL, CMD_DELIMITER);
            char *school_name = strtok(NULL, CMD_DELIMITER);
            char *date = strtok(NULL, CMD_DELIMITER);

            if (edu_office && school_name && date)
            {
                // 이미 코드이므로 변환 없이 바로 사용
                handle_meal(client_socket, edu_office, school_name, date);
            }
            else
            {
                sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
                send_data(client_socket, response, strlen(response));
            }
        }
        else if (strcmp(cmd, CMD_GET_MULTI_MEAL) == 0)
        {
            char *edu_office = strtok(NULL, CMD_DELIMITER);
            char *school_name = strtok(NULL, CMD_DELIMITER);
            char *start_date = strtok(NULL, CMD_DELIMITER);
            char *end_date = strtok(NULL, CMD_DELIMITER);

            if (edu_office && school_name && start_date && end_date)
            {
                // 이미 코드이므로 변환 없이 바로 사용
                handle_multi_meal(client_socket, edu_office, school_name, start_date, end_date);
            }
            else
            {
                sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
                send_data(client_socket, response, strlen(response));
            }
        }
        else if (strcmp(cmd, CMD_OTHER_MEAL) == 0)
        {
            char *edu_office_name = strtok(NULL, CMD_DELIMITER);
            char *school_name = strtok(NULL, CMD_DELIMITER);
            char *date = strtok(NULL, CMD_DELIMITER);
            if (edu_office_name && school_name && date)
            {
                char edu_code[10] = {0};
                char school_code[20] = {0};
                if (!resolve_school_code(school_name, edu_code, school_code))
                {
                    send_response(client_socket, RESP_ERROR, "학교 정보를 찾을 수 없습니다.", "");
                    return 0;
                }
                char meal[MAX_MEAL_LEN] = {0};
                if (get_meal_from_neis(edu_code, school_code, date, meal))
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
        else if (strcmp(cmd, CMD_MULTI_OTHER_MEAL) == 0)
        {
            char *edu_office_name = strtok(NULL, CMD_DELIMITER);
            char *school_name = strtok(NULL, CMD_DELIMITER);
            char *start_date = strtok(NULL, CMD_DELIMITER);
            char *end_date = strtok(NULL, CMD_DELIMITER);
            if (edu_office_name && school_name && start_date && end_date)
            {
                char edu_code[10] = {0};
                char school_code[20] = {0};
                if (!resolve_school_code(school_name, edu_code, school_code))
                {
                    sprintf(response, "%s//%s", RESP_ERROR, "학교 정보를 찾을 수 없습니다.");
                    send_data(client_socket, response, strlen(response));
                    continue;
                }
                handle_multi_meal(client_socket, edu_code, school_code, start_date, end_date);
            }
            else
            {
                sprintf(response, "%s//%s", RESP_ERROR, RESP_INVALID_REQUEST);
                send_data(client_socket, response, strlen(response));
            }
        }
        else
        {
            sprintf(response, "%s//%s", RESP_ERROR, RESP_UNKNOWN_COMMAND);
        }

        if (!send_data(client_socket, response, strlen(response))) // 응답 메시지를 클라이언트에게 전송
        {
            printf("클라이언트에게 응답을 보내는 데 실패했습니다.\n");
            break;
        }
    }

    close_socket(client_socket); // 클라이언트 소켓 닫기
}

int main()
{
    // SIGINT 핸들러 설정
    signal(SIGINT, signal_handler);

    // 네트워크 초기화
    if (!init_network())
    {
        fprintf(stderr, "Failed to initialize network\n");
        return 1;
    }

    // DB 초기화
    if (!init_db())
    {
        fprintf(stderr, "Failed to initialize database\n");
        cleanup_network();
        return 1;
    }

    // 소켓 생성
    SOCKET server_fd = create_socket();
    if (server_fd == INVALID_SOCKET)
    {
        fprintf(stderr, "Failed to create socket\n");
        cleanup_network();
        close_db();
        return 1;
    }

    // 소켓 옵션 설정
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to set socket options\n");
        close_socket(server_fd);
        cleanup_network();
        close_db();
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
        fprintf(stderr, "Failed to bind\n");
        close_socket(server_fd);
        cleanup_network();
        close_db();
        return 1;
    }

    // 리스닝
    if (listen(server_fd, 3) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to listen\n");
        close_socket(server_fd);
        cleanup_network();
        close_db();
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

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
            fprintf(stderr, "Failed to accept connection\n");
            continue;
        }

        handle_client(client_socket);
    }

    // 서버 종료
    close_socket(server_fd);
    close_db();
    cleanup_network();
    return 0;
}