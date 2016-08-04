// Config.h

#ifndef _CONFIG_h
#define _CONFIG_h

#define DEBUG_CONFIGH
#define CONFIG_FILE "/config.json"
#define SECRET_FILE "/secret.json"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


//#define CONNECTION_LED 0 // Connection LED pin (External)

#define CONNECTION_LED 2 // Connection LED pin (Built in)
#define AP_ENABLE_BUTTON 4 // Button pin to enable AP during startup for configuration

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <FS.h>
//#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

typedef struct {
	String ssid;
	String password;
	byte  IP[4];
	byte  Netmask[4];
	byte  Gateway[4];
	byte  DNS[4];
	boolean dhcp;
	String ntpServerName;
	long Update_Time_Via_NTP_Every;
	long timezone;
	boolean daylight;
	String DeviceName;
	//int connectionLed;
} strConfig;

typedef struct {
	String APssid = "ESP"; // ChipID is appended to this name
	String APpassword = "12345678";
	boolean APenable = false; // AP disabled by default
} strApConfig;

typedef struct {
	boolean auth;
	String wwwUsername = "";
	String wwwPassword = "";
} strHTTPAuth;

extern strConfig config; // General and WiFi configuration
extern strApConfig apConfig; // Static AP config settings
extern strHTTPAuth httpAuth;

/**
* converts a single hex digit character to its integer value
* @param[in] hex character
* @param[out] number that represents hex digit
*/
unsigned char h2int(char c);

/**
* Loads default system configuration
*/
void defaultConfig();

/**
* Loads system configuration from SPIFFS file
* @param[out] true if OK
*/
boolean load_config();

/**
* Saves system configuration to SPIFFS file
* @param[out] true if OK
*/
boolean save_config();

/**
* Loads HTTP secret from SPIFFS file
* @param[out] true if OK
*/
boolean loadHTTPAuth();

/**
* Saves HTTP secret to SPIFFS file
* @param[out] true if OK
*/
boolean saveHTTPAuth();

String formatBytes(size_t bytes);

void ConfigureWifiAP();

void ConfigureWifi();

#endif

