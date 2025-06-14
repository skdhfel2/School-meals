#include "network.h"
#include "protocol.h"
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
    int bytes_sent;

    // 최대 5초 동안 전송 시도
    fd_set write_fds;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    while (total_sent < len)
    {
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        int select_result = select(0, NULL, &write_fds, NULL, &tv);
        if (select_result <= 0)
        {
            return false;
        }

        bytes_sent = send(sock, data + total_sent, len - total_sent, 0);
        if (bytes_sent == SOCKET_ERROR)
        {
            return false;
        }
        total_sent += bytes_sent;
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
    char buffer[BUFFER_SIZE] = {0};
    int total_received = 0;
    int bytes_received;

    // 최대 5초 동안 응답 대기
    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(client_socket, &read_fds);

    int select_result = select(0, &read_fds, NULL, NULL, &tv);
    if (select_result <= 0)
    {
        strcpy(response, "서버 응답 시간 초과");
        return false;
    }

    while (total_received < BUFFER_SIZE - 1)
    {
        bytes_received = recv(client_socket, buffer + total_received,
                              BUFFER_SIZE - total_received - 1, 0);
        if (bytes_received == SOCKET_ERROR)
        {
            strcpy(response, "서버 통신 오류");
            return false;
        }
        if (bytes_received == 0)
        {
            break;
        }
        total_received += bytes_received;
        buffer[total_received] = '\0';

        // 응답이 완전히 수신되었는지 확인
        if (strstr(buffer, CMD_DELIMITER) != NULL)
        {
            break;
        }
    }

    if (total_received == 0)
    {
        strcpy(response, "서버로부터 응답 없음");
        return false;
    }

    strcpy(response, buffer);
    return true;
}