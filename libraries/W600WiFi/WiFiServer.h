/**
 * @file WiFiServer.h
 *
 * @brief   WiFiServer Module
 *
 * @author Huang Leilei
 *
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 */
#ifndef _WIFI_SERVER_H_
#define _WIFI_SERVER_H_

extern "C" {
#include "lwip/err.h"
#include "lwip/tcp.h"
}

#include "Server.h"
#include "IPAddress.h"

class ClientContext;
class WiFiClient;

class WiFiServer : public Server {
  // Secure server needs access to all the private entries here
protected:
  uint16_t _port;
  IPAddress _addr;
  tcp_pcb* _pcb;

  ClientContext* _unclaimed;
  ClientContext* _discarded;
  bool _noDelay = false;

public:

  WiFiServer(IPAddress addr, uint16_t port);
  
  WiFiServer(uint16_t port);
  
  virtual ~WiFiServer() {}
  
  WiFiClient available(uint8_t* status = NULL);
  
  bool hasClient();
  
  void begin();
  
  void begin(uint16_t port);
  
  void setNoDelay(bool nodelay);
  
  bool getNoDelay();
  
  virtual size_t write(uint8_t);
  
  virtual size_t write(const uint8_t *buf, size_t size);
  
  uint8_t status();
  
  void close();
  
  void stop();

  //using Print::write;
  long _accept(tcp_pcb* newpcb, long err);
  void   _discard(ClientContext* client);

  static err_t _s_accept(void *arg, tcp_pcb* newpcb, err_t err);
  static void _s_discard(void* server, ClientContext* ctx);


};


#endif
