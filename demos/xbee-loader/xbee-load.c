/* xbee-load.c - Xbee Wi-Fi loader program */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

/* for windows builds */
#ifdef __MINGW32__
#include <winsock.h>

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
#endif

#define DEF_IPADDR  "10.0.1.66"
#define DEF_PORT    80

#define CHUNK_SIZE  1024

#define POST_REQUEST      "\
XPOST /ld HTTP/1.1\r\n\
Content-Length: %d\r\n\
\r\n"

SOCKET ConnectSocket(char *hostName, short port);
void DisconnectSocket(SOCKET socket);
int SocketDataAvailableP(SOCKET socket, int timeout);
int SendSocketData(SOCKET socket, char *buf, int len);
int ReceiveSocketData(SOCKET socket, char *buf, int len);

static void usage(void);

/* main - the main function */
int main(int argc, char *argv[])
{
    char buf[CHUNK_SIZE];
    int xbee, fileSize, remaining, cnt, i;
    char *file = NULL;
    char *ipaddr = DEF_IPADDR;
    int port = DEF_PORT;
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
    
    if ((xbee = ConnectSocket(ipaddr, port)) < 0) {
        printf("Failed to connect to %s:%d\n", ipaddr, port);
        return 1;
    }
    printf("Connected\n");
    
    cnt = sprintf(buf, POST_REQUEST, fileSize);
    if (SendSocketData(xbee, buf, cnt) < 0) {
        printf("Failed to send request header to %s:%d\n", ipaddr, port);
        return 1;
    }
    printf("Header sent\n");
    
    for (remaining = fileSize; remaining > 0; remaining -= cnt) {
        if ((cnt = remaining) > CHUNK_SIZE)
            cnt = CHUNK_SIZE;
        printf("Sending %d bytes\n", cnt);
        if (fread(buf, 1, cnt, fp) != cnt) {
            printf("error: reading binary file\n");
            return 1;
        }
        if (SendSocketData(xbee, buf, cnt) < 0) {
            printf("Failed to send request header to %s:%d\n", ipaddr, port);
            return 1;
        }
        //sleep(1);
    }
    printf("Request sent\n");
    fclose(fp);
    
    if ((cnt = ReceiveSocketData(xbee, buf, sizeof(buf) - 1)) < 0) {
        printf("Failed to receive data from %s:%d\n", ipaddr, port);
        return 1;
    }
    buf[cnt] = '\0';
    printf("Data received: %s\n", buf);
    
    DisconnectSocket(xbee);
    printf("Disconnected\n");
    
    return 0;
}

static void usage(void)
{
printf("\
usage: xbee-load\n\
         [ -i <ip-addr> ]  set the IP address of the Xbee Wi-Fi module\n\
         [ -p <port> ]     set the port number (default is 80)\n\
         [ -? ]            display a usage message and exit\n\
         <file>            spin binary file to load\n");
    exit(1);
}

/* ConnectSocket - connect to the server */
SOCKET ConnectSocket(char *hostName, short port)
{
    HOSTENT *hostEntry;
    SOCKADDR_IN addr;
    SOCKET sock;

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
            close(sock);
            return -1;
        }
        addr.sin_addr = *(IN_ADDR *)*hostEntry->h_addr_list;
    }
    
    /* connect to the server */
    if (connect(sock, (SOCKADDR *)&addr, sizeof(addr)) != 0) {
        close(sock);
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

#ifndef __MINGW32__
    /* shutdown the socket */
    shutdown(sock, SHUT_RDWR);
#endif

    /* close the socket */
    close(sock);
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
