/**
 * @file    ClientContext.h
 *
 * @brief   Client Module
 *
 * @author  WinnerMicro
 *
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 */
#ifndef _CLIENT_CONTEXT_H_
#define _CLIENT_CONTEXT_H_

class ClientContext;
class WiFiClient;

typedef void (*discard_cb_t)(void*, ClientContext*);

#ifdef __cplusplus
extern "C"
{
	#include "wm_type_def.h"
	#include "wm_socket.h"
}
#endif

#include "Print.h"
#include "DataSource.h"

#define WIFI_DBG //printf

class ClientContext
{
public:
    ClientContext(tcp_pcb* pcb, discard_cb_t discard_cb, void* discard_cb_arg) :
        _pcb(pcb), _rx_buf(0), _rx_buf_offset(0), _discard_cb(discard_cb), _discard_cb_arg(discard_cb_arg), _refcnt(0), _next(0)
    {
        tcp_setprio(pcb, TCP_PRIO_MIN);
        tcp_arg(pcb, this);
        tcp_recv(pcb, &_s_recv);
        tcp_sent(pcb, &_s_acked);
        tcp_err(pcb, &_s_error);
        tcp_poll(pcb, &_s_poll, 1);

        // not enabled by default for 2.4.0
        //keepAlive();
    }

    err_t abort()
    {
        if(_pcb) {
            //WIFI_DBG(":abort\r\n");
            tcp_arg(_pcb, NULL);
            tcp_sent(_pcb, NULL);
            tcp_recv(_pcb, NULL);
            tcp_err(_pcb, NULL);
            tcp_poll(_pcb, NULL, 0);
            tcp_abort(_pcb);
            _pcb = nullptr;
        }
        return ERR_ABRT;
    }

    err_t close()
    {
        err_t err = ERR_OK;
        if(_pcb) {
            //WIFI_DBG(":close\r\n");
            tcp_arg(_pcb, NULL);
            tcp_sent(_pcb, NULL);
            tcp_recv(_pcb, NULL);
            tcp_err(_pcb, NULL);
            tcp_poll(_pcb, NULL, 0);
            err = tcp_close(_pcb);
            if(err != ERR_OK) {
                //WIFI_DBG(":tc err %d\r\n", (int) err);
                tcp_abort(_pcb);
                err = ERR_ABRT;
            }
            _pcb = nullptr;
        }
        return err;
    }

    ~ClientContext()
    {
    }

    ClientContext* next() const
    {
        return _next;
    }

    ClientContext* next(ClientContext* new_next)
    {
        _next = new_next;
        return _next;
    }

    void ref()
    {
        ++_refcnt;
        //WIFI_DBG(":ref %d\r\n", _refcnt);
    }

    void unref()
    {
        //WIFI_DBG(":ur %d\r\n", _refcnt);
        if(--_refcnt == 0) {
            discard_received();
            close();
            if(_discard_cb) {
                _discard_cb(_discard_cb_arg, this);
            }
            //WIFI_DBG(":del\r\n");
            delete this;
        }
    }

    int connect(ip_addr_t* addr, uint16_t port)
    {
        err_t err = tcp_connect(_pcb, addr, port, &ClientContext::_s_connected);
        if (err != ERR_OK) {
            return 0;
        }
        _connect_pending = 1;
        _op_start_time = millis();

        delay(_timeout_ms);
        _connect_pending = 0;
        if (!_pcb) {
            //WIFI_DBG(":cabrt\r\n");
            return 0;
        }
        if (state() != ESTABLISHED) {
            //WIFI_DBG(":ctmo\r\n");
            abort();
            return 0;
        }
        return 1;
    }

    size_t availableForWrite()
    {
        return _pcb? tcp_sndbuf(_pcb): 0;
    }

    void setNoDelay(bool nodelay)
    {
        if(!_pcb) {
            return;
        }
        if(nodelay) {
            tcp_nagle_disable(_pcb);
        } else {
            tcp_nagle_enable(_pcb);
        }
    }

    bool getNoDelay()
    {
        if(!_pcb) {
            return false;
        }
        return tcp_nagle_disabled(_pcb);
    }

    void setTimeout(int timeout_ms) 
    {
        _timeout_ms = timeout_ms;
    }

    int getTimeout()
    {
        return _timeout_ms;
    }

    uint32_t getRemoteAddress()
    {
        if(!_pcb) {
            return 0;
        }

        return _pcb->remote_ip.addr;
    }

    uint16_t getRemotePort()
    {
        if(!_pcb) {
            return 0;
        }

        return _pcb->remote_port;
    }

    uint32_t getLocalAddress()
    {
        if(!_pcb) {
            return 0;
        }

        return _pcb->local_ip.addr;
    }

    uint16_t getLocalPort()
    {
        if(!_pcb) {
            return 0;
        }

        return _pcb->local_port;
    }

    size_t getSize() const
    {
        if(!_rx_buf) {
            return 0;
        }

        return _rx_buf->tot_len - _rx_buf_offset;
    }

    char read()
    {
        if(!_rx_buf) {
            return 0;
        }

        char c = reinterpret_cast<char*>(_rx_buf->payload)[_rx_buf_offset];
        _consume(1);
        return c;
    }

    size_t read(char* dst, size_t size)
    {
        if(!_rx_buf) {
            return 0;
        }

        size_t max_size = _rx_buf->tot_len - _rx_buf_offset;
        size = (size < max_size) ? size : max_size;

        //WIFI_DBG(":rd %d, %d, %d\r\n", size, _rx_buf->tot_len, _rx_buf_offset);
        size_t size_read = 0;
        while(size) {
            size_t buf_size = _rx_buf->len - _rx_buf_offset;
            size_t copy_size = (size < buf_size) ? size : buf_size;
            //WIFI_DBG(":rdi %d, %d\r\n", buf_size, copy_size);
            memcpy(dst, reinterpret_cast<char*>(_rx_buf->payload) + _rx_buf_offset, copy_size);
            dst += copy_size;
            _consume(copy_size);
            size -= copy_size;
            size_read += copy_size;
        }
        return size_read;
    }

    char peek()
    {
        if(!_rx_buf) {
            return 0;
        }

        return reinterpret_cast<char*>(_rx_buf->payload)[_rx_buf_offset];
    }

    size_t peekBytes(char *dst, size_t size)
    {
        if(!_rx_buf) {
            return 0;
        }

        size_t max_size = _rx_buf->tot_len - _rx_buf_offset;
        size = (size < max_size) ? size : max_size;

        //WIFI_DBG(":pd %d, %d, %d\r\n", size, _rx_buf->tot_len, _rx_buf_offset);
        size_t buf_size = _rx_buf->len - _rx_buf_offset;
        size_t copy_size = (size < buf_size) ? size : buf_size;
        //WIFI_DBG(":rpi %d, %d\r\n", buf_size, copy_size);
        memcpy(dst, reinterpret_cast<char*>(_rx_buf->payload) + _rx_buf_offset, copy_size);
        return copy_size;
    }

    void discard_received()
    {
        if(!_rx_buf) {
            return;
        }
        if(_pcb) {
            tcp_recved(_pcb, (size_t) _rx_buf->tot_len);
        }
        pbuf_free(_rx_buf);
        _rx_buf = 0;
        _rx_buf_offset = 0;
    }

    void wait_until_sent()
    {
        #define WAIT_TRIES_MS 10	// at most 10ms

        int tries = 1+ WAIT_TRIES_MS;

        while (state() == ESTABLISHED && tcp_sndbuf(_pcb) != TCP_SND_BUF && --tries) {
            _write_some();
            delay(4);
        }
    }

    uint8_t state() const
    {
        if(!_pcb) {
            return CLOSED;
        }

        return _pcb->state;
    }


    size_t write(const uint8_t* data, size_t size)
    {
        if (!_pcb) {
            return 0;
        }
        return _write_from_source(new BufferDataSource(data, size));
    }

    size_t write(Stream& stream)
    {
        if (!_pcb) {
            return 0;
        }
        return _write_from_source(new BufferedStreamDataSource<Stream>(stream, stream.available()));
    }

