# ESP8266_WebGen
Design a website and then embed it in the ESP8266 firmware

# How to use
1. Edit the webpage as you see fit
2. Run the following command:
    python generate_arduino_webpage.py > website.h
3. Copy the website.h file into the Arduino project
4. Compile the Arduino project and load to the board

# Notes:
* In the APandClient code, you can force the device to revert to the AccessPointMode by pulling pin 12 low at startup. This resets the EEPROM, so you will have to input the ssid and password again after connecting to the device acting as an access point. I am contemplating making the device revert to an access point if it can't connect to a router for 30 seconds even when in WifiClientMode.
* This should handle any html, css, jpg, or png file
* For intereacting with hardware, you will need to add handlers for URLs that aren't static. I eventually want to improve the parser so that any links that don't have an associated HMTL file in the folder will generate a stub function.
* This webpage was based on the Skeleton Responsive CSS Boilerplate code (http://getskeleton.com/)
