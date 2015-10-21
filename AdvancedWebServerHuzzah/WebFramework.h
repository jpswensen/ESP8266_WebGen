#ifndef __WEB_FRAMEWORK_H__
#define __WEB_FRAMEWORK_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define BINARY_BUFFER_SIZE 512

void handleNotFound();

void sendBinaryFile(String mimeType, const char *realword, int realLen);

#endif // __WEB_FRAMEWORK_H__
