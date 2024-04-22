#include "Arduino.h"

char net_name[] = "ZTE";
char passwd[] = "";

void setup() {
  // put your setup code here, to run once:
	tls_netif_add_status_event(con_net_status_changed_event);
	demo_connect_net(net_name, passwd);
}

void loop() {
  // put your main code here, to run repeatedly:
delay(10);
}

static void con_net_status_changed_event(u8 status )
{
    switch(status)
    {
    case NETIF_WIFI_JOIN_SUCCESS:
        printf("NETIF_WIFI_JOIN_SUCCESS\n");
        break;
    case NETIF_WIFI_JOIN_FAILED:
        printf("NETIF_WIFI_JOIN_FAILED\n");
        break;
    case NETIF_WIFI_DISCONNECTED:
        printf("NETIF_WIFI_DISCONNECTED\n");
        break;
    case NETIF_IP_NET_UP:
    {
        struct tls_ethif *tmpethif = tls_netif_get_ethif();
        print_ipaddr(&tmpethif->ip_addr);
    }
    break;
    default:
        //printf("UNKONWN STATE:%d\n", status);
        break;
    }
}

int demo_connect_net(char *ssid, char *pwd)
{
    struct tls_param_ip *ip_param = NULL;
    u8 wireless_protocol = 0;

    if (!ssid)
    {
        return WM_FAILED;
    }

    printf("\nssid:%s \n", ssid);
    printf("password=%s \n", pwd);
    tls_wifi_disconnect();

    tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, TRUE);
    if (TLS_PARAM_IEEE80211_INFRA != wireless_protocol)
    {
        tls_wifi_softap_destroy();
        wireless_protocol = TLS_PARAM_IEEE80211_INFRA;
        tls_param_set(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, FALSE);
    }

    tls_wifi_set_oneshot_flag(0);

    ip_param = (tls_param_ip*)tls_mem_alloc(sizeof(struct tls_param_ip));
    if (ip_param)
    {
        tls_param_get(TLS_PARAM_ID_IP, ip_param, FALSE);
        ip_param->dhcp_enable = TRUE;
        tls_param_set(TLS_PARAM_ID_IP, ip_param, FALSE);
        tls_mem_free(ip_param);
    }

    tls_wifi_connect((u8 *)ssid, strlen(ssid), (u8 *)pwd, strlen(pwd));
    printf("\nplease wait connect net......\n");

    return WM_SUCCESS;
}

