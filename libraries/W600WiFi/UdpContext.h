/**
 * @file    UdpContext.h
 *
 * @brief   UDP Module
 *
 * @author  WinnerMicro
 *
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 */
#ifndef _UDP_CONTEXT_H_
#define _UDP_CONTEXT_H_

class UdpContext;

extern "C" {
#include "wm_type_def.h"
	#include "wm_socket.h"
}

#define WIFI_DBG //printf

#define GET_IP_HDR(pb) reinterpret_cast<ip_hdr*>(((uint8_t*)((pb)->payload)) - UDP_HLEN - IP_HLEN);
#define GET_UDP_HDR(pb) reinterpret_cast<udp_hdr*>(((uint8_t*)((pb)->payload)) - UDP_HLEN);

class UdpContext
{
public:
    typedef std::function<void(void)> rxhandler_t;
    UdpContext()
    : _pcb(0)
    , _rx_buf(0)
    , _first_buf_taken(false)
    , _rx_buf_offset(0)
    , _refcnt(0)
    , _tx_buf_head(0)
    , _tx_buf_cur(0)
    , _tx_buf_offset(0)
    {
        _pcb = udp_new();
#ifdef LWIP_MULTICAST_TX_OPTIONS
        _mcast_ttl = 1;
#endif
    }
    ~UdpContext()
    {
        udp_remove(_pcb);
        _pcb = 0;
        if (_tx_buf_head)
        {
            pbuf_free(_tx_buf_head);
            _tx_buf_head = 0;
            _tx_buf_cur = 0;
            _tx_buf_offset = 0;
        }
        if (_rx_buf)
        {
            pbuf_free(_rx_buf);
            _rx_buf = 0;
            _rx_buf_offset = 0;
        }
    }

    void ref()
    {
        ++_refcnt;
    }

    void unref()
    {
        if(this != NULL) {
            //WIFI_DBG(":ur %d\r\n", _refcnt);
            if(--_refcnt == 0) {
                delete this;
            }
        }
    }

    bool connect(ip_addr_t addr, uint16_t port)
    {
        ip_addr_copy(_pcb->remote_ip, addr);
        _pcb->remote_port = port;
        return true;
    }

    bool listen(ip_addr_t addr, uint16_t port)
    {
        udp_recv(_pcb, &_s_recv, (void *) this);
        err_t err = udp_bind(_pcb, &addr, port);
        return err == ERR_OK;
    }

    void disconnect()
    {
        udp_disconnect(_pcb);
    }

#if LWIP_VERSION_MAJOR == 1
    void setMulticastInterface(ip_addr_t addr)
    {
        udp_set_multicast_netif_addr(_pcb, addr);
    }
#else
    void setMulticastInterface(const ip_addr_t& addr)
    {
        udp_set_multicast_netif_addr(_pcb, &addr);
    }
#endif

    void setMulticastTTL(int ttl)
    {
#ifdef LWIP_MULTICAST_TX_OPTIONS
        _mcast_ttl = ttl;
#else
        udp_set_multicast_ttl(_pcb, ttl);
#endif
    }

    void onRx(rxhandler_t handler) {
        _on_rx = handler;
    }

    size_t getSize() const
    {
        if (!_rx_buf)
            return 0;

        return _rx_buf->len - _rx_buf_offset;
    }

    size_t tell() const
    {
        return _rx_buf_offset;
    }

    void seek(const size_t pos)
    {
        //assert(isValidOffset(pos));
        _rx_buf_offset = pos;
    }

    bool isValidOffset(const size_t pos) const {
        return (pos <= _rx_buf->len);
    }

    uint32_t getRemoteAddress()
    {
        if (!_rx_buf)
            return 0;

        ip_hdr* iphdr = GET_IP_HDR(_rx_buf);
        return iphdr->src.addr;
    }

    uint16_t getRemotePort()
    {
        if (!_rx_buf)
            return 0;

        udp_hdr* udphdr = GET_UDP_HDR(_rx_buf);
        return ntohs(udphdr->src);
    }

    uint32_t getDestAddress()
    {
        if (!_rx_buf)
            return 0;

        ip_hdr* iphdr = GET_IP_HDR(_rx_buf);
        return iphdr->dest.addr;
    }

    uint16_t getLocalPort()
    {
        if (!_pcb)
            return 0;

        return _pcb->local_port;
    }

    bool next()
    {
        if (!_rx_buf)
            return false;

        if (!_first_buf_taken)
        {
            _first_buf_taken = true;
            return true;
        }

        auto head = _rx_buf;
        _rx_buf = _rx_buf->next;
        _rx_buf_offset = 0;

        if (_rx_buf)
        {
            pbuf_ref(_rx_buf);
        }
        pbuf_free(head);
        return _rx_buf != 0;
    }

    int read()
    {
        if (!_rx_buf || _rx_buf_offset >= _rx_buf->len)
            return -1;

        char c = reinterpret_cast<char*>(_rx_buf->payload)[_rx_buf_offset];
        _consume(1);
        return c;
    }

    size_t read(char* dst, size_t size)
    {
        if (!_rx_buf)
            return 0;

        size_t max_size = _rx_buf->len - _rx_buf_offset;
        size = (size < max_size) ? size : max_size;
        //WIFI_DBG(":urd %d, %d, %d\r\n", size, _rx_buf->len, _rx_buf_offset);

        memcpy(dst, reinterpret_cast<char*>(_rx_buf->payload) + _rx_buf_offset, size);
        _consume(size);

        return size;
    }

    int peek()
    {
        if (!_rx_buf || _rx_buf_offset == _rx_buf->len)
            return -1;

        return reinterpret_cast<char*>(_rx_buf->payload)[_rx_buf_offset];
    }

