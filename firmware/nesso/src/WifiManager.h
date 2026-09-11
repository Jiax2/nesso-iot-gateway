#pragma once 

#include <WiFi.h> 

class WifiManager{

  public: 

    WifiManager(
      const char* ssid, const char* password
    ); 

    void begin(); 
    void loop(); 
    bool connected() const; 
    int32_t rssi() const; 
    IPAddress localIP() const; 

  
  private: 
    
    const char* _ssid; 
    const char* _password; 
    unsigned long _lastAttempt; 
    bool _wasConnected; 
    void connect(); 
}; 