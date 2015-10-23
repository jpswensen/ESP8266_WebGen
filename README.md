# ESP8266_WebGen
Design a website and then embed it in the ESP8266 firmware. This
project is now subdivided into three different sketches:

* AdvancedWebServerHuzzah: This is a very basic example that lets you
choose either acting as an access point or as a wifi client.
* AdvancedWebServerHuzzah_APandClient: This one is similar to the
  basic examples except that it includes the ability to switch between
  the access point or provide an SSID and (optional) password. This
  also gives examples of how to use jQuery and code-implemented URL
  handlers to have dynamics content on the web pages. This technique
  is still embedding the webpage in PROGMEM as C arrays.
* AdvancedWebServerHuzzah_SPIFFS: This final technique uses the SPIFFS
  flash filesystem to implement the website in FLASH memory such that
  the items can be retrieved just like files on an external SD
  card. The SPIFFS writing utility (which can be added to the
  ArduinoIDE following instructions at
  http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html)
  seems to require you to write the entire 3MB of reserved file space
  every time. This takes forever. So, in addition to the faster
  example when in PROGMEM above, I implemented a modified version of
  the Arduino WebUpdate example to allow us to upload just the HTML,
  JS, CSS, and image files. This can be done by sending commands like
  the following: curl -F
  "file=@css/dropdown.css;filename=/css/dropdown.css" 192.168.4.1/update. I
  will eventually make another Python script to generate the setup
  code for the webserver to retrieve the web files from SPIFF and to
  update all the website files on the device. 


# How to use the script for AdvancedWebServerHuzzah and AdvancedWebServerHuzzah_APandClient:
1. Edit the webpage as you see fit
2. Run the following command:
    python generate_arduino_webpage.py > website.h
3. Copy the website.h file into the Arduino project
4. Compile the Arduino project and load to the board

# How to use the script for AdvancedWebServerHuzzah_SPIFFS 
1. Follow the instructions at
   http://arduino.esp8266.com/versions/1.6.5-1160-gef26c5f/doc/reference.html
   to get the SPIFF data uploaded to the device at lead one time. This
   ensures the FLASH space is "formatted" for SPIFFS.
2. Load the AdvancedWebServerHuzzah_SPIFFS program to the board and it
should start serving the website
3. Update the SPIFFS files either using curl and the "update" URL or
   by flashing the whole thing again from ArduinoIDE


# Notes:
* In the APandClient code, you can force the device to revert to the AccessPointMode by pulling pin 12 low at startup. This resets the EEPROM, so you will have to input the ssid and password again after connecting to the device acting as an access point. I am contemplating making the device revert to an access point if it can't connect to a router for 30 seconds even when in WifiClientMode.
* This should handle any html, css, jpg, or png file
* For intereacting with hardware, you will need to add handlers for URLs that aren't static. I eventually want to improve the parser so that any links that don't have an associated HMTL file in the folder will generate a stub function.
* This webpage was based on the Skeleton Responsive CSS Boilerplate code (http://getskeleton.com/)
