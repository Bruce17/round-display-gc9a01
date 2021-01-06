#include "../../include/general-inc.h"
#include "wifi-manager.h"


// Avoid issues with " multiple definition of `filesystem`"
#ifdef ESP32
  #if USE_LITTLEFS
    FS* filesystem = &LITTLEFS;
  #elif USE_SPIFFS
    FS* filesystem = &SPIFFS;
  #else
    FS* filesystem = &FFat;
  #endif

  WiFiMulti wifiMulti;
#else
  #if USE_LITTLEFS
    FS* filesystem = &LittleFS;
  #else
    FS* filesystem = &SPIFFS;
  #endif
  
  ESP8266WiFiMulti wifiMulti;
#endif


#include <ESP_DoubleResetDetector.h>


//DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);
DoubleResetDetector* drd;

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
const char* password = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

WM_Config WM_config;


// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

#if ( USE_DHCP_IP || ( defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP ) )
  // Use DHCP
  #warning Using DHCP IP
  IPAddress stationIP   = IPAddress(0, 0, 0, 0);
  IPAddress gatewayIP   = IPAddress(192, 168, 178, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#else
  // Use static IP
  #warning Using static IP
  #ifdef ESP32
    IPAddress stationIP   = IPAddress(192, 168, 178, 232);
  #else
    IPAddress stationIP   = IPAddress(192, 168, 178, 186);
  #endif

  IPAddress gatewayIP   = IPAddress(192, 168, 178, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS      true

IPAddress dns1IP = gatewayIP;
IPAddress dns2IP = IPAddress(1, 1, 1, 1);


#include <ESP_WiFiManager.h>



void heartBeatPrint(void) {
#ifdef DEBUG
  static int num = 1;

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("H");        // H means connected to WiFi
  }
  else {
    Serial.print("F");        // F means not connected to WiFi
  }

  if (num == 80) {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0) {
    Serial.print(" ");
  }
#endif
}

void check_WiFi(void) {
  if ( (WiFi.status() != WL_CONNECTED) ) {
  #ifdef DEBUG
    Serial.println("\nWiFi lost. Call connectMultiWiFi in loop");
  #endif
    connectMultiWiFi();
  }
}

void check_status(void) {
  static ulong checkstatus_timeout  = 0;
  static ulong checkwifi_timeout    = 0;

  static ulong current_millis;

#define WIFICHECK_INTERVAL    1000L
#define HEARTBEAT_INTERVAL    10000L

  current_millis = millis();

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0)) {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0)) {
  #if DEBUG_HEARTBEAT
    heartBeatPrint();
  #endif
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }
}

void loadConfigData(void) {
  File file = FileFS.open(CONFIG_FILENAME, "r");
#ifdef DEBUG
  LOGERROR(F("LoadWiFiCfgFile "));
#endif

  if (file) {
    file.readBytes((char *) &WM_config, sizeof(WM_config));
    file.close();
  #ifdef DEBUG
    LOGERROR(F("OK"));
  #endif
  }
  else {
  #ifdef DEBUG
    LOGERROR(F("failed"));
  #endif
  }
}

void saveConfigData(void) {
  File file = FileFS.open(CONFIG_FILENAME, "w");
#ifdef DEBUG
  LOGERROR(F("SaveWiFiCfgFile "));
#endif

  if (file) {
    file.write((uint8_t*) &WM_config, sizeof(WM_config));
    file.close();
  #ifdef DEBUG
    LOGERROR(F("OK"));
  #endif
  }
  else {
  #ifdef DEBUG
    LOGERROR(F("failed"));
  #endif
  }
}

uint8_t connectMultiWiFi(void) {
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS       0
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS       2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS           100L

  uint8_t status;

#ifdef DEBUG
  LOGERROR(F("ConnectMultiWiFi with :"));
#endif

  if ( (Router_SSID != "") && (Router_Pass != "") ) {
  #ifdef DEBUG
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
  #endif
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) ) {
    #ifdef DEBUG
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
    #endif
    }
  }

#ifdef DEBUG
  LOGERROR(F("Connecting MultiWifi..."));
#endif

  WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  WiFi.config(stationIP, gatewayIP, netMask, dns1IP, dns2IP);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  WiFi.config(stationIP, gatewayIP, netMask);
#endif
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ( ( i++ < 10 ) && ( status != WL_CONNECTED ) ) {
    status = wifiMulti.run();

    if ( status == WL_CONNECTED ) {
      break;
    }
    else {
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
    }
  }

#ifdef DEBUG
  if ( status == WL_CONNECTED ) {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
  }
  else {
    LOGERROR(F("WiFi not connected"));
  }
#endif

  return status;
}



