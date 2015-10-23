/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "FS.h"

#include "eeprom_util.h"

#include "WebFramework.h"
#include "webpage.h"


//void (* resetFunc)(void) = 0;
void setupWiFiAccessPoint();
void setupWiFiClient();

const char WiFiAPPSK[] = "sparkfun";
char ssid[256] = {0};
char password[256] = {0};
MDNSResponder mdns;

enum WifiMode {
  WIFI_MODE_AP = 1,
  WIFI_MODE_CLIENT = 2,
};

WifiMode wifiMode = WIFI_MODE_AP;

void hardResetWifiMode()
{
  Serial.println("Hard reset of EEPROM and SSID");
  eeprom_erase_all(0x00);

  EEPROM.commit();
  wifiMode = WIFI_MODE_AP;
}

void readEEPROMandCheckMode()
{
  

  int ssidAddr = 0;
  int passwordAddr = 256;
  char buffer[256];
  
  bool readSSIDOK = eeprom_read_string(ssidAddr, buffer, 255);
  if (readSSIDOK)
  {
//    Serial.print("Read ssid len:" );
//    Serial.println(strlen(buffer));
//    Serial.println((int)buffer[0]);
    strcpy(ssid,buffer);
  }
  
  bool readPassOK = eeprom_read_string(passwordAddr, buffer, 255);
  if (readPassOK)
  {
    strcpy(password, buffer);
  }

  if (readSSIDOK && strlen(ssid) > 0)
  {
    wifiMode = WIFI_MODE_CLIENT;
  }
  else
  {
    wifiMode = WIFI_MODE_AP;
  }
}

void writeSSIDandPasswordToEEPROMandReset()
{
  int ssidAddr = 0;
  int passwordAddr = 256;
  
  bool ssidWritten = eeprom_write_string(ssidAddr, ssid);
  if (ssidWritten)
  {
    Serial.print("SSID written to EEPROM: ");
    Serial.println(ssid);
  }
  else
  {
    Serial.print("!!! Failed to write SSID to EEPROM.");
  }
  
  bool passwordWritten = eeprom_write_string(passwordAddr, password);

  EEPROM.commit();
  

  delay(1000);
  Serial.print("Resetting...");
//  resetFunc();
  ESP.restart();
}


ESP8266WebServer server ( 80 );
File f;
String uploadError;

int hardResetPin = 12;

void setup ( void ) {
// Read the SSID and PASSWORD from EEPROM
  EEPROM.begin(512);
  SPIFFS.begin();
  
	Serial.begin ( 115200 );

  pinMode(hardResetPin, INPUT_PULLUP);
  if (digitalRead(hardResetPin) == LOW)
  {
    hardResetWifiMode();
  }

  readEEPROMandCheckMode();

  if (wifiMode == WIFI_MODE_AP)
    setupWiFiAccessPoint();
  else if (wifiMode == WIFI_MODE_CLIENT)
    setupWiFiClient();

  // Create a local DNS server
  if (wifiMode == WIFI_MODE_CLIENT)
  {
    if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
      Serial.println ( "MDNS responder started" );
    }
  }

  setupPageHandlers();

  // Upload new HTML files
  // This responds to commands like:
  //    curl -F "file=@css/dropdown.css;filename=/css/dropdown.css" 192.168.4.1/update
  //    curl -F "file=@index.html;filename=/index.html" 192.168.4.1/update
  // To upload web pages. This is using the underlying SPIFF filesystem, but doesn't require
  // a completely new upload
  server.onFileUpload([](){
      
      if(server.uri() != "/update") return;
      HTTPUpload& upload = server.upload();
      if(upload.status == UPLOAD_FILE_START){
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        Serial.printf("Update: %s\n", upload.filename.c_str());

        //  (1) Rename the old file
        if (SPIFFS.exists(upload.filename.c_str()))
        {
          SPIFFS.rename(upload.filename.c_str(),(upload.filename+".BAK").c_str());
        }
        //  (2) Create the new file
        f = SPIFFS.open(upload.filename.c_str(), "w+");
        uploadError = "";
        
      } else if(upload.status == UPLOAD_FILE_WRITE){
        // (1) Append this buffer to the end of the open file
        if (f.write(upload.buf, upload.currentSize) != upload.currentSize){
          uploadError = "Error writing file chunk";
        }
        else
        {
          Serial.printf("Wrote bytes: %d\n", upload.currentSize);
        }
        
      } else if(upload.status == UPLOAD_FILE_END){

      // Close the file
        f.close();
        // (1) Check if the update was successful
        // (2) If Successful, close the file and delete the renamed one
        // (3) If failed, close and delete the new file and move the renamed one back in place
        if (uploadError == "")
        {
          Serial.printf("Upload error: %s\n", uploadError.c_str());
          SPIFFS.remove((upload.filename+".BAK").c_str());
        }
        else
        {
          Serial.printf("Error uploading new file putting old file back in place: %s\n", upload.filename.c_str());
          SPIFFS.remove((upload.filename).c_str());
          SPIFFS.rename((upload.filename+".BAK").c_str(), upload.filename.c_str());
        }
        
        Serial.setDebugOutput(false);
      }
      yield();
    });
    server.on("/update", HTTP_POST, [](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");

      //server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
      // TODO: Send back information based on whether the upload was successful.
      server.send(200, "text/plain", uploadError);
      
      //ESP.restart(); // I don't think we need to restart after every file upload
      
    });

  
  // Handle the AP vs client interactions
  server.on( "/mode", []() {

    if (wifiMode == WIFI_MODE_CLIENT)
    {
      server.send ( 200, "text/plain", "wificlient" );
    }
    else
    {
      server.send ( 200, "text/plain", "accesspoint" );
    }    
  });

  server.on( "/ssid", []() {
    server.send ( 200, "text/plain", ssid );    
  });

  server.on( "/password", []() {
    server.send ( 200, "text/plain", password );    
  });
  

  server.on( "/buttonlink.html", []() {
    strcpy(ssid, "dd-wrt");
    strcpy(password, "ELGALLOGRINGO");

    //sendBinaryFile("text/html", _ssid_success_html, _ssid_success_html_len);
    server.send ( 200, "text/plain", "Writing SSID and password to EEPROM and resetting" );
    
    writeSSIDandPasswordToEEPROMandReset();
    
  });

  server.on( "/set_ssid_password.html", []() {

    if (server.args() > 0)
    {
      Serial.println("args present");

      String ssidParm = server.arg("ssid"); 
      String passwordParm = server.arg("password");

      ssidParm.toCharArray(ssid,255);
      passwordParm.toCharArray(password,255);

      Serial.print("Setting ssid to: ");
      Serial.println(ssid);

      Serial.print("Setting password to: ");
      Serial.println(password);
    }
    
    //sendBinaryFile("text/html", _ssid_success_html, _ssid_success_html_len);
    server.send ( 200, "text/plain", "Writing SSID and password to EEPROM and resetting" );
    
    writeSSIDandPasswordToEEPROMandReset();
    
  });
  
	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
}


void loop ( void ) {
  if (wifiMode == WIFI_MODE_CLIENT)
	  mdns.update();

	server.handleClient();

  delay(10);
}



void setupWiFiAccessPoint()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);

  Serial.print("Access point name: ");
  Serial.println(AP_NameString);
  Serial.println ("Done configuring access point");
  
  delay(2000);
  
}



void setupWiFiClient()
{
  // If no password was given, assume it is an unsecured network
  if (strlen(password) == 0)
    WiFi.begin ( ssid);
  else
    WiFi.begin ( ssid, password );

  Serial.print ( "Connecting to " );
  Serial.println ( ssid );

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );

}



