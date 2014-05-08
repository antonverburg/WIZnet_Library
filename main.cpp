/*
*
*  Test program for W5500 mbed Library

*/
#include "mbed.h"
#include "WIZnetInterface.h"

#define ECHO_SERVER_PORT   5000
const char* ECHO_SERVER_ADDRESS = "192.168.1.229";

#define LPC
//#define ST_NUCLEO

// Setting Seial setting
//Serial pc(P1_13, P1_14); // tx, rx
//Serial pc(PA_2, PA_3); // tx, rx

#define TCP_Server
//#define TCP_Client
//#define UDP_Server
//#define UDP_Client

#ifdef LPC
// LPC1768 & LPC11U24
SPI spi(p5, p6, p7); // mosi, miso, sclk
WIZnetInterface eth(&spi, p8, p9); // spi, cs, reset
Serial pc(USBTX,USBRX);
#endif

#ifdef ST_NUCLEO
// ST Nucleo
SPI spi(PA_7, PA_6, PA_5); // mosi, miso, sclk
WIZnetInterface eth(&spi, PB_6, PA_10); // spi, cs, reset
Serial pc(USBTX,USBRX);
#endif

// Seeedstudio Arch
//SPI spi(P1_22, P1_21, P1_20); // mosi, miso, sclk
//WIZnetInterface eth(&spi, P0_2, P0_0); // spi, cs, reset

// Freescale FRDM KL25Z
//SPI spi(PTD2, PTD3, PTD1); // mosi, miso, sclk
//WIZnetInterface eth(&spi, PTD0, PTA20); // spi, cs, reset
//Serial pc(USBTX,USBRX);

const char * IP_Addr    = "192.168.1.120";
const char * IP_Subnet  = "255.255.255.0";
const char * IP_Gateway = "192.168.1.111";


int main()
{
    pc.printf("Start\n");

//    int ret = eth.init(); //Use DHCP
    int ret = eth.init(IP_Addr, IP_Subnet, IP_Gateway); // static

    if (!ret) {
        pc.printf("Initialized, MAC: %s\n", eth.getMACAddress());
    } else {
        pc.printf("Error eth.init() - ret = %d\n", ret);
        return -1;
    }

    ret = eth.connect();
    if (!ret) {
        pc.printf("IP: %s, MASK: %s, GW: %s\n",
                  eth.getIPAddress(), eth.getNetworkMask(), eth.getGateway());
    } else {
        pc.printf("Error eth.connect() - ret = %d\n", ret);
        return -1;
    }


#ifdef TCP_Server
    TCPSocketServer server;
    server.bind(ECHO_SERVER_PORT);
    server.listen();

    while (true) {
        pc.printf("\nWait for new connection...\n");
        TCPSocketConnection client;
        server.accept(client);
        //client.set_blocking(false, 1500); // Timeout after (1.5)s

        pc.printf("Connection from: %s\n", client.get_address());
        char buffer[256];
        while (true) {
            int n = client.receive(buffer, sizeof(buffer));
            if (n <= 0) break;

            client.send_all(buffer, n);
            if (n <= 0) break;
        }

        client.close();
    }
#endif

#ifdef TCP_Client
    TCPSocketConnection socket;
    while (socket.connect(ECHO_SERVER_ADDRESS, ECHO_SERVER_PORT) < 0) {
        printf("Unable to connect to (%s) on port (%d)\n", ECHO_SERVER_ADDRESS, ECHO_SERVER_PORT);
        wait(1);
    }

    char hello[] = "Hello World\n";
    socket.send_all(hello, sizeof(hello) - 1);

    char buf[256];
    int n = socket.receive(buf, 256);
    buf[n] = '\0';
    printf("%s", buf);

    socket.close();
    eth.disconnect();

    while(true) {}
#endif


#ifdef UDP_Server
    UDPSocket server;
    ret = server.bind(ECHO_SERVER_PORT);
    printf("sock.bind = %d\n", ret);

    Endpoint client;
    char buffer[256];
    while (true) {
        printf("\nWait for packet...\n");
        int n = server.receiveFrom(client, buffer, sizeof(buffer));

        printf("Received packet from: %s\n", client.get_address());
        server.sendTo(client, buffer, n);
    }
#endif

#ifdef UDP_Client
    UDPSocket sock;
    ret = sock.init();
    sock.bind(0);
    printf("sock.bind = %d\n", ret);
    if (ret == -1) printf("Socket creation Fail\n");

    Endpoint echo_server;
    echo_server.set_address(ECHO_SERVER_ADDRESS, ECHO_SERVER_PORT);

    printf("\nSend UDP data\n");

    char out_buffer[] = "Hello World\n";
    ret = sock.sendTo(echo_server, out_buffer, sizeof(out_buffer));
    if (ret < 0) printf("UDP Send Error\n");
    else printf("UDP Send: %d\n", ret);

    char in_buffer[256];
    int n = sock.receiveFrom(echo_server, in_buffer, sizeof(in_buffer));

    in_buffer[n] = '\0';
    printf("%s\n", in_buffer);

    sock.close();

    eth.disconnect();
    while(1) {}
#endif

#ifdef NTP
// NTP
    time_t ctTime;
    ctTime = time(NULL);
    printf("1. Current Time is: %s\r\n", ctime(&ctTime));

    printf("Trying to update time...\r\n");
    if (ntp.setTime("0.pool.ntp.org") == 0) {
        ctTime = time(NULL);
        printf("2. Current Time is: %s\r\n", ctime(&ctTime));

        // resetting GMT+9
        set_time( time(NULL) + 32400 ); // 9x60x60
        //
        printf("Set time successfully\r\n");
        //time_t ctTime;
        ctTime = time(NULL);
        printf("Time is set to (UTC): %s\r\n", ctime(&ctTime));
    } else {
        printf("Error\r\n");
    }

    eth.disconnect();

    while(1) {
    }
#endif
}
