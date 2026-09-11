#include "WifiManager.h"
#include "config.h"

WifiManager::WifiManager(
  const char* ssid, const char* password
)
  : _ssid(ssid), 
    _password(password), 
    _lastAttempt(0),
    _wasConnected(false)
{

}

void WifiManager::begin()
{
    Serial.println("[WiFi] Initializing...");

    WiFi.mode(WIFI_STA);

    WiFi.setAutoReconnect(true);

    connect();
}

void WifiManager::loop(){

  const bool isConnected =  WiFi.status() == WL_CONNECTED; 

  if(isConnected && !_wasConnected){
    Serial.println(); 
    Serial.println("[WiFi] Connected");

    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());

    Serial.print("[WiFi] RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  }

  if(!isConnected){
    unsigned long now = millis(); 

    if(
      now - _lastAttempt >= Config::WIFI_RETRY_MS
    )
    {
      connect(); 
    }
  }
  _wasConnected = isConnected; 
}

void WifiManager::connect(){
  
  _lastAttempt = millis(); 

  Serial.print("[WiFi] Connecting to"); 
  Serial.println(_ssid); 

  WiFi.begin(
    _ssid, _password
  ); 
}

bool WifiManager::connected() const {

    return WiFi.status() == WL_CONNECTED;
}


int32_t WifiManager::rssi() const {

    if (!connected()) {
        return 0;
    }

    return WiFi.RSSI();
}


IPAddress WifiManager::localIP() const {

    return WiFi.localIP();
}