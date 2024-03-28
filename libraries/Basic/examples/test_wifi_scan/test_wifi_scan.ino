#include "Arduino.h"

void setup() {
  tls_wifi_scan_result_cb_register(wifi_scan_handler);
}

void loop() {
  tls_wifi_scan();
  delay(5000);
}


static  char *scan_privacy_string(u8 privacy)
{
    char *sec;

    switch (privacy)
    {
    case WM_WIFI_AUTH_MODE_OPEN:
        sec = (char *)"NONE";
        break;
    case WM_WIFI_AUTH_MODE_WEP_AUTO:
        sec = (char *)"WEP/AUTO";
        break;
    case WM_WIFI_AUTH_MODE_WPA_PSK_TKIP:
        sec = (char *)"WPA_PSK/TKIP";
        break;
    case WM_WIFI_AUTH_MODE_WPA_PSK_CCMP:
        sec = (char *)"WPA_PSK/CCMP";
        break;
    case WM_WIFI_AUTH_MODE_WPA_PSK_AUTO:
        sec = (char *)"WPA_PSK/AUTO";
        break;
    case WM_WIFI_AUTH_MODE_WPA2_PSK_TKIP:
        sec = (char *)"WPA2_PSK/TKIP";
        break;
    case WM_WIFI_AUTH_MODE_WPA2_PSK_CCMP:
        sec = (char *)"WPA2_PSK/CCMP";
        break;
    case WM_WIFI_AUTH_MODE_WPA2_PSK_AUTO:
        sec = (char *)"WPA2_PSK/AUTO";
        break;
    case WM_WIFI_AUTH_MODE_WPA_WPA2_PSK_TKIP:
        sec = (char *)"WPA_PSK/WPA2_PSK/TKIP";
        break;
    case WM_WIFI_AUTH_MODE_WPA_WPA2_PSK_CCMP:
        sec = (char *)"WPA_PSK/WPA2_PSK/CCMP";
        break;
    case WM_WIFI_AUTH_MODE_WPA_WPA2_PSK_AUTO:
        sec = (char *)"WPA_PSK/WPA2_PSK/AUTO";
        break;

    default:
        sec = (char *)"Unknown";
        break;
    }
    return sec;
}

static  char *scan_mode_string(u8 mode)
{
    char *ap_mode;

    switch (mode)
    {
    case 1:
        ap_mode = (char *)"IBSS";
        break;
    case 2:
        ap_mode = (char *)"ESS";
        break;

    default:
        ap_mode = (char *)"ESS";
        break;
    }
    return ap_mode;
}


static void wifi_scan_handler(void)
{
    char *buf = NULL;
    char *buf1 = NULL;
    u32 buflen;
    int j;
    int err;
    u8 ssid[33];
    struct tls_scan_bss_t *wsr;
    struct tls_bss_info_t *bss_info;

    buflen = 2000;
    buf = (char *)tls_mem_alloc(buflen);
    if (!buf)
    {
        goto end;
    }

    buf1 = (char *)tls_mem_alloc(300);
    if(!buf1)
    {
        goto end;
    }
    memset(buf1, 0, 300);

    err = tls_wifi_get_scan_rslt((u8 *)buf, buflen);
    if (err)
    {
        goto end;
    }

    wsr = (struct tls_scan_bss_t *)buf;
    bss_info = (struct tls_bss_info_t *)(buf + 8);

    printf("\n");

    for(unsigned int i = 0; i < wsr->count; i++)
    {
        j = sprintf(buf1, "bssid:%02X%02X%02X%02X%02X%02X, ", bss_info->bssid[0], bss_info->bssid[1],
                    bss_info->bssid[2], bss_info->bssid[3], bss_info->bssid[4], bss_info->bssid[5]);
        j += sprintf(buf1 + j, "ch:%d, ", bss_info->channel);
        j += sprintf(buf1 + j, "rssi:%d, ", (signed char)bss_info->rssi);
        j += sprintf(buf1 + j, "wps:%d, ", bss_info->wps_support);
        j += sprintf(buf1 + j, "max_rate:%dMbps, ", bss_info->max_data_rate);
        j += sprintf(buf1 + j, "%s, ", scan_mode_string(bss_info->mode));
        j += sprintf(buf1 + j, "%s, ", scan_privacy_string(bss_info->privacy));
        memcpy(ssid, bss_info->ssid, bss_info->ssid_len);
        ssid[bss_info->ssid_len] = '\0';
        j += sprintf(buf1 + j, "%s", ssid);

        printf("%s\n", buf1);

        bss_info ++;
    }

end:
    if(buf)
    {
        tls_mem_free(buf);
    }
    if(buf1)
    {
        tls_mem_free(buf1);
    }
}
