#ifndef LORA_H
#define LORA_H

#include <stdint.h>

#define NO_ERROR 1
#define ERROR_GENERIC 0
#define ERROR_NO_LORA -1
#define ERROR_NO_JOIN -2
#define ERROR_NO_SEND -3

#define ELSYS_TEMP      0x01
#define ELSYS_HUMIDITY  0x02

struct lora_config {
    const char* app_eui;
    const char* app_key;
    const char* dev_eui;
    uint32_t port;
    uint32_t time_out;
};

class lora
{
private:
    lora_config* config;
    int at_send_check_response(const char* p_ack, const char* p_cmd, ...);
public:
    int begin(lora_config* cfg);
    int send(float temperature, int humidity);
};



#endif