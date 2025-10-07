#include "lora.hpp"
#include <stdio.h>
#include "Arduino.h"

static char cmd_buf[512];
static char recv_buf[512];

int lora::at_send_check_response(const char* p_ack, const char* p_cmd, ...)
{
    int ch, index, start_time;
    va_list args;

    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    Serial2.printf(p_cmd, args);
    Serial2.print("\r\n");
    va_end(args);
    delay(300);

    start_time = millis();
    if (p_ack == NULL){
        return NO_ERROR;
    }
    do
    {
        while (Serial2.available() > 0)
        {
            char ch = Serial2.read();
            Serial.print(ch);
            recv_buf[index++] = ch;
            delay(2);
        }
        if (strstr(recv_buf, p_ack) != NULL)
        {
            return NO_ERROR;
        }

    } while (millis() - start_time < this->config->time_out);
    return ERROR_GENERIC;
}

int lora::begin(lora_config* config){
    // Starting serial to lora module
    Serial2.begin(115200);
    delay(100);
    this->config = config;

    at_send_check_response("OK", "AT");
    at_send_check_response("OK", "AT");
    // // Connecting to lora network
    // if (at_send_check_response("OK", "AT")) {
    //     at_send_check_response("+MODE: LWOTAA", "AT+MODE=LWOTAA");
    //     at_send_check_response("+DR: EU868", "AT+DR=EU868");
    //     at_send_check_response("+CLASS: A", "AT+CLASS=A");
    //     at_send_check_response("+CH: NUM", "AT+CH=NUM,0-2");
    //     at_send_check_response("+ID: DevEui", "AT+ID=DevEui,\"%s\"", config->dev_eui);
    //     at_send_check_response("+ID: AppEui", "AT+ID=AppEui,\"%s\"", config->app_eui);
    //     at_send_check_response("+KEY: APPKEY", "AT+KEY=APPKEY,\"%s\"", config->app_key);
    //     at_send_check_response("+PORT: %d", "AT+PORT=%d", config->port);
    //     int err = at_send_check_response("+JOIN: Network joined", "AT+JOIN");
    //     if (err != NO_ERROR){
    //         return ERROR_NO_JOIN;
    //     }
    // } else{
    //     return ERROR_NO_LORA;
    // }

    return NO_ERROR;
}

int lora::send(float temperature, int humidity){
    char cmd[120] = {0};
    int16_t temp_raw = (int16_t)(temperature * 10);
    uint8_t temp_msb = (temp_raw >> 8) & 0xFF;
    uint8_t temp_lsb = temp_raw & 0xFF;
    int8_t hum = (uint8_t)humidity;
    sprintf(cmd, "AT+CMSGHEX=\"%02X%02X%02X%02X%02X\"",
            0x01, temp_msb, temp_lsb, 0x02, hum);

    int err = at_send_check_response("OK", cmd);
    if (err != NO_ERROR) {
        return ERROR_NO_SEND;
    }

    return NO_ERROR;
 }
