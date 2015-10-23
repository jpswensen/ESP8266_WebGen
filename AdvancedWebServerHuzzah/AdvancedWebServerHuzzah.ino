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

#include "WebFramework.h"

#include "webpage.h"

// This can select whether your device should act as a Wifi client or as an access point (for debugging)
//#define MODE_WIFI_AP
#define MODE_WIFI_CLIENT


#if defined  MODE_WIFI_AP
  // Parameters for running as wifi access point
  const char WiFiAPPSK[] = "sparkfun";

  void setupWiFiAccessPoint();
  
#elif defined MODE_WIFI_CLIENT
  // Parameters for running as wifi client
  const char *ssid = "dd-wrt";
  const char *password = "ELGALLOGRINGO";
  MDNSResponder mdns;

  void setupWiFiClient();
#endif


ESP8266WebServer server ( 80 );



void setup ( void ) {
	Serial.begin ( 115200 );

#if defined MODE_WIFI_AP
  setupWiFiAccessPoint();
  
  
#elif defined MODE_WIFI_CLIENT
	setupWiFiClient();

#endif

  setupPageHandlers();
  
//	server.on ( "/test", []() {
//		server.send ( 200, "text/plain", "this works as well" );
//	} );
//
//  server.on ( "/", []() {    
//    sendBinaryFile("text/html", _index_html, _index_html_len);    
//  } );
//
//  server.on ( "/index.html", []() {    
//    sendBinaryFile("text/html", _index_html, _index_html_len);    
//  } );
//  
//  server.on ( "/css/skeleton.css", []() {
//    sendBinaryFile("text/css", css_skeleton_css, css_skeleton_css_len);    
//  } );
//
//  server.on ( "/css/normalize.css", []() {
//    sendBinaryFile("text/css", css_normalize_css, css_normalize_css_len);    
//  } );
//
//  server.on ("/light_the_a.html", []() {
//    server.send ( 200, "text/plain", "WE WON!!!!!!" );
//  } );
//
//  server.on ( "/images/favicon.png", []() {    
//    sendBinaryFile("image/png", images_favicon_png, images_favicon_png_len);    
//  });

	server.onNotFound ( handleNotFound );
	server.begin();
	Serial.println ( "HTTP server started" );
}



void loop ( void ) {
#if defined MODE_WIFI_CLIENT
	mdns.update();
#endif

	server.handleClient();
}

#if defined MODE_WIFI_AP

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

#elif defined MODE_WIFI_CLIENT

void setupWiFiClient()
{
  WiFi.begin ( ssid, password );
  //WiFi.begin ( ssid);

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

  if ( mdns.begin ( "esp8266", WiFi.localIP() ) ) {
    Serial.println ( "MDNS responder started" );
  }
}

#endif

