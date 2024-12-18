#include "W600WiFiGeneric.h"
#include "wiring_time.h"

#ifdef __cplusplus
extern "C"  {
#endif
#include "wm_type_def.h"
#include "wm_osal.h"
#include "wm_socket2.1.3.h"
#include "lwip/dns.h"
#ifdef __cplusplus
}
#endif


WiFiMode_t WiFiMode = WIFI_OFF;
extern "C" void _delay(int us); 
extern "C" void _myDelay(uint32_t ms){_delay(ms*1000);}

static bool _dns_lookup_pending = false;

/**
 * @brief         This function is used to Get the WiFi's work mode.\
 * @return      enum of WiFiMode: WIFI_OFF, WIFI_STA or WIFI_AP
 * 
 * @note 
 */ 
WiFiMode_t WiFiGenericClass::getMode()
{
    return WiFiMode;
}

/**
 * @brief         This function is used to resolve hostname .
 *
 * @param[in] aHostname The hostname which you want to resolve.
 * @param[out] aResult The result of resove the hostname.
 *
 * @return      1 - success, 0 - failure.
 * 
 * @note 
 */ 
int WiFiGenericClass::hostByName(const char* aHostname, IPAddress& aResult)
{
    return hostByName(aHostname, aResult, 10000);
}

#if LWIP_VERSION_MAJOR == 1
void wifi_dns_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg)
#else
void wifi_dns_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
#endif
{
    (void) name;
    if (!_dns_lookup_pending) {
        return;
    }
    if(ipaddr) {
        (*reinterpret_cast<IPAddress*>(callback_arg)) = ipaddr->addr;
    }
   	_myDelay(1); // resume the hostByName function
	//tls_os_time_delay(1000);
}

/**
 * @brief         This function is used to resolve hostname .
 *
 * @param[in] aHostname The hostname which you want to resolve.
 * @param[in] timeout_ms The timeout when resolve.
 * @param[out] aResult The result of resove the hostname.
 *
 * @return      1 - success, 0 - failure.
 * 
 * @note 
 */ 
 

int WiFiGenericClass::hostByName(const char* aHostname, IPAddress& aResult, uint32_t timeout_ms)
{
    ip_addr_t addr;
    aResult = static_cast<uint32_t>(0);

    if(aResult.fromString(aHostname)) {
        // Host name is a IP address use it!
        return 1;
    }

    err_t err = dns_gethostbyname(aHostname, &addr, &wifi_dns_found_callback, &aResult);
    if(err == ERR_OK) {
        aResult = addr.addr;
    } else if(err == ERR_INPROGRESS) {
        _dns_lookup_pending = true;
        _myDelay(timeout_ms);
        _dns_lookup_pending = false;
        // will return here when dns_found_callback fires
        if(aResult != 0) {
            err = ERR_OK;
        }
    }

    return (err == ERR_OK) ? 1 : 0;
}

