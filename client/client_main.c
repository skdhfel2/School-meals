#include "common.h"
#include "protocol.h"
#include "input_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// 전역 변수 선언
char current_user_id[MAX_ID_LEN];
char current_user_edu_office[MAX_EDU_OFFICE_LEN];
char current_user_school[MAX_SCHOOL_NAME_LEN];
char current_user_role[MAX_ROLE_LEN];

bool connect_to_server(SOCKET sock, const char *ip, int port)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return false;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return false;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        closesocket(sock);
        WSACleanup();
        return false;
    }

    return true;
}

void send_command(SOCKET sock, const char *cmd)
{
    send(sock, cmd, strlen(cmd), 0);
}

void receive_response(SOCKET sock, char *buffer)
{
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0)
    {
        strcpy(buffer, "0//서버 응답 수신 실패//");
    }
    else
    {
        buffer[bytes_received] = '\0';
    }
}

void handle_register_general(SOCKET sock)
{
    char id[MAX_ID_LEN];
    char pw[MAX_PW_LEN];
    char name[MAX_NAME_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN];
    char school_name[MAX_SCHOOL_NAME_LEN];

    char buffer[BUFFER_SIZE];

    printf("아이디: ");
    scanf("%s", id);
    printf("비밀번호: ");
    scanf("%s", pw);
    printf("이름: ");
    scanf("%s", name);
    printf("교육청: ");
    scanf("%s", edu_office);
    printf("학교명: ");
    scanf("%s", school_name);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s//%s//%s",
             CMD_REGISTER_GENERAL, id, pw, name, edu_office, school_name);

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
    clear_input_buffer();
    printf("비밀번호: ");
    scanf("%s", pw);
    clear_input_buffer();

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s",
             CMD_LOGIN, id, pw);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    printf("서버 응답 원본: %s\n", buffer); // 디버깅용

    // 응답 파싱
    char *status = strtok(buffer, "//");    // "1"
    char *command = strtok(NULL, "//");     // "LOGIN"
    char *msg = strtok(NULL, "//");         // "로그인 성공"
    char *edu_office = strtok(NULL, "//");  // "충청남도교육청"
    char *school_name = strtok(NULL, "//"); // "천안중앙고등학교"
    char *role = strtok(NULL, "//");        // "GENERAL"

    printf("파싱 결과:\n"); // 디버깅용
    printf("status: %s\n", status);
    printf("msg: %s\n", msg);
    printf("edu_office: %s\n", edu_office);
    printf("school_name: %s\n", school_name);
    printf("role: %s\n", role);

    if (status && atoi(status) == 1)
    {
        // 로그인 성공 시 사용자 정보 저장
        strncpy(current_user_id, id, MAX_ID_LEN - 1);
        current_user_id[MAX_ID_LEN - 1] = '\0';

        if (edu_office)
        {
            strncpy(current_user_edu_office, edu_office, MAX_EDU_OFFICE_LEN - 1);
            current_user_edu_office[MAX_EDU_OFFICE_LEN - 1] = '\0';
        }

        if (school_name)
        {
            strncpy(current_user_school, school_name, MAX_SCHOOL_NAME_LEN - 1);
            current_user_school[MAX_SCHOOL_NAME_LEN - 1] = '\0';
        }

        if (role)
        {
            strncpy(current_user_role, role, MAX_ROLE_LEN - 1);
            current_user_role[MAX_ROLE_LEN - 1] = '\0';
        }

        printf("저장된 정보:\n"); // 디버깅용
        printf("current_user_id: %s\n", current_user_id);
        printf("current_user_edu_office: %s\n", current_user_edu_office);
        printf("current_user_school: %s\n", current_user_school);

        printf("로그인 성공: %s\n", msg);
    }
    else
    {
        printf("로그인 실패: %s\n", msg ? msg : "알 수 없는 오류");
    }
}

void handle_meal(int sock)
{
    char date[9];
    char buffer[BUFFER_SIZE];

    printf("날짜 (YYYYMMDD): ");
    scanf("%s", date);

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s",
             CMD_GET_MEAL, current_user_edu_office, current_user_school, date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    // 응답 파싱
    char *status = strtok(buffer, "//");
    char *msg = strtok(NULL, "//");
    char *meal = strtok(NULL, ""); // 나머지를 한꺼번에 받음

    if (status && atoi(status) == 1 && meal)
    {
        printf("\n🍱 급식 메뉴:\n%s\n", meal); // 줄바꿈 포함 그대로 출력
    }
    else
    {
        printf("급식 정보를 가져오는데 실패했습니다: %s\n", msg ? msg : "알 수 없음");
    }
}

void handle_multi_meal(int sock)
{
    char start_date[10], end_date[10]; // 8자리 날짜 + null 문자
    char buffer[BUFFER_SIZE];

    if (!get_period_input(start_date, end_date, sizeof(start_date)))
    {
        return;
    }

    char cmd[BUFFER_SIZE];
    snprintf(cmd, sizeof(cmd), "%s//%s//%s//%s-%s",
             CMD_GET_MULTI_MEAL, current_user_edu_office, current_user_school, start_date, end_date);

    send_command(sock, cmd);
    receive_response(sock, buffer);

    // 응답 파싱
    char *status = strtok(buffer, "//");
    char *msg = strtok(NULL, "//");
    char *meals = strtok(NULL, ""); // 나머지를 한꺼번에 받음

    if (status && atoi(status) == 1 && meals)
    {
        printf("\n🍱 기간별 급식 메뉴:\n%s\n", meals); // 줄바꿈 포함 그대로 출력
    }
    else
    {
        printf("급식 정보를 가져오는데 실패했습니다: %s\n", msg ? msg : "알 수 없음");
    }
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

    if (!get_period_input(start_date, end_date, sizeof(start_date)))
    {
        return;
    }

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
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        return 1;
    }

    if (!connect_to_server(sock, SERVER_IP, SERVER_PORT))
    {
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    int choice;
    while (1)
    {
        show_main_menu();
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;

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
                while (getchar() != '\n')
                    ;

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
