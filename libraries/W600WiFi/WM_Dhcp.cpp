#include "WM_Dhcp.h"

int DhcpClass::request_DHCP_lease()
{
	return 0;
}

void DhcpClass::reset_DHCP_lease()
{
}

void DhcpClass::presend_DHCP()
{
}

void DhcpClass::send_DHCP_MESSAGE(uint8_t, uint16_t)
{
}

void DhcpClass::printByte(char *, uint8_t)
{
}

uint8_t DhcpClass::parseDHCPResponse(unsigned long responseTimeout, u32 &transactionId)
{
	return 0;
}

u32 DhcpClass::getLocalIP()
{
	return 0;
}

u32 DhcpClass::getSubnetMask()
{
	return 0;
}

u32 DhcpClass::getGatewayIp()
{
	return 0;
}

u32 DhcpClass::getDhcpServerIp()
{
	return 0;
}

u32 DhcpClass::getDnsServerIp()
{
	return 0;
}

char * DhcpClass::getLocalIPStr()
{
	return 0;
}

char * DhcpClass::getSubnetMaskStr()
{
	return 0;
}

char * DhcpClass::getGatewayIpStr()
{
	return 0;
}

char * DhcpClass::getDhcpServerIpStr()
{
	return 0;
}

char * DhcpClass::getDnsServerIpStr()
{
	return 0;
}

int DhcpClass::beginWithDHCP(uint8_t *, u32 timeout=60000, u32 responseTimeout = 4000)
{
	return 0;
}

int DhcpClass::checkLease()
{
	return 0;
}