    size_t write_P(PGM_P buf, size_t size)
    {
        if (!_pcb) {
            return 0;
        }
        ProgmemStream stream(buf, size);
        return _write_from_source(new BufferedStreamDataSource<ProgmemStream>(stream, size));
    }

    void keepAlive (uint16_t idle_sec = TCP_DEFAULT_KEEPALIVE_IDLE_SEC, uint16_t intv_sec = TCP_DEFAULT_KEEPALIVE_INTERVAL_SEC, uint8_t count = TCP_DEFAULT_KEEPALIVE_COUNT)
    {
        if (idle_sec && intv_sec && count) {
            _pcb->so_options |= SOF_KEEPALIVE;
            _pcb->keep_idle = (uint32_t)1000 * idle_sec;
            _pcb->keep_intvl = (uint32_t)1000 * intv_sec;
            _pcb->keep_cnt = count;
        }
        else
            _pcb->so_options &= ~SOF_KEEPALIVE;
    }

    bool isKeepAliveEnabled () const
    {
        return !!(_pcb->so_options & SOF_KEEPALIVE);
    }

    uint16_t getKeepAliveIdle () const
    {
        return isKeepAliveEnabled()? (_pcb->keep_idle + 500) / 1000: 0;
    }

    uint16_t getKeepAliveInterval () const
    {
        return isKeepAliveEnabled()? (_pcb->keep_intvl + 500) / 1000: 0;
    }

    uint8_t getKeepAliveCount () const
    {
        return isKeepAliveEnabled()? _pcb->keep_cnt: 0;
    }

protected:

    bool _is_timeout()
    {
        return millis() - _op_start_time > _timeout_ms;
    }

    void _notify_error()
    {
        if (_connect_pending || _send_waiting) {
            delay(4);
        }
    }

    size_t _write_from_source(DataSource* ds)
    {
        assert(_datasource == nullptr);
        assert(_send_waiting == 0);
        _datasource = ds;
        _written = 0;
        _op_start_time = millis();
        do {
            if (_write_some()) {
                _op_start_time = millis();
            }

            if (!_datasource->available() || _is_timeout() || state() == CLOSED) {
                if (_is_timeout()) {
                    //WIFI_DBG(":wtmo\r\n");
                }
                delete _datasource;
                _datasource = nullptr;
                break;
            }

            ++_send_waiting;
            delay(4);
        } while(true);
        _send_waiting = 0;
        return _written;
    }

    bool _write_some()
    {
        if (!_datasource || !_pcb) {
            return false;
        }

        size_t left = _datasource->available();
        size_t can_send = tcp_sndbuf(_pcb);
        if (_pcb->snd_queuelen >= TCP_SND_QUEUELEN) {
            can_send = 0;
        }
        size_t will_send = (can_send < left) ? can_send : left;
        //WIFI_DBG(":wr %d %d %d\r\n", will_send, left, _written);
        bool need_output = false;
        while( will_send && _datasource) {
            size_t next_chunk =
                will_send > _write_chunk_size ? _write_chunk_size : will_send;
            const uint8_t* buf = _datasource->get_buffer(next_chunk);
            if (state() == CLOSED) {
                need_output = false;
                break;
            }
            err_t err = tcp_write(_pcb, buf, next_chunk, TCP_WRITE_FLAG_COPY);
            //WIFI_DBG(":wrc %d %d %d\r\n", next_chunk, will_send, (int) err);
            if (err == ERR_OK) {
                _datasource->release_buffer(buf, next_chunk);
                _written += next_chunk;
                need_output = true;
            } else {
		// ERR_MEM(-1) is a valid error meaning
		// "come back later". It leaves state() opened
                break;
            }
            will_send -= next_chunk;
        }
        if( need_output ) {
            tcp_output(_pcb);
            return true;
        }
        return false;
    }

    void _write_some_from_cb()
    {
        if (_send_waiting == 1) {
            _send_waiting--;
            delay(4);
        }
    }