    void flush()
    {
        if (!_rx_buf)
            return;

        _consume(_rx_buf->len - _rx_buf_offset);
    }


    size_t append(const char* data, size_t size)
    {
        if (!_tx_buf_head || _tx_buf_head->tot_len < _tx_buf_offset + size)
        {
            _reserve(_tx_buf_offset + size);
        }
        if (!_tx_buf_head || _tx_buf_head->tot_len < _tx_buf_offset + size)
        {
            //WIFI_DBG("failed _reserve");
            return 0;
        }

        size_t left_to_copy = size;
        while(left_to_copy)
        {
            // size already used in current pbuf
            size_t used_cur = _tx_buf_offset - (_tx_buf_head->tot_len - _tx_buf_cur->tot_len);
            size_t free_cur = _tx_buf_cur->len - used_cur;
            if (free_cur == 0)
            {
                _tx_buf_cur = _tx_buf_cur->next;
                continue;
            }
            size_t will_copy = (left_to_copy < free_cur) ? left_to_copy : free_cur;
            memcpy(reinterpret_cast<char*>(_tx_buf_cur->payload) + used_cur, data, will_copy);
            _tx_buf_offset += will_copy;
            left_to_copy -= will_copy;
            data += will_copy;
        }
        return size;
    }

    bool send(ip_addr_t* addr = 0, uint16_t port = 0)
    {
        size_t data_size = _tx_buf_offset;
        pbuf* tx_copy = pbuf_alloc(PBUF_TRANSPORT, data_size, PBUF_RAM);
        if(!tx_copy){
            //WIFI_DBG("failed pbuf_alloc");
        }
        else{
            uint8_t* dst = reinterpret_cast<uint8_t*>(tx_copy->payload);
            for (pbuf* p = _tx_buf_head; p; p = p->next) {
                size_t will_copy = (data_size < p->len) ? data_size : p->len;
                memcpy(dst, p->payload, will_copy);
                dst += will_copy;
                data_size -= will_copy;
            }
        }
        if (_tx_buf_head)
            pbuf_free(_tx_buf_head);
        _tx_buf_head = 0;
        _tx_buf_cur = 0;
        _tx_buf_offset = 0;
        if(!tx_copy){
            return false;
        }


        if (!addr) {
            addr = &_pcb->remote_ip;
            port = _pcb->remote_port;
        }
#ifdef LWIP_MULTICAST_TX_OPTIONS
        uint16_t old_ttl = _pcb->ttl;
        if (ip_addr_ismulticast(addr)) {
            _pcb->ttl = _mcast_ttl;
        }
#endif
        err_t err = udp_sendto(_pcb, tx_copy, addr, port);
        if (err != ERR_OK) {
            //WIFI_DBG(":ust rc=%d\r\n", (int) err);
        }
#ifdef LWIP_MULTICAST_TX_OPTIONS
        _pcb->ttl = old_ttl;
#endif
        pbuf_free(tx_copy);
        return err == ERR_OK;
    }

private:

    void _reserve(size_t size)
    {
        const size_t pbuf_unit_size = 128;
        if (!_tx_buf_head)
        {
            _tx_buf_head = pbuf_alloc(PBUF_TRANSPORT, pbuf_unit_size, PBUF_RAM);
            if (!_tx_buf_head)
            {
                return;
            }
            _tx_buf_cur = _tx_buf_head;
            _tx_buf_offset = 0;
        }

        size_t cur_size = _tx_buf_head->tot_len;
        if (size < cur_size)
            return;

        size_t grow_size = size - cur_size;

        while(grow_size)
        {
            pbuf* pb = pbuf_alloc(PBUF_TRANSPORT, pbuf_unit_size, PBUF_RAM);
            if (!pb)
            {
                return;
            }
            pbuf_cat(_tx_buf_head, pb);
            if (grow_size < pbuf_unit_size)
                return;
            grow_size -= pbuf_unit_size;
        }
    }

    void _consume(size_t size)
    {
        _rx_buf_offset += size;
        if (_rx_buf_offset > _rx_buf->len) {
            _rx_buf_offset = _rx_buf->len;
        }
    }

    void _recv(udp_pcb *upcb, pbuf *pb,
            const ip_addr_t *addr, u16_t port)
    {
        (void) upcb;
        (void) addr;
        (void) port;
        if (_rx_buf)
        {
            // there is some unread data
            // chain the new pbuf to the existing one
            //WIFI_DBG(":urch %d, %d\r\n", _rx_buf->tot_len, pb->tot_len);
            pbuf_cat(_rx_buf, pb);
        }
        else
        {
            //WIFI_DBG(":urn %d\r\n", pb->tot_len);
            _first_buf_taken = false;
            _rx_buf = pb;
            _rx_buf_offset = 0;
        }
        if (_on_rx) {
            _on_rx();
        }
    }


#if LWIP_VERSION_MAJOR == 1
    static void _s_recv(void *arg,
            udp_pcb *upcb, pbuf *p,
            ip_addr_t *addr, u16_t port)
#else
    static void _s_recv(void *arg,
            udp_pcb *upcb, pbuf *p,
            const ip_addr_t *addr, u16_t port)
#endif
    {
        reinterpret_cast<UdpContext*>(arg)->_recv(upcb, p, addr, port);
    }

private:
    udp_pcb* _pcb;
    pbuf* _rx_buf;
    bool _first_buf_taken;
    size_t _rx_buf_offset;
    int _refcnt;
    pbuf* _tx_buf_head;
    pbuf* _tx_buf_cur;
    size_t _tx_buf_offset;
    rxhandler_t _on_rx;
#ifdef LWIP_MULTICAST_TX_OPTIONS
    uint16_t _mcast_ttl;
#endif
};

#endif
