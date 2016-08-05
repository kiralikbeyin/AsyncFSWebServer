
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Config.h"

#define DBG_PORT Serial
#define DEBUG
#define DEBUG_DYNAMICDATA

AsyncWebServer server(80);

void send_connection_state_values_html(AsyncWebServerRequest * request)
{

	String state = "N/A";
	String Networks = "";
	if (WiFi.status() == 0) state = "Idle";
	else if (WiFi.status() == 1) state = "NO SSID AVAILBLE";
	else if (WiFi.status() == 2) state = "SCAN COMPLETED";
	else if (WiFi.status() == 3) state = "CONNECTED";
	else if (WiFi.status() == 4) state = "CONNECT FAILED";
	else if (WiFi.status() == 5) state = "CONNECTION LOST";
	else if (WiFi.status() == 6) state = "DISCONNECTED";



	int n = WiFi.scanNetworks();
	DBG_PORT.println(n);

	if (n == 0)
	{
		Networks = "<font color='#FF0000'>No networks found!</font>";
	}
	else
	{


		Networks = "Found " + String(n) + " Networks<br>";
		Networks += "<table border='0' cellspacing='0' cellpadding='3'>";
		Networks += "<tr bgcolor='#DDDDDD' ><td><strong>Name</strong></td><td><strong>Quality</strong></td><td><strong>Enc</strong></td><tr>";
		for (int i = 0; i < n; ++i)
		{
			int quality = 0;
			if (WiFi.RSSI(i) <= -100)
			{
				quality = 0;
			}
			else if (WiFi.RSSI(i) >= -50)
			{
				quality = 100;
			}
			else
			{
				quality = 2 * (WiFi.RSSI(i) + 100);
			}


			Networks += "<tr><td><a href='javascript:selssid(\"" + String(WiFi.SSID(i)) + "\")'>" + String(WiFi.SSID(i)) + "</a></td><td>" + String(quality) + "%</td><td>" + String((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*") + "</td></tr>";
		}
		Networks += "</table>";
	}

	String values = "";
	values += "connectionstate|" + state + "|div\n";
	values += "networks|" + Networks + "|div\n";
	request->send(200, "text/plain", values);
#ifdef DEBUG_DYNAMICDATA
	DBG_PORT.println(__FUNCTION__);
#endif // DEBUG_DYNAMICDATA

}

void send_network_configuration_values_html(AsyncWebServerRequest * request)
{

	String values = "";

	values += "ssid|" + (String)config.ssid + "|input\n";
	values += "password|" + (String)config.password + "|input\n";
	values += "ip_0|" + (String)config.IP[0] + "|input\n";
	values += "ip_1|" + (String)config.IP[1] + "|input\n";
	values += "ip_2|" + (String)config.IP[2] + "|input\n";
	values += "ip_3|" + (String)config.IP[3] + "|input\n";
	values += "nm_0|" + (String)config.Netmask[0] + "|input\n";
	values += "nm_1|" + (String)config.Netmask[1] + "|input\n";
	values += "nm_2|" + (String)config.Netmask[2] + "|input\n";
	values += "nm_3|" + (String)config.Netmask[3] + "|input\n";
	values += "gw_0|" + (String)config.Gateway[0] + "|input\n";
	values += "gw_1|" + (String)config.Gateway[1] + "|input\n";
	values += "gw_2|" + (String)config.Gateway[2] + "|input\n";
	values += "gw_3|" + (String)config.Gateway[3] + "|input\n";
	values += "dns_0|" + (String)config.DNS[0] + "|input\n";
	values += "dns_1|" + (String)config.DNS[1] + "|input\n";
	values += "dns_2|" + (String)config.DNS[2] + "|input\n";
	values += "dns_3|" + (String)config.DNS[3] + "|input\n";
	values += "dhcp|" + (String)(config.dhcp ? "checked" : "") + "|chk\n";

	request->send(200, "text/plain", values);
#ifdef DEBUG_DYNAMICDATA
	DBG_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG_DYNAMICDATA
}

String getContentType(String filename, AsyncWebServerRequest *request) {
	if (request->hasArg("download")) return "application/octet-stream";
	else if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".json")) return "application/json";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

bool handleFileRead(String path, AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
	DBG_OUTPUT_PORT.println("handleFileRead: " + path);
#endif // DEBUG_WEBSERVER
	/*if (CONNECTION_LED >= 0) {
		flashLED(CONNECTION_LED, 1, 25); // Show activity on LED
	}*/
	if (path.endsWith("/"))
		path += "index.htm";
	String contentType = getContentType(path,request);
	String pathWithGz = path + ".gz";
	if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
		if (SPIFFS.exists(pathWithGz))
			path += ".gz";
		//File file = SPIFFS.open(path, "r");
		request->send(SPIFFS, path, contentType);
			//server.streamFile(file, contentType);
		//DBG_OUTPUT_PORT.printf("File %s exist\n", file.name());
		//file.close();
		return true;
	}
#ifdef DEBUG_WEBSERVER
	else
		DBG_OUTPUT_PORT.printf("Cannot find %s\n", path.c_str());
#endif // DEBUG_WEBSERVER
	return false;
}


void setup()
{
	DBG_PORT.begin(115200);
	DBG_PORT.println();
#ifdef DEBUG
	DBG_PORT.setDebugOutput(true);
#endif // DEBUG
	if (CONNECTION_LED >= 0) {
		pinMode(CONNECTION_LED, OUTPUT); // CONNECTION_LED pin defined as output
	}
	if (AP_ENABLE_BUTTON >= 0) {
		pinMode(AP_ENABLE_BUTTON, INPUT); // If this pin is HIGH during startup ESP will run in AP_ONLY mode. Backdoor to change WiFi settings when configured WiFi is not available.
	}	

	if (AP_ENABLE_BUTTON >= 0) {
		apConfig.APenable = digitalRead(AP_ENABLE_BUTTON); // Read AP button
#ifdef DEBUG
		DBG_PORT.printf("AP Enable = %d\n", apConfig.APenable);
#endif // DEBUG
	}
	if (CONNECTION_LED >= 0) {
		digitalWrite(CONNECTION_LED, HIGH); // Turn LED off
	}

	//File System Init
	SPIFFS.begin();
#ifdef DEBUG
	{ // List files
		Dir dir = SPIFFS.openDir("/");
		while (dir.next()) {
			String fileName = dir.fileName();
			size_t fileSize = dir.fileSize();

			DBG_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
		}
		DBG_PORT.printf("\n");
	}
#endif // DEBUG

	if (!load_config()) { // Try to load configuration from SPIFFS
		defaultConfig(); // Load defaults if any error
		apConfig.APenable = true;
	}

	WiFi.hostname(config.DeviceName.c_str());
	if (AP_ENABLE_BUTTON >= 0) {
		if (apConfig.APenable) {
			ConfigureWifiAP(); // Set AP mode if AP button was pressed
		}
		else {
			ConfigureWifi(); // Set WiFi config
		}
	}
	else {
		ConfigureWifi(); // Set WiFi config
	}

#ifdef DEBUG
	DBG_PORT.print("Open http://");
	DBG_PORT.print(config.DeviceName);
	DBG_PORT.println(".local/edit to see the file browser");
	DBG_PORT.printf("Flash chip size: %u\n", ESP.getFlashChipRealSize());
	DBG_PORT.printf("Scketch size: %u\n", ESP.getSketchSize());
	DBG_PORT.printf("Free flash space: %u\n", ESP.getFreeSketchSpace());
#endif

	//Send OTA events to the browser
	ArduinoOTA.onStart([]() { DBG_PORT.println("OTA Update Start"); });
	ArduinoOTA.onEnd([]() { 
		SPIFFS.end();
		DBG_PORT.println("OTA Update End"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		char p[32];
		sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
		DBG_PORT.println(p);
	});
	ArduinoOTA.onError([](ota_error_t error) {
		if (error == OTA_AUTH_ERROR) DBG_PORT.println("OTA Auth Failed");
		else if (error == OTA_BEGIN_ERROR) DBG_PORT.println("OTA Begin Failed");
		else if (error == OTA_CONNECT_ERROR) DBG_PORT.println("OTA Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) DBG_PORT.println("OTA Recieve Failed");
		else if (error == OTA_END_ERROR) DBG_PORT.println("OTA End Failed");
	});
	ArduinoOTA.begin();

	server.serveStatic("/config.html",SPIFFS,"/config.html");
	server.on("/admin/connectionstate", [](AsyncWebServerRequest *request) {
		send_connection_state_values_html(request);
	});
	server.on("/admin/values", [](AsyncWebServerRequest *request) {
		send_network_configuration_values_html(request);
	});
	server.onNotFound([](AsyncWebServerRequest *request) {
		if (!handleFileRead(request->url(), request))
			request->send(404, "text/plain", "FileNotFound");
	});
	
	server.begin();
	//DBG_PORT.println(WiFi.scanNetworks());
}

void loop()
{

	ArduinoOTA.handle();

}
