#ifndef _ETHERNET_DHCP_H_
#define _ETHERNET_DHCP_H_

#include <wm_type_def.h>

class DhcpClass
{
private:
    int request_DHCP_lease();
    void reset_DHCP_lease();
    void presend_DHCP();
    void send_DHCP_MESSAGE(uint8_t, uint16_t);
    void printByte(char *, uint8_t);
    uint8_t parseDHCPResponse(unsigned long responseTimeout, u32 &transactionId);
public:
    u32 getLocalIP();
    u32 getSubnetMask();
    u32 getGatewayIp();
    u32 getDhcpServerIp();
    u32 getDnsServerIp();
    char * getLocalIPStr();
    char * getSubnetMaskStr();
    char * getGatewayIpStr();
    char * getDhcpServerIpStr();
    char * getDnsServerIpStr();
    int beginWithDHCP(uint8_t *, u32 timeout, u32 responseTimeout);
    int checkLease();
};

#endif
