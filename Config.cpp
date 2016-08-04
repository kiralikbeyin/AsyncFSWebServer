// 
// 
// 

#define DBG_OUTPUT_PORT Serial

#include "Config.h"
//#include "FSWebServer.h"
//#include "DynamicData.h"


strConfig config;
strApConfig apConfig;
strHTTPAuth httpAuth;


// convert a single hex digit character to its integer value (from https://code.google.com/p/avr-netino/)
unsigned char h2int(char c)
{
	if (c >= '0' && c <= '9') {
		return((unsigned char)c - '0');
	}
	if (c >= 'a' && c <= 'f') {
		return((unsigned char)c - 'a' + 10);
	}
	if (c >= 'A' && c <= 'F') {
		return((unsigned char)c - 'A' + 10);
	}
	return(0);
}

//format bytes
String formatBytes(size_t bytes) {
	if (bytes < 1024) {
		return String(bytes) + "B";
	}
	else if (bytes < (1024 * 1024)) {
		return String(bytes / 1024.0) + "KB";
	}
	else if (bytes < (1024 * 1024 * 1024)) {
		return String(bytes / 1024.0 / 1024.0) + "MB";
	}
	else {
		return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
	}
}

void ConfigureWifi()
{
	WiFi.mode(WIFI_STA);
#ifdef DEBUG_CONFIGH
	DBG_OUTPUT_PORT.printf("Connecting to %s\n", config.ssid.c_str());
#endif // DEBUG_CONFIGH
	WiFi.begin(config.ssid.c_str(), config.password.c_str());
	if (!config.dhcp)
	{
#ifdef DEBUG_CONFIGH
		DBG_OUTPUT_PORT.println("NO DHCP");
#endif // DEBUG_CONFIGH
		WiFi.config(
			IPAddress(config.IP[0], config.IP[1], config.IP[2], config.IP[3]),
			IPAddress(config.Gateway[0], config.Gateway[1], config.Gateway[2], config.Gateway[3]),
			IPAddress(config.Netmask[0], config.Netmask[1], config.Netmask[2], config.Netmask[3]),
			IPAddress(config.DNS[0], config.DNS[1], config.DNS[2], config.DNS[3])
		);
	}
	delay(1000); // Wait for WiFi


	while (!WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
	}

	DBG_OUTPUT_PORT.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
#ifdef DEBUG_CONFIGH
	DBG_OUTPUT_PORT.printf("Gateway:    %s\n", WiFi.gatewayIP().toString().c_str());
	DBG_OUTPUT_PORT.printf("DNS:        %s\n", WiFi.dnsIP().toString().c_str());
	Serial.println(__PRETTY_FUNCTION__);
#endif // DEBUG_CONFIGH
}

void ConfigureWifiAP() {
#ifdef DEBUG_GLOBALH
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG_GLOBALH
	//WiFi.disconnect();
	WiFi.mode(WIFI_AP);
	String APname = apConfig.APssid + (String)ESP.getChipId();
	if (httpAuth.wwwPassword != "")
		WiFi.softAP(APname.c_str(), httpAuth.wwwPassword.c_str());
	else
		WiFi.softAP(APname.c_str());
}

void defaultConfig (){
	// DEFAULT CONFIG
	config.ssid = "YOUR_DEFAULT_WIFI_SSID";
	config.password = "YOUR_DEFAULT_WIFI_PASSWD";
	config.dhcp = true;
	config.IP[0] = 192; config.IP[1] = 168; config.IP[2] = 1; config.IP[3] = 4;
	config.Netmask[0] = 255; config.Netmask[1] = 255; config.Netmask[2] = 255; config.Netmask[3] = 0;
	config.Gateway[0] = 192; config.Gateway[1] = 168; config.Gateway[2] = 1; config.Gateway[3] = 1;
	config.DNS[0] = 192; config.DNS[1] = 168; config.DNS[2] = 1; config.DNS[3] = 1;
	config.ntpServerName = "es.pool.ntp.org";
	config.Update_Time_Via_NTP_Every = 5;
	config.timezone = 10;
	config.daylight = true;
	config.DeviceName = "ESP8266fs";
	//config.connectionLed = CONNECTION_LED;
	save_config();
#ifdef DEBUG
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
}

