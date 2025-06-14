#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include <winsock2.h>

// 네트워크 초기화 및 종료
bool init_network(void);
void cleanup_network(void);

// 서버 연결
bool connect_to_server(SOCKET sock, const char *ip, int port);

// 데이터 송수신
bool send_data(SOCKET sock, const char *data, int len);
bool receive_data(SOCKET sock, char *buffer, int buffer_size);
bool receive_response(char *response);

#endif // NETWORK_H