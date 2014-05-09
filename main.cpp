/*
*
*  Test program for W5500 mbed Library

*/
#include "mbed.h"
#include "WIZnetInterface.h"

#define ECHO_SERVER_PORT   5000
const char* ECHO_SERVER_ADDRESS = "192.168.1.229";  // Server IP address

/** 
* Setting DHCP or static
*/
//#define USE_DHCP

/** 
* Setting the platform to test
*/
#define LPC
//#define ST_NUCLEO
//#define FRDM_KL25Z
//#define Seeeduino_Arch

#ifdef LPC
// LPC1768 & LPC11U24
SPI spi(p5, p6, p7); // mosi, miso, sclk
WIZnetInterface eth(&spi, p8, p9); // spi, cs, reset
#endif

#ifdef ST_NUCLEO
// ST Nucleo
SPI spi(PA_7, PA_6, PA_5); // mosi, miso, sclk
WIZnetInterface eth(&spi, PB_6, PA_10); // spi, cs, reset
#endif

#ifdef FRDM_KL25Z
// Freescale FRDM KL25Z
SPI spi(PTD2, PTD3, PTD1); // mosi, miso, sclk
WIZnetInterface eth(&spi, PTD0, PTA20); // spi, cs, reset
#endif

#ifdef Seeeduino_Arch
// Seeedstudio Arch
SPI spi(P1_22, P1_21, P1_20); // mosi, miso, sclk
WIZnetInterface eth(&spi, P0_2, P0_0); // spi, cs, reset
    Serial pc(P1_13, P1_14); // tx, rx
#else
    Serial pc(USBTX,USBRX);
#endif

#ifndef USE_DHCP
// for static IP setting
const char * IP_Addr    = "192.168.1.120";
const char * IP_Subnet  = "255.255.255.0";
const char * IP_Gateway = "192.168.1.111";
#endif


int main()
{
    uint8_t mac[6];
    
    mbed_mac_address((char *)mac);     // using the MAC address in LPC11U24 or LPC1178
//    mac[0] = 0x00; mac[1] = 0x08; mac[2] = 0xDC; mac[3] = 0x00; mac[4] = 0x00; mac[5] = 0x00; 
// you can alo use WIZ550io's MAC address by enabling "USE_WIZ550IO_MAC" in wiznet.h
    
    pc.printf("Start\n");
    #ifdef USE_DHCP
      int ret = eth.init(mac); //Use DHCP
    #else
      int ret = eth.init(mac, IP_Addr, IP_Subnet, IP_Gateway); // static
    #endif

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
}