boolean load_config() {
#ifdef DEBUG
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
	File configFile = SPIFFS.open(CONFIG_FILE, "r");
	if (!configFile) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Failed to open config file");
#endif // DEBUG
		return false;
	}

	size_t size = configFile.size();
	if (size > 1024) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Config file size is too large");
#endif
		configFile.close();
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	configFile.close();
#ifdef DEBUG
	DBG_OUTPUT_PORT.print("JSON file size: "); Serial.print(size); Serial.println(" bytes");
#endif

	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Failed to parse config file");
#endif // DEBUG
		return false;
	}
#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	Serial.println(temp);
#endif
	//memset(config.ssid, 0, 28);
	//memset(config.pass, 0, 50);
	//String("Virus_Detected!!!").toCharArray(config.ssid, 28); // Assign WiFi SSID
	//String("LaJunglaSigloXX1@.").toCharArray(config.pass, 50); // Assign WiFi PASS

	config.ssid = json["ssid"].asString();
	//String(ssid_str).toCharArray(config.ssid, 28);

	config.password = json["pass"].asString();
	
	config.IP[0] = json["ip"][0];
	config.IP[1] = json["ip"][1];
	config.IP[2] = json["ip"][2];
	config.IP[3] = json["ip"][3];

	config.Netmask[0] = json["netmask"][0];
	config.Netmask[1] = json["netmask"][1];
	config.Netmask[2] = json["netmask"][2];
	config.Netmask[3] = json["netmask"][3];

	config.Gateway[0] = json["gateway"][0];
	config.Gateway[1] = json["gateway"][1];
	config.Gateway[2] = json["gateway"][2];
	config.Gateway[3] = json["gateway"][3];

	config.DNS[0] = json["dns"][0];
	config.DNS[1] = json["dns"][1];
	config.DNS[2] = json["dns"][2];
	config.DNS[3] = json["dns"][3];

	config.dhcp = json["dhcp"];

	//String(pass_str).toCharArray(config.pass, 28);
	config.ntpServerName = json["ntp"].asString();
	config.Update_Time_Via_NTP_Every = json["NTPperiod"];
	config.timezone = json["timeZone"];
	config.daylight = json["daylight"];
	config.DeviceName = json["deviceName"].asString();

	//config.connectionLed = json["led"];

#ifdef DEBUG
	DBG_OUTPUT_PORT.println("Data initialized.");
	DBG_OUTPUT_PORT.print("SSID: "); Serial.println(config.ssid);
	DBG_OUTPUT_PORT.print("PASS: "); Serial.println(config.password);
	DBG_OUTPUT_PORT.print("NTP Server: "); Serial.println(config.ntpServerName);
	//DBG_OUTPUT_PORT.printf("Connection LED: %d\n", config.connectionLed);
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
	return true;
}

boolean save_config() {
	//flag_config = false;
#ifdef DEBUG
	DBG_OUTPUT_PORT.println("Save config");
#endif
	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["ssid"] = config.ssid;
	json["pass"] = config.password;
	
	JsonArray& jsonip = json.createNestedArray("ip");
	jsonip.add(config.IP[0]);
	jsonip.add(config.IP[1]);
	jsonip.add(config.IP[2]);
	jsonip.add(config.IP[3]);
	
	JsonArray& jsonNM = json.createNestedArray("netmask");
	jsonNM.add(config.Netmask[0]);
	jsonNM.add(config.Netmask[1]);
	jsonNM.add(config.Netmask[2]);
	jsonNM.add(config.Netmask[3]);
	
	JsonArray& jsonGateway = json.createNestedArray("gateway");
	jsonGateway.add(config.Gateway[0]);
	jsonGateway.add(config.Gateway[1]);
	jsonGateway.add(config.Gateway[2]);
	jsonGateway.add(config.Gateway[3]);
	
	JsonArray& jsondns = json.createNestedArray("dns");
	jsondns.add(config.DNS[0]);
	jsondns.add(config.DNS[1]);
	jsondns.add(config.DNS[2]);
	jsondns.add(config.DNS[3]);
	
	json["dhcp"] = config.dhcp;
	json["ntp"] = config.ntpServerName;
	json["NTPperiod"] = config.Update_Time_Via_NTP_Every;
	json["timeZone"] = config.timezone;
	json["daylight"] = config.daylight;
	json["deviceName"] = config.DeviceName;

	//json["led"] = config.connectionLed;
			
	//TODO add AP data to html
	File configFile = SPIFFS.open(CONFIG_FILE, "w");
	if (!configFile) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Failed to open config file for writing");
#endif // DEBUG
		configFile.close();
		return false;
	}

#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	Serial.println(temp);
#endif

	json.printTo(configFile);
	configFile.flush();
	configFile.close();
	return true;
}

boolean loadHTTPAuth() {
	File configFile = SPIFFS.open(SECRET_FILE, "r");
	if (!configFile) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Failed to open secret file");
#endif // DEBUG
		httpAuth.auth = false;
		httpAuth.wwwUsername = "";
		httpAuth.wwwPassword = "";
		configFile.close();
		return false;
	}

	size_t size = configFile.size();
	if (size > 256) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Secret file size is too large");
#endif
		httpAuth.auth = false;
		configFile.close();
		return false;
	}

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);

	// We don't use String here because ArduinoJson library requires the input
	// buffer to be mutable. If you don't use ArduinoJson, you may as well
	// use configFile.readString instead.
	configFile.readBytes(buf.get(), size);
	configFile.close();
#ifdef DEBUG
	DBG_OUTPUT_PORT.printf("JSON secret file size: %d %s\n", size, "bytes");
#endif

	StaticJsonBuffer<256> jsonBuffer;
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
#ifdef DEBUG
		String temp;
		json.prettyPrintTo(temp);
		DBG_OUTPUT_PORT.println(temp);
		DBG_OUTPUT_PORT.println("Failed to parse secret file");
#endif // DEBUG
		httpAuth.auth = false;
		return false;
	}
#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	DBG_OUTPUT_PORT.println(temp);
#endif
	//memset(config.ssid, 0, 28);
	//memset(config.pass, 0, 50);
	//String("Virus_Detected!!!").toCharArray(config.ssid, 28); // Assign WiFi SSID
	//String("LaJunglaSigloXX1@.").toCharArray(config.pass, 50); // Assign WiFi PASS

	httpAuth.auth = json["auth"];
	httpAuth.wwwUsername = json["user"].asString();
	httpAuth.wwwPassword = json["pass"].asString();

#ifdef DEBUG
	DBG_OUTPUT_PORT.println("Secret initialized.");
	DBG_OUTPUT_PORT.print("User: "); DBG_OUTPUT_PORT.println(httpAuth.wwwUsername);
	DBG_OUTPUT_PORT.print("Pass: "); DBG_OUTPUT_PORT.println(httpAuth.wwwPassword);
	DBG_OUTPUT_PORT.println(__PRETTY_FUNCTION__);
#endif // DEBUG
	return true;
}

boolean saveHTTPAuth() {
	//flag_config = false;
#ifdef DEBUG
	DBG_OUTPUT_PORT.println("Save secret");
#endif
	StaticJsonBuffer<256> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["auth"] = httpAuth.auth;
	json["user"] = httpAuth.wwwUsername;
	json["pass"] = httpAuth.wwwPassword;

	//TODO add AP data to html
	File configFile = SPIFFS.open(SECRET_FILE, "w");
	if (!configFile) {
#ifdef DEBUG
		DBG_OUTPUT_PORT.println("Failed to open secret file for writing");
#endif // DEBUG
		configFile.close();
		return false;
	}

#ifdef DEBUG
	String temp;
	json.prettyPrintTo(temp);
	Serial.println(temp);
#endif

	json.printTo(configFile);
	configFile.flush();
	configFile.close();
	return true;
}
