#include "lora.hpp"
#include <stdio.h>
#include "Arduino.h"

static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;

static int at_send_check_response(char *p_ack, int timeout_ms, char*p_cmd, ...)
{
    int ch;
    int index = 0;
    int startMillis = 0;
    va_list args;
    memset(recv_buf, 0, sizeof(recv_buf));
    va_start(args, p_cmd);
    Serial.printf(p_cmd, args);
    va_end(args);
    delay(200);
    startMillis = millis();

    if (p_ack == NULL)
    {
        return 0;
    }

    do
    {
        while (Serial.available() > 0)
        {
            ch = Serial.read();
            recv_buf[index++] = ch;
            delay(2);
        }

        if (strstr(recv_buf, p_ack) != NULL)
        {
            return 1;
        }

    } while (millis() - startMillis < timeout_ms);
    return 0;
}

int lora::begin(){
    Serial.begin(9600);

    // Waking up modem
    at_send_check_response(NULL, 100, "AAAA");

    // Connecting to lora network
    if (at_send_check_response("+AT: OK", 100, "AT\r\n"))
    {
        is_exist = true;
        at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
        at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
        at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");
        at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");
        at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"2B7E151628AED2A6ABF7158809CF4F3C\"\r\n");
        at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
        at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
        at_send_check_response("+JOIN: Network joined", 12000, "AT+JOIN\r\n");
        delay(200);
        is_join = true;
    }
    else
    {
        is_exist = false;
    }

    // Sending error if not connected.
    if (is_exist && is_join) {
        return NO_ERROR;
    }

    return -1;
}

int lora::send(int people){
    CayenneLPP lpp(20);
    lpp.reset();
    lpp.addPresence(1, people);
    lpp.addAnalogOutput(2, (float)people);

    String cmd;
    char temp[2] = {0};
    cmd += "AT+CMSGHEX=\"";
    for (size_t i = 0; i < lpp.getSize(); i++)
    {
        sprintf(temp, "%02x", lpp.getBuffer()[i]);
        cmd += temp;
    }
    cmd +=  "\"\n\r";


    char result[120];
    strcpy(result, cmd.c_str());
    
    // Sending messages and going into sleep again.
    at_send_check_response("Done", 5000, result);
    at_send_check_response(NULL, 5000, "AT+LOWPOWER");

    return NO_ERROR;
 }

lora::lora(/* args */)
{
}

lora::~lora()
{
}