#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include "protocol.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int connect_to_server(void)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    return sock;
}

void send_command(SOCKET sock, const char *cmd)
{
    send(sock, cmd, strlen(cmd), 0);
}

void receive_response(SOCKET sock, char *buffer)
{
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE - 1, 0);
}

void handle_register_general(SOCKET sock)
{
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char buffer[BUFFER_SIZE];

    printf("아이디: ");
    scanf("%s", id);
    printf("비밀번호: ");
    scanf("%s", pw);
    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s//%s",
             CMD_REGISTER_GENERAL, id, pw, edu_office, school_name);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_register_parent(SOCKET sock)
{
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
    char buffer[BUFFER_SIZE];

    printf("아이디: ");
    scanf("%s", id);
    printf("비밀번호: ");
    scanf("%s", pw);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s",
             CMD_REGISTER_PARENT, id, pw);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_login(int sock)
{
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
    char buffer[BUFFER_SIZE];

    printf("아이디: ");
    scanf("%s", id);
    printf("비밀번호: ");
    scanf("%s", pw);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s",
             CMD_LOGIN, id, pw);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_meal(int sock)
{
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char date[9];
    char buffer[BUFFER_SIZE];

    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);
    printf("날짜 (YYYYMMDD): ");
    scanf("%s", date);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s",
             CMD_MEAL, edu_office, school_name, date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_multi_meal(int sock)
{
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char start_date[9], end_date[9];
    char buffer[BUFFER_SIZE];

    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);
    printf("시작 날짜 (YYYYMMDD): ");
    scanf("%s", start_date);
    printf("종료 날짜 (YYYYMMDD): ");
    scanf("%s", end_date);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s-%s",
             CMD_MULTI_MEAL, edu_office, school_name, start_date, end_date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_other_meal(int sock)
{
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char date[9];
    char buffer[BUFFER_SIZE];

    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);
    printf("날짜 (YYYYMMDD): ");
    scanf("%s", date);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s",
             CMD_OTHER_MEAL, edu_office, school_name, date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void handle_multi_other_meal(int sock)
{
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];
    char start_date[9], end_date[9];
    char buffer[BUFFER_SIZE];

    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);
    printf("시작 날짜 (YYYYMMDD): ");
    scanf("%s", start_date);
    printf("종료 날짜 (YYYYMMDD): ");
    scanf("%s", end_date);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s-%s",
             CMD_MULTI_OTHER_MEAL, edu_office, school_name, start_date, end_date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답: %s\n", buffer);
}

void show_main_menu(void)
{
    printf("\n=== 급식 정보 시스템 ===\n");
    printf("1. 일반 회원가입\n");
    printf("2. 부모 회원가입\n");
    printf("3. 로그인\n");
    printf("4. 종료\n");
    printf("선택: ");
}

void show_meal_menu(void)
{
    printf("\n=== 급식 조회 메뉴 ===\n");
    printf("1. 내 학교 급식 조회 (단일)\n");
    printf("2. 내 학교 급식 조회 (기간)\n");
    printf("3. 다른 학교 급식 조회 (단일)\n");
    printf("4. 다른 학교 급식 조회 (기간)\n");
    printf("5. 이전 메뉴\n");
    printf("선택: ");
}

int main()
{
    SOCKET sock = connect_to_server();
    if (sock < 0)
    {
        return 1;
    }

    int choice;
    while (1)
    {
        show_main_menu();
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            handle_register_general(sock);
            break;
        case 2:
            handle_register_parent(sock);
            break;
        case 3:
            handle_login(sock);
            while (1)
            {
                show_meal_menu();
                scanf("%d", &choice);

                switch (choice)
                {
                case 1:
                    handle_meal(sock);
                    break;
                case 2:
                    handle_multi_meal(sock);
                    break;
                case 3:
                    handle_other_meal(sock);
                    break;
                case 4:
                    handle_multi_other_meal(sock);
                    break;
                case 5:
                    goto main_menu;
                default:
                    printf("잘못된 선택입니다.\n");
                }
            }
        main_menu:
            break;
        case 4:
            closesocket(sock);
            return 0;
        default:
            printf("잘못된 선택입니다.\n");
        }
    }

    return 0;
}
