#include "lora.hpp"
#include <stdio.h>
#include "Arduino.h"

static char cmd_buf[512];
static char recv_buf[1024];

int lora::at_send_check_response(const char* p_ack, const char* p_cmd, ...)
{
    int index = 0;
    int start_time = 0;
    va_list args;
    memset(cmd_buf, 0, sizeof(cmd_buf));
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    vsnprintf(cmd_buf, sizeof(cmd_buf), p_cmd, args);
    va_end(args);
    Serial.println(cmd_buf);
    Serial2.println(cmd_buf);
    delay(100);

    start_time = millis();
    if (p_ack == NULL){
        return NO_ERROR;
    }
    do
    {
        while (Serial2.available() > 0)
        {
            char ch = Serial2.read();
            recv_buf[index++] = ch;
            delay(2);
        }

        recv_buf[index+1] = '\0';
        if (strstr(recv_buf, p_ack) != NULL)
        {
            Serial.print(recv_buf);
            return NO_ERROR;
        }

    } while (millis() - start_time < this->config->time_out_ms);
    Serial.print(recv_buf);
    return ERROR_GENERIC;
}

int lora::begin(lora_config* config){
    Serial2.begin(115200);
    delay(100);
    this->config = config;

    int err = at_send_check_response("OK", "AT");
    if (err != NO_ERROR) {
        return ERROR_NO_LORA;
    }
    
    err = at_send_check_response("AT+NJS=1", "AT+NJS=?");
    if (err == NO_ERROR) { 
        return NO_ERROR;
    }

    at_send_check_response("OK", "AT+CLASS=A");
    at_send_check_response("OK", "AT+DEVEUI=%s", config->dev_eui);
    at_send_check_response("OK", "AT+APPEUI=%s", config->app_eui);
    at_send_check_response("OK", "AT+APPKEY=%s", config->app_key);
    err = at_send_check_response("+EVT:JOINED", "AT+JOIN=1:0:10:0");
    if (err != NO_ERROR){
        return ERROR_NO_JOIN;
    }


    return NO_ERROR;
}

int lora::send(float temperature, float humidity){
    char cmd[120] = {0};
    int16_t temp_raw = (int16_t)(temperature * 10);
    uint8_t temp_msb = (temp_raw >> 8) & 0xFF;
    uint8_t temp_lsb = temp_raw & 0xFF;
    int8_t hum = (uint8_t)humidity;
    sprintf(cmd, "AT+SEND=%d:%02X%02X%02X%02X%02X", this->config->port,
            0x01, temp_msb, temp_lsb, 0x02, hum);

    int err = at_send_check_response("OK", cmd);
    if (err != NO_ERROR) {
        return ERROR_NO_SEND;
    }

    return NO_ERROR;
 }
