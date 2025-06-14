#include "common.h"
#include "protocol.h"
#include "handlers.h"
#include "network.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

// 전역 변수
SOCKET client_socket;
char current_user_id[MAX_ID_LEN] = {0};
char current_user_role[MAX_ROLE_LEN] = {0};
char current_user_edu_office[MAX_EDU_OFFICE_LEN] = {0};
char current_user_school[MAX_SCHOOL_NAME_LEN] = {0};

// 함수 선언
void print_main_menu(void);
void handle_main_menu(int choice);

int main(void)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup 실패\n");
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("서버 연결 실패\n");
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("서버에 연결되었습니다.\n");

    while (1)
    {
        print_main_menu();
        int choice;
        if (scanf("%d", &choice) != 1)
        {
            printf("잘못된 입력입니다.\n");
            while (getchar() != '\n'); // 입력 버퍼 비우기
            continue;
        }
        getchar(); // 버퍼 비우기

        if (choice == 3)
        {
            break;
        }

        handle_main_menu(choice);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

void print_main_menu(void)
{
    printf("\n=== 메인 메뉴 ===\n");
    printf("1. 로그인\n");
    printf("2. 회원가입\n");
    printf("3. 종료\n");
    printf("선택: ");
}

void handle_main_menu(int choice)
{
    char id[MAX_ID_LEN], pw[MAX_PW_LEN];
    char edu_office[MAX_EDU_OFFICE_LEN], school_name[MAX_SCHOOL_NAME_LEN];
    char response[BUFFER_SIZE];

    switch (choice)
    {
    case 1: // 로그인
        printf("아이디: ");
        fgets(id, sizeof(id), stdin);
        id[strcspn(id, "\n")] = 0;

        printf("비밀번호: ");
        fgets(pw, sizeof(pw), stdin);
        pw[strcspn(pw, "\n")] = 0;

        if (handle_login(id, pw, response))
        {
            printf("로그인 성공: %s\n", response);
            while (1) {
                if (strcmp(current_user_role, "parent") == 0) {
                    print_parent_menu();
                    int menu_choice;
                    scanf("%d", &menu_choice);
                    getchar();
                    
                    if (menu_choice == 5) break;
                    handle_parent_menu(menu_choice);
                } else {
                    print_general_menu();
                    int menu_choice;
                    scanf("%d", &menu_choice);
                    getchar();
                    
                    if (menu_choice == 5) break;
                    handle_general_menu(menu_choice);
                }
            }
        }
        else
        {
            printf("로그인 실패: %s\n", response);
        }
        break;

    case 2: // 회원가입
        printf("아이디: ");
        fgets(id, sizeof(id), stdin);
        id[strcspn(id, "\n")] = 0;

        printf("비밀번호: ");
        fgets(pw, sizeof(pw), stdin);
        pw[strcspn(pw, "\n")] = 0;

        printf("교육청 이름: ");
        fgets(edu_office, sizeof(edu_office), stdin);
        edu_office[strcspn(edu_office, "\n")] = 0;

        printf("학교 코드: ");
        fgets(school_name, sizeof(school_name), stdin);
        school_name[strcspn(school_name, "\n")] = 0;

        if (handle_register(id, pw, edu_office, school_name, response))
        {
            printf("회원가입 성공: %s\n", response);
        }
        else
        {
            printf("회원가입 실패: %s\n", response);
        }
        break;

    default:
        printf("잘못된 선택입니다.\n");
    }
}