#ifndef CORE_CORE_H
#define CORE_CORE_H

#include <Arduino.h>

#if defined(FIREBASE_CLIENT_VERSION)
#undef FIREBASE_CLIENT_VERSION
#endif

#define FIREBASE_CLIENT_VERSION "2.1.3"


#ifndef FIREBASE_CORE_IDLE_DELAY
#define FIREBASE_CORE_IDLE_DELAY 0
#endif


static void sys_idle()
{
#if defined(ARDUINO_ESP8266_MAJOR) && defined(ARDUINO_ESP8266_MINOR) && defined(ARDUINO_ESP8266_REVISION) && ((ARDUINO_ESP8266_MAJOR == 3 && ARDUINO_ESP8266_MINOR >= 1) || ARDUINO_ESP8266_MAJOR > 3)
    esp_yield();
#else
    delay(FIREBASE_CORE_IDLE_DELAY);
#endif
}

#endif