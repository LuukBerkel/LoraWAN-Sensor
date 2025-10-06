#include "lora.hpp"
#include <stdio.h>
#include "Arduino.h"

static char cmd_buf[512];
static char ack_buf[512];
static char recv_buf[512];

int lora::at_send_check_response(char* p_ack, char* p_cmd, ...)
{
    int ch, index, start_time;
    va_list args_cmd, args_ack;
    memset(ack_buf, 0, sizeof(ack_buf));
    memset(cmd_buf, 0, sizeof(cmd_buf));
    memset(recv_buf, 0, sizeof(recv_buf));

    va_start(args_cmd, p_cmd);
    vsnprintf(cmd_buf, sizeof(cmd_buf), p_cmd, args_cmd);  
    va_copy(args_ack, args_cmd);
    vsnprintf(ack_buf, sizeof(ack_buf), p_ack, args_ack);  
    va_end(args_cmd);
    va_end(args_ack);
    Serial2.println(cmd_buf);


    start_time = millis();
    if (p_ack == NULL){
        return NO_ERROR;
    }
    do
    {
        while (Serial2.available() > 0)
        {
            ch = Serial2.read();
            recv_buf[index++] = ch;
        }
        if (strstr(recv_buf, ack_buf) != NULL)
        {
            return NO_ERROR;
        }

    } while (millis() - start_time < this->config->time_out);
    Serial.println(recv_buf);
    return ERROR_GENERIC;
}

int lora::begin(lora_config* config){
    // Starting serial to lora module
    Serial2.begin(115200);
    this->config = config;

    // Connecting to lora network
    if (at_send_check_response("+AT: OK", "AT")) {
        at_send_check_response("+MODE: LWOTAA", "AT+MODE=LWOTAA");
        at_send_check_response("+DR: EU868", "AT+DR=EU868");
        at_send_check_response("+CLASS: A", "AT+CLASS=A");
        at_send_check_response("+CH: NUM", "AT+CH=NUM,0-2");
        at_send_check_response("+ID: DevEui", "AT+ID=DevEui,\"%s\"", config->dev_eui);
        at_send_check_response("+ID: AppEui", "AT+ID=AppEui,\"%s\"", config->app_eui);
        at_send_check_response("+KEY: APPKEY", "AT+KEY=APPKEY,\"%s\"", config->app_key);
        at_send_check_response("+PORT: %d", "AT+PORT=%d", config->port);
        int err = at_send_check_response("+JOIN: Network joined", "AT+JOIN");
        if (err != NO_ERROR){
            return ERROR_NO_JOIN;
        }
    } else{
        return ERROR_NO_LORA;
    }

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
