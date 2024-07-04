#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H
void on_connect(struct mosquitto *mosq, void *obj, int rc);

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

#endif
