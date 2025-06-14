#include "common.h"
#include <ws2tcpip.h> //  inet_pton() 사용을 위해 추가
#include <iphlpapi.h> //  IP Helper API 사용을 위해 추가

bool init_network(void) // Windows 전용 소켓 초기화 함수
{
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true;
#endif
}

void cleanup_network(void) // Windows 전용 소켓 정리 함수
{
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET create_socket(void) // 소켓 생성 함수
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        perror("소켓 생성 실패");
        return INVALID_SOCKET;
    }
    return sock;
}

bool connect_to_server(SOCKET sock, const char *ip, int port) // 서버에 연결하는 함수
{
    struct sockaddr_in server_addr;     // 서버 주소 정보를 담을 구조체 선언
    server_addr.sin_family = AF_INET;   // IPv4 사용 설정
    server_addr.sin_port = htons(port); // 포트 번호를 네트워크 바이트 순서로 변환

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        perror("주소 변환 실패");
        return false;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("서버 연결 실패");
        return false;
    }

    return true;
}

bool send_data(SOCKET sock, const char *data, int len)
{
    if (send(sock, data, len, 0) < 0)
    {
        perror("데이터 전송 실패");
        return false;
    }
    return true;
}

bool receive_data(SOCKET sock, char *buffer, int size)
{
    memset(buffer, 0, size);
    if (recv(sock, buffer, size - 1, 0) < 0)
    {
        perror("데이터 수신 실패");
        return false;
    }
    return true;
}

void close_socket(SOCKET sock)
{
    if (sock != INVALID_SOCKET)
    {
        closesocket(sock);
    }
}

bool parse_response(const char *buffer, int *status, char *message, char *data) // 클라이언트가 서버로부터 받은 응답 메시지(buffer)를 **구조화된 형식으로 파싱(분리)**하는 함수
// 서버 응답 문자열 → 상태 코드 / 메시지 / 데이터로 나눠주는 역할을 함.
{
    if (!buffer || !status || !message)
        return false;
    if (data) // | 구분자를 기준으로 세 항목을 분리
        return sscanf(buffer, "%d|%[^|]|%[^\n]", status, message, data) >= 2;
    else // 데이터 없이 상태코드+메시지만 있을 경우 "400|오류|"
        return sscanf(buffer, "%d|%[^|]|", status, message) >= 2;
}
