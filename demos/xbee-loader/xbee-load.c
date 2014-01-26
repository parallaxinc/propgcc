/* xbee-load.c - Xbee Wi-Fi loader program */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* for windows builds */
#ifdef __MINGW32__
#include <windows.h>
#include <winsock.h>
#define sleep(n)        Sleep((n) * 1000)

/* for linux and mac builds */
#else
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#define SOCKET int
#define IN_ADDR struct in_addr
#define SOCKADDR_IN struct sockaddr_in
#define SOCKADDR struct sockaddr
#define HOSTENT struct hostent
#define INVALID_SOCKET  -1
#define closesocket(x)  close(x)
#endif

#define DEF_IPADDR  "10.0.1.66"
#define DEF_PORT    80

#define CHUNK_SIZE  512

#define POST_REQUEST      "\
XPOST /ld HTTP/1.1\r\n\
Content-Length: %d\r\n\
\r\n"

SOCKET ConnectSocket(char *hostName, short port);
void DisconnectSocket(SOCKET socket);
int SocketDataAvailableP(SOCKET socket, int timeout);
int SendSocketData(SOCKET socket, char *buf, int len);
int ReceiveSocketData(SOCKET socket, char *buf, int len);
void msdelay(int ms);

int port = DEF_PORT;
int delay = 500;

static void usage(void);

/* main - the main function */
int main(int argc, char *argv[])
{
    char buf[CHUNK_SIZE];
    int fileSize, remaining, cnt, i;
    char *ipaddr = DEF_IPADDR;
    char *file = NULL;
    SOCKET xbee;
    FILE *fp;
    
    /* get the arguments */
    for(i = 1; i < argc; ++i) {

        /* handle switches */
        if(argv[i][0] == '-') {
            switch(argv[i][1]) {
            case 'i':   // set the ip address
                if (argv[i][2])
                    ipaddr = &argv[i][2];
                else if (++i < argc)
                    ipaddr = argv[i];
                else
                    usage();
                break;
            case 'p':   // set the port
                if (argv[i][2])
                    port = atoi(&argv[i][2]);
                else if (++i < argc)
                    port = atoi(argv[i]);
                else
                    usage();
                break;
            case 'd':   // set the delay in ms between chunks
                if (argv[i][2])
                    delay = atoi(&argv[i][2]);
                else if (++i < argc)
                    delay = atoi(argv[i]);
                else
                    usage();
                break;
            default:
                usage();
                break;
            }
        }
        
        /* remember the file to load */
        else {
            if (file)
                usage();
            file = argv[i];
        }
    }

    /* make sure a file to load was specified */
    if (!file)
        usage();
    
    /* open the file to load */
    if ((fp = fopen(file, "rb")) == NULL) {
        printf("error: can't open '%s'\n", file);
        return 1;
    }
    
    /* get the size of the binary file */
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    if ((xbee = ConnectSocket(ipaddr, port)) == INVALID_SOCKET) {
        printf("Failed to connect to %s:%d\n", ipaddr, port);
        return 1;
    }
    
    cnt = sprintf(buf, POST_REQUEST, fileSize);
    if (SendSocketData(xbee, buf, cnt) < 0) {
        printf("Failed to send request header to %s:%d\n", ipaddr, port);
        return 1;
    }
    
    for (remaining = fileSize; remaining > 0; remaining -= cnt) {
        msdelay(delay);
        if ((cnt = remaining) > CHUNK_SIZE)
            cnt = CHUNK_SIZE;
        printf("  %d bytes remaining                \r", remaining);
        fflush(stdout);
        if (fread(buf, 1, cnt, fp) != cnt) {
            printf("error: reading binary file\n");
            return 1;
        }
        if (SendSocketData(xbee, buf, cnt) < 0) {
            printf("Failed to send request header to %s:%d\n", ipaddr, port);
            return 1;
        }
    }
    printf("  %d bytes sent                         \n", fileSize);
    fclose(fp);
    
    if ((cnt = ReceiveSocketData(xbee, buf, sizeof(buf) - 1)) < 0) {
        printf("Failed to receive data from %s:%d\n", ipaddr, port);
        return 1;
    }
    buf[cnt] = '\0';
    printf("Data received: %s\n", buf);
    
    DisconnectSocket(xbee);
    
    return 0;
}

static void usage(void)
{
printf("\
usage: xbee-load\n\
         [ -d <delay> ]    set the delay in ms between packets (default is %d)\n\
         [ -i <ip-addr> ]  set the IP address of the Xbee Wi-Fi module\n\
         [ -p <port> ]     set the port number (default is %d)\n\
         [ -? ]            display a usage message and exit\n\
         <file>            spin binary file to load\n",
         delay, port);
    exit(1);
}

#ifdef __MINGW32__
int socketsInitialized = FALSE;
#endif

/* ConnectSocket - connect to the server */
SOCKET ConnectSocket(char *hostName, short port)
{
    HOSTENT *hostEntry;
    SOCKADDR_IN addr;
    SOCKET sock;

#ifdef __MINGW32__
    WSADATA WsaData;

    /* initialize winsock */
    if (!socketsInitialized) {
        if (WSAStartup(0x0101,&WsaData) != 0)
            return INVALID_SOCKET;
        socketsInitialized = TRUE;
    }
#endif

    /* create the socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return -1;

    /* setup the address */
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    /* get the host address by address in dot notation */
    if (isdigit(hostName[0])) {
        unsigned long hostAddr = inet_addr(hostName);
        addr.sin_addr = *(IN_ADDR *)&hostAddr;
    }
    
    /* get the host address by name */
    else {
        if ((hostEntry = gethostbyname(hostName)) == NULL) {
            closesocket(sock);
            return -1;
        }
        addr.sin_addr = *(IN_ADDR *)*hostEntry->h_addr_list;
    }
    
    /* connect to the server */
    if (connect(sock, (SOCKADDR *)&addr, sizeof(addr)) != 0) {
        closesocket(sock);
        return -1;
    }

    /* return the socket */
    return sock;
}

/* DisconnectSocket - close a connection */
void DisconnectSocket(SOCKET sock)
{
    struct timeval tv;
    fd_set sockets;
    char buf[512];

    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    /* wait for the close to complete */
    for (;;) {
        FD_ZERO(&sockets);
        FD_SET(sock, &sockets);
        if (select(sock + 1, &sockets, NULL, NULL, &tv) > 0) {
            if (FD_ISSET(sock, &sockets))
                if (recv(sock, buf, sizeof(buf), 0) == 0)
                    break;;
        }
        else
            break;
    }

    /* close the socket */
    closesocket(sock);
}

/* SocketDataAvailableP - check for data being available on a socket */
int SocketDataAvailableP(SOCKET sock, int timeout)
{
    struct timeval timeVal;
    fd_set sockets;
    int cnt;

    /* setup the read socket set */
    FD_ZERO(&sockets);
    FD_SET(sock, &sockets);

    timeVal.tv_sec = timeout / 1000;
    timeVal.tv_usec = (timeout % 1000) * 1000;

    /* check for data available */
    cnt = select(sock + 1, &sockets, NULL, NULL, timeout < 0 ? NULL : &timeVal);

    /* return whether data is available */
    return cnt > 0 && FD_ISSET(sock, &sockets);
}

/* SendSocketData - send socket data */
int SendSocketData(SOCKET sock, char *buf, int len)
{
    return send(sock, buf, len, 0);
}

/* ReceiveSocketData - receive socket data */
int ReceiveSocketData(SOCKET sock, char *buf, int len)
{
    return recv(sock, buf, len, 0);
}

void msdelay(int ms)
{
    usleep(ms * 1000);
}
