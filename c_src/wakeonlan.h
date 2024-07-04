#ifndef WAKEONLAN_H
#define WAKEONLAN_H

void send_wake_on_lan(const char *mac_address_str);
char* bridge_test();

#endif
