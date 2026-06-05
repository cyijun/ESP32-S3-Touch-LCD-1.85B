#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>

void network_init(void);
void network_check(void);
bool network_is_connected(void);
String network_get_ip(void);
int network_get_rssi(void);
bool network_sync_ntp_to_rtc(void);

#endif
