#ifndef __ESP8266WIFI__H
#define __ESP8266WIFI__H

#include "usart.h"
#include "System.h"
#include "stdbool.h"

/*#ifndef BOOL
#define BOOL
typedef enum
{
  FALSE = 0, TRUE  = !FALSE
}
bool;
#endif*/

#define SOCKET_ERROR 1
#define INVALID_SOCKET 1
#define TCP_RX_BUFFER_LEN 1024

///////////////////////////////
// Command Response Timeouts //
///////////////////////////////
#define COMMAND_RESPONSE_TIMEOUT 5000
#define COMMAND_PING_TIMEOUT 800
#define WIFI_CONNECT_TIMEOUT 20000
#define COMMAND_RESET_TIMEOUT 800
#define CLIENT_CONNECT_TIMEOUT 20000
#define TCP_TIMEOUT 2000

#define ESP8266_MAX_SOCK_NUM 5
#define ESP8266_SOCK_NOT_AVAIL 255

typedef enum {
	ESP8266_CMD_BAD = -5,
	ESP8266_RSP_MEMORY_ERR = -4,
	ESP8266_RSP_FAIL = -3,
	ESP8266_RSP_UNKNOWN = -2,
	ESP8266_RSP_TIMEOUT = -1,
	ESP8266_RSP_SUCCESS = 0
}esp8266_cmd_rsp;

typedef enum {
	ESP8266_MODE_STA = 1,
	ESP8266_MODE_AP = 2,
	ESP8266_MODE_STAAP = 3
}esp8266_wifi_mode;

typedef enum {
	ESP8266_CMD_QUERY,
	ESP8266_CMD_SETUP,
	ESP8266_CMD_EXECUTE
}esp8266_command_type;

typedef enum  {
	ESP8266_ECN_OPEN,
	ESP8266_ECN_WPA_PSK,
	ESP8266_ECN_WPA2_PSK,
	ESP8266_ECN_WPA_WPA2_PSK
}esp8266_encryption;

typedef enum  {
	ESP8266_STATUS_GOTIP = 2,
	ESP8266_STATUS_CONNECTED = 3,
	ESP8266_STATUS_DISCONNECTED = 4,
	ESP8266_STATUS_NOWIFI = 5	
}esp8266_connect_status;

typedef enum  {
	AVAILABLE = 0,
	TAKEN = 1,
}esp8266_socket_state;

typedef enum  {
	ESP8266_TCP,
	ESP8266_UDP,
	ESP8266_TYPE_UNDEFINED
}esp8266_connection_type;

typedef enum  {
	ESP8266_CLIENT,
	ESP8266_SERVER
}esp8266_tetype;

//struct esp8266_ipstatus
//{
//	uint8_t linkID;
//	esp8266_connection_type type;
//	IPAddress remoteIP;
//	uint16_t port;
//	esp8266_tetype tetype;
//};

//struct esp8266_status
//{
//	esp8266_connect_status stat;
//	esp8266_ipstatus ipstatus[ESP8266_MAX_SOCK_NUM];
//};

////////////////////
// Initialization //
////////////////////

bool esp8266Begin(void);

///////////////////////
// Basic AT Commands //
///////////////////////

bool esp8266Test(void);
void resetSTM(void);
void resetCube(void);

////////////////////
// WiFi Functions //
////////////////////

int16_t esp8266GetMode(void);
bool esp8266SetMode(esp8266_wifi_mode mode);
bool esp8266Connect(const char * ssid, const char * pwd);

/////////////////////
// TCP/IP Commands //
/////////////////////

bool esp8266UDPConnect(uint8_t client);
bool esp8266NTPConnect(uint8_t client);
bool esp8266TcpConnect(uint8_t * destination, uint8_t * port);
bool esp8266TcpSend(uint8_t con, uint16_t size);
bool esp8266UDPSend(uint8_t con, uint16_t size);
void esp8266TcpSendBuffer(char *buf, uint16_t size);
bool esp8266CheckSend(void);
void sendOKHTTPHeader(void);
bool esp8266TcpClose(uint8_t con);
bool esp8266SetMux(uint8_t mux);
int tcp_getdata(unsigned char* buf, int count);

//////////////////////////////////////////////////
// Private, Low-Level, Ugly, Hardware Functions //
//////////////////////////////////////////////////

void esp8266SendCommand(const char * cmd, esp8266_command_type type, const char * params);
bool esp8266ReadForResponse(const char * rsp, unsigned int timeout);
bool esp8266ReadForResponses(const char * pass, const char * fail, unsigned int timeout, uint16_t bS, uint8_t changeStart);

//////////////////
// Buffer Stuff //
//////////////////

bool esp8266ReadTcpData(void);
void esp8266ClearBuffer(void);
bool esp8266RxBufferAvailable(uint16_t bS);
bool esp8266BufferAvailable(uint8_t con, uint16_t size);
bool esp8266SearchBuffer(uint16_t bS, const char * toFind, uint8_t changeStart);
void printWifiBuffer(uint16_t bS);
void sendAJAXHeader(void);
void sendAJAXFooter(void);

extern char esp8266RxBuffer[];
extern char tcpRxBuffer[];
bool esp8266Read(unsigned int timeout);
extern char myIpAddress[];
extern volatile uint8_t sendUDP;
extern volatile uint8_t lastKey;
 
bool esp8266GetIPAddress(void);
uint16_t esp8266SearchData(const char * find,const char * resp, uint8_t strip);
void initWIFI(void);
bool getNTPTime(void);
uint8_t updateDateTime(void);
bool esp8266StartServer(void);
void esp8266CheckServer(void);
volatile extern uint8_t tcpBufferClient;
uint16_t esp8266CharsInBuffer(uint16_t bS);
volatile extern uint8_t inSend;
void replaceHTMLVariables(uint16_t size);
void ESP8266ChangeSpeed(void);
uint8_t simulateKey(char* key, char* value);
void wifiProcessBuffer(void);
void WIFI_IRQ(char c);
void WIFI_REC_DMA(uint16_t ndtr);
void esp8266TcpSendBufferDMA(char *buf, uint16_t size);
void setupSharedVariables(void);
extern char c[100];
extern USART_TypeDef* ESP8266_USART;

#endif