void prepare_wifi() {
  // Initialize the LED digital pin as an output.
  pinMode(PIN_LED, OUTPUT);

#ifdef DEBUG
  Serial.print("\nStarting ConfigOnDoubleReset with DoubleResetDetect using " + String(FS_Name));
  Serial.println(" on " + String(ARDUINO_BOARD));
  Serial.println("ESP_WiFiManager Version " + String(ESP_WIFIMANAGER_VERSION));
  Serial.println("ESP_DoubleResetDetector Version " + String(ESP_DOUBLERESETDETECTOR_VERSION));

  Serial.setDebugOutput(false);
#endif

  if (FORMAT_FILESYSTEM)
    FileFS.format();

  // Format FileFS if not yet
#ifdef ESP32
  if (!FileFS.begin(true))
#else
  if (!FileFS.begin())
#endif
  {
  #ifdef DEBUG
    Serial.print(FS_Name);
    Serial.println(F(" failed! AutoFormatting."));
  #endif

#ifdef ESP8266
    FileFS.format();
#endif
  }

  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);

  unsigned long startedAt = millis();

  // Local intialization. Once its business is done, there is no need to keep it around. Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESP_WiFiManager ESP_wifiManager;
  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESP_WiFiManager ESP_wifiManager("WifiClock");


  ESP_wifiManager.setMinimumSignalQuality(-1);

  // From v1.0.10 only
  // Set config portal channel, default = 1. Use 0 => random channel from 1-13
  ESP_wifiManager.setConfigPortalChannel(0);

#if !USE_DHCP_IP
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  ESP_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask, dns1IP, dns2IP);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  ESP_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask);
#endif
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESP_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");  
#endif

  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();

  // Remove this line if you do not want to see WiFi password printed
  // Serial.println("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  // SSID to uppercase
  ssid.toUpperCase();

  // From v1.1.0, Don't permit NULL password
  if ( (Router_SSID != "") && (Router_Pass != "") ) {
  #ifdef DEBUG
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
  #endif
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());

    ESP_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
  #ifdef DEBUG
    Serial.println("Got stored Credentials. Timeout 120s for Config Portal");
  #endif
  }
  else {
  #ifdef DEBUG
    Serial.println("Open Config Portal without Timeout: No stored Credentials.");
  #endif
    initialConfig = true;
  }

  if (drd->detectDoubleReset()) {
    // DRD, disable timeout.
    ESP_wifiManager.setConfigPortalTimeout(0);

  #ifdef DEBUG
    Serial.println("Open Config Portal without Timeout: Double Reset Detected");
  #endif
    initialConfig = true;
  }

  if (initialConfig) {
  #ifdef DEBUG
    Serial.println("Starting configuration portal.");
  #endif
    // Turn the LED on by making the voltage LOW to tell us we are in configuration mode.
    digitalWrite(PIN_LED, LED_ON);

    // Sets timeout in seconds until configuration portal gets turned off. If not specified device will remain in configuration mode until switched off via webserver or device is restarted.
    // ESP_wifiManager.setConfigPortalTimeout(600);

    // Starts an access point
    if (!ESP_wifiManager.startConfigPortal((const char *) ssid.c_str(), password)) {
    #ifdef DEBUG
      Serial.println("Not connected to WiFi but continuing anyway.");
    #endif
    }
    else {
    #ifdef DEBUG
      Serial.println("WiFi connected...yeey :)");
    #endif
    }

    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
      String tempSSID = ESP_wifiManager.getSSID(i);
      String tempPW   = ESP_wifiManager.getPW(i);

      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1) {
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      }
      else {
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);
      }

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1) {
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      }
      else {
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);
      }

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) ) {
      #ifdef DEBUG
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
      #endif
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    saveConfigData();
  }

  // Turn led off as we are not in configuration mode.
  digitalWrite(PIN_LED, LED_OFF);

  startedAt = millis();

  if (!initialConfig) {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    loadConfigData();

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++) {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) ) {
      #ifdef DEBUG
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
      #endif
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

    if ( WiFi.status() != WL_CONNECTED ) {
    #ifdef DEBUG
      Serial.println("ConnectMultiWiFi in setup");
    #endif

      connectMultiWiFi();
    }
  }

#ifdef DEBUG
  Serial.print("After waiting ");
  Serial.print((float) (millis() - startedAt) / 1000L);
  Serial.print(" secs more in setup(), connection result is ");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("connected. Local IP: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println(ESP_wifiManager.getStatus(WiFi.status()));
  }
#endif
}

void wifi_loop() {
  // Call the double reset detector loop method every so often, so that it can recognise when the timeout expires. You can also call drd.stop() when you wish to no longer consider the next reset as a double reset.
  drd->loop();

  // put your main code here, to run repeatedly
  check_status();
}
