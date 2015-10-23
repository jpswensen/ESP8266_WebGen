#include "WebFramework.h"

extern ESP8266WebServer server;

void sendBinaryFile(String mimeType, const char *realword, int realLen)
{
  static int failCount = 0;
  char chopped_send_buffer[BINARY_BUFFER_SIZE];
  
  WiFiClient client = server.client();
  client.setNoDelay(true);
  server.setContentLength(realLen);
  server.send(200, mimeType, "");
    
  int remaining = realLen;
  const char * offsetPtr = realword;
  int nSize = BINARY_BUFFER_SIZE;

  while (remaining > 0)
  {
    // print content
    if (nSize > remaining)
      nSize = remaining;      // Partial buffer left to send

    // Copy the image to a local buffer
//    Serial.print("Begin pgm_read_bytes:" );
//    Serial.println(millis());
    for (int offset = 0 ; offset < nSize ; offset++)
    {
      chopped_send_buffer[offset] = pgm_read_byte (offsetPtr + offset);
    }
//    Serial.print("End pgm_read_bytes:" );
//    Serial.println(millis());

    size_t bytesSent = client.write((const uint8_t *)chopped_send_buffer, nSize);
    if (bytesSent != nSize)
    {
      for (int offset = 0 ; offset < 5; offset++)
      {
        Serial.print(chopped_send_buffer[offset],HEX);
      }
      Serial.println("");
      Serial.println("Failed to send data");
      Serial.println(nSize);
      Serial.println(bytesSent);
      failCount++;

      if (failCount > 5)
      {
        Serial.println("*** Failed to send data. Terminating send.");
        return;
      }
    }
    else
    {
      // more content to print?
      remaining -= nSize;
      offsetPtr += nSize;
    }
  }
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );

}



