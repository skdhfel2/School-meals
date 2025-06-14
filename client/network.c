#include "network.h"
#include <stdio.h>
#include <string.h>

// 전역 변수
extern SOCKET client_socket;

// 네트워크 초기화 및 종료
bool init_network(void)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup 실패\n");
        return false;
    }
    return true;
}

void cleanup_network(void)
{
    WSACleanup();
}

// 서버 연결
bool connect_to_server(SOCKET sock, const char *ip, int port)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("서버 연결 실패\n");
        return false;
    }
    return true;
}

// 데이터 송수신
bool send_data(SOCKET sock, const char *data, int len)
{
    int total_sent = 0;
    while (total_sent < len)
    {
        int sent = send(sock, data + total_sent, len - total_sent, 0);
        if (sent == SOCKET_ERROR)
        {
            return false;
        }
        total_sent += sent;
    }
    return true;
}

bool receive_data(SOCKET sock, char *buffer, int buffer_size)
{
    int total_received = 0;
    int bytes_received;

    while (total_received < buffer_size - 1)
    {
        bytes_received = recv(sock, buffer + total_received, buffer_size - total_received - 1, 0);
        if (bytes_received == SOCKET_ERROR)
        {
            return false;
        }
        if (bytes_received == 0)
        {
            break;
        }
        total_received += bytes_received;
        if (buffer[total_received - 1] == '\0')
        {
            break;
        }
    }
    buffer[total_received] = '\0';
    return true;
}

// 응답 처리
bool receive_response(char *response)
{
    char buffer[BUFFER_SIZE];
    int total_received = 0;

    while (total_received < BUFFER_SIZE - 1)
    {
        int received = recv(client_socket, buffer + total_received,
                            BUFFER_SIZE - total_received - 1, 0);
        if (received == SOCKET_ERROR)
        {
            return false;
        }
        if (received == 0)
        {
            break;
        }
        total_received += received;
    }

    buffer[total_received] = '\0';
    strcpy(response, buffer);
    return true;
}