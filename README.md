# simple_espnow_esp8266
A simple example of using esp-now protocol between two ESP8266 boards (not ESP32)

You will need to run the slave.ino file first and check the serial monitor to get the MAC address. Update the line in the master.ino file with that MAC address in order for the two ESPs to communicate.

I found that esp-now is not very reliable on the ESP8266, so the master.ino script checks for a successful transmission and retries every 100 milliseconds until it gets a success. After 10 seconds, it tries again.