    err_t _acked(tcp_pcb* pcb, uint16_t len)
    {
        (void) pcb;
        (void) len;
        //WIFI_DBG(":ack %d\r\n", len);
        _write_some_from_cb();
        return ERR_OK;
    }

    void _consume(size_t size)
    {
        ptrdiff_t left = _rx_buf->len - _rx_buf_offset - size;
        if(left > 0) {
            _rx_buf_offset += size;
        } else if(!_rx_buf->next) {
            //WIFI_DBG(":c0 %d, %d\r\n", size, _rx_buf->tot_len);
            if(_pcb) {
                tcp_recved(_pcb, _rx_buf->len);
            }
            pbuf_free(_rx_buf);
            _rx_buf = 0;
            _rx_buf_offset = 0;
        } else {
            //WIFI_DBG(":c %d, %d, %d\r\n", size, _rx_buf->len, _rx_buf->tot_len);
            auto head = _rx_buf;
            _rx_buf = _rx_buf->next;
            _rx_buf_offset = 0;
            pbuf_ref(_rx_buf);
            if(_pcb) {
                tcp_recved(_pcb, head->len);
            }
            pbuf_free(head);
        }
    }

    err_t _recv(tcp_pcb* pcb, pbuf* pb, err_t err)
    {
        (void) pcb;
        (void) err;
        if(pb == 0) { // connection closed
            //WIFI_DBG(":rcl\r\n");
            _notify_error();
            abort();
            return ERR_ABRT;
        }

        if(_rx_buf) {
            //WIFI_DBG(":rch %d, %d\r\n", _rx_buf->tot_len, pb->tot_len);
            pbuf_cat(_rx_buf, pb);
        } else {
            //WIFI_DBG(":rn %d\r\n", pb->tot_len);
            _rx_buf = pb;
            _rx_buf_offset = 0;
        }
        return ERR_OK;
    }

    void _error(err_t err)
    {
        (void) err;
        //WIFI_DBG(":er %d 0x%08x\r\n", (int) err, (uint32_t) _datasource);
        tcp_arg(_pcb, NULL);
        tcp_sent(_pcb, NULL);
        tcp_recv(_pcb, NULL);
        tcp_err(_pcb, NULL);
        _pcb = nullptr;
        _notify_error();
    }

    err_t _connected(struct tcp_pcb *pcb, err_t err)
    {
        (void) err;
        (void) pcb;
        //assert(pcb == _pcb);
        //assert(_connect_pending);
        return ERR_OK;
    }

    err_t _poll(tcp_pcb*)
    {
        _write_some_from_cb();
        return ERR_OK;
    }

    static err_t _s_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *pb, err_t err)
    {
        return reinterpret_cast<ClientContext*>(arg)->_recv(tpcb, pb, err);
    }

    static void _s_error(void *arg, err_t err)
    {
        reinterpret_cast<ClientContext*>(arg)->_error(err);
    }

    static err_t _s_poll(void *arg, struct tcp_pcb *tpcb)
    {
        return reinterpret_cast<ClientContext*>(arg)->_poll(tpcb);
    }

    static err_t _s_acked(void *arg, struct tcp_pcb *tpcb, uint16_t len)
    {
        return reinterpret_cast<ClientContext*>(arg)->_acked(tpcb, len);
    }

    static err_t _s_connected(void* arg, struct tcp_pcb *pcb, err_t err)
    {
        return reinterpret_cast<ClientContext*>(arg)->_connected(pcb, err);
    }

private:
    tcp_pcb* _pcb;

    pbuf* _rx_buf;
    size_t _rx_buf_offset;

    discard_cb_t _discard_cb;
    void* _discard_cb_arg;

    DataSource* _datasource = nullptr;
    size_t _written = 0;
    size_t _write_chunk_size = 256;
    uint32_t _timeout_ms = 5000;
    uint32_t _op_start_time = 0;
    uint8_t _send_waiting = 0;
    uint8_t _connect_pending = 0;

    int8_t _refcnt;
    ClientContext* _next;
};

#endif//_CLIENT_CONTEXT_H_