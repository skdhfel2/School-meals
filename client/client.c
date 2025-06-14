#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "common.h"
#include "protocol.h"
#include "network.h"
#include "menu.h"
#include "handlers.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

// 전역 변수
SOCKET client_socket;
char current_user_id[MAX_ID_LEN] = {0};
char current_user_role[MAX_ROLE_LEN] = {0};
char current_user_edu_office[MAX_EDU_OFFICE_LEN] = {0};
char current_user_school[MAX_SCHOOL_NAME_LEN] = {0};

int main()
{
    if (!init_network())
    {
        return 1;
    }

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        printf("소켓 생성 실패\n");
        cleanup_network();
        return 1;
    }

    if (!connect_to_server(client_socket, SERVER_IP, SERVER_PORT))
    {
        printf("서버 연결에 실패했습니다.\n");
        closesocket(client_socket);
        cleanup_network();
        return 1;
    }

    int choice;

    while (1)
    {
        if (strlen(current_user_id) == 0)
        {
            // 로그인 메뉴
            print_login_menu();
            scanf("%d", &choice);
            getchar();

            if (choice == 3)
            {
                break;
            }

            handle_login_menu_input(choice);
        }
        else
        {
            // 로그인 후 메뉴
            if (strcmp(current_user_role, ROLE_PARENT) == 0)
            {
                print_parent_menu();
            }
            else
            {
                print_general_menu();
            }
            scanf("%d", &choice);
            getchar();

            if (choice == 5)
            {
                break;
            }

            if (strcmp(current_user_role, ROLE_PARENT) == 0)
            {
                handle_parent_menu_input(choice);
            }
            else
            {
                handle_general_menu_input(choice);
            }
        }
    }

    closesocket(client_socket);
    cleanup_network();
    return 0;
}