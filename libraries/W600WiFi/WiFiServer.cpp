
#define LWIP_INTERNAL

extern "C" {
//    #include "osapi.h"
//    #include "ets_sys.h"
}

#include "debug.h"
#include "WiFiClient.h"
#include "WiFiServer.h"
extern "C" {
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/inet.h"
}
#include "ClientContext.h"

WiFiServer::WiFiServer(IPAddress addr, uint16_t port)
: _port(port)
, _addr(addr)
, _pcb(NULL)
{
}

WiFiServer::WiFiServer(uint16_t port)
{
    _port = port;
    _pcb = NULL;
    memset(&_addr, 0, sizeof(_addr));
}

void WiFiServer::begin() {
	begin(_port);
}

void WiFiServer::begin(uint16_t port) {
    WiFiServer::close();
	_port = port;
    err_t err;
    tcp_pcb* pcb = tcp_new();

    if (!pcb)
    {
        printf("[%s %d] <WiFiServer> tcp_new error!!!\n", __func__, __LINE__);
        return;
    }

    ip_addr_t local_addr;
    local_addr.addr = (uint32_t) _addr;
    pcb->so_options |= SOF_REUSEADDR;
    err = tcp_bind(pcb, &local_addr, _port);

    if (err != ERR_OK) {
        printf("[%s %d] <WiFiServer> tcp_bind error!!!\n", __func__, __LINE__);
        tcp_close(pcb);
        return;
    }

    tcp_pcb* listen_pcb = tcp_listen(pcb);
    if (!listen_pcb) {
        printf("[%s %d] <WiFiServer> tcp_listen error!!!\n", __func__, __LINE__);
        tcp_close(pcb);
        return;
    }
    _pcb = listen_pcb;
    tcp_accept(listen_pcb, (tcp_accept_fn)(WiFiServer::_s_accept));
    tcp_arg(listen_pcb, (void*) this);
}

void WiFiServer::setNoDelay(bool nodelay) {
    _noDelay = nodelay;
}

bool WiFiServer::getNoDelay() {
    return _noDelay;
}

bool WiFiServer::hasClient() {
    if (_unclaimed)
        return true;
    return false;
}

 WiFiClient WiFiServer::available(uint8_t* status) {
    (void) status;
    if (_unclaimed) {
        WiFiClient result(_unclaimed);
        _unclaimed = _unclaimed->next();
        result.setNoDelay(_noDelay);
        //DEBUGV("WS:av\r\n");
        return result;
    }

    //optimistic_yield(1000);
    return WiFiClient();
}

uint8_t WiFiServer::status()  {
    if (!_pcb)
        return CLOSED;
    return _pcb->state;
}

void WiFiServer::close() {
    if (!_pcb) {
      return;
    }
    tcp_close(_pcb);
    _pcb = nullptr;
}


void WiFiServer::stop() {
    WiFiServer::close();
}

size_t WiFiServer::write(uint8_t b) {
    return WiFiServer::write(&b, 1);
}

size_t WiFiServer::write(const uint8_t *buffer, size_t size) {
    // write to all clients
    // not implemented
    (void) buffer;
    (void) size;
    return 0;
}

template<typename T>
T* slist_append_tail(T* head, T* item) {
    if (!head)
        return item;
    T* last = head;
    while(last->next())
        last = last->next();
    last->next(item);
    return head;
}

long WiFiServer::_accept(tcp_pcb* apcb, long err) {
    //printf("[%s %s %d]\n", strrchr(__FILE__, '\\') + 1, __func__, __LINE__);
    (void) err;
    //DEBUGV("WS:ac\r\n");
    ClientContext* client = new ClientContext(apcb, &WiFiServer::_s_discard, this);
    _unclaimed = slist_append_tail(_unclaimed, client);
    tcp_accepted(_pcb);
    return ERR_OK;
}

void WiFiServer::_discard(ClientContext* client) {
    (void) client;
    // _discarded = slist_append_tail(_discarded, client);
    //DEBUGV("WS:dis\r\n");
}

err_t WiFiServer::_s_accept(void *arg, tcp_pcb* newpcb, err_t err) {
    return reinterpret_cast<WiFiServer*>(arg)->_accept(newpcb, err);
}

void WiFiServer::_s_discard(void* server, ClientContext* ctx) {
    reinterpret_cast<WiFiServer*>(server)->_discard(ctx);
}

