bool StartAPMode();

void init_WIFI() {
  // Попытка подключения к точке доступа  //WiFi.disconnect();
  WiFi.mode(WIFI_STA); 
  //WiFi.mode(WIFI_AP_STA);
  uint8_t tries = 10;
  WiFi.begin(ssid.c_str(), password.c_str());
  while (--tries && WiFi.status() != WL_CONNECTED)  {   // Делаем проверку подключения до тех пор пока счетчик tries не станет равен нулю или не получим подключение
    Serial.print(F("."));
    //bounce(); // Animation of a bouncing ball
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED)  {    // Если не удалось подключиться запускаем в режиме AP
    Serial.println(F("WiFi up AP"));
    if (StartAPMode())  {
      Serial.print(F("AP IP address: ")); Serial.println(WiFi.softAPIP()); 
    }
    else Serial.println(F("Can't create AP"));  
  }
  else {  // Иначе удалось подключиться отправляем сообщение о подключении и выводим адрес IP
    Serial.println(F("WiFi connected"));
    Serial.print(F("IP address: "));    Serial.println(WiFi.localIP());
  }
}

bool StartAPMode() {
  WiFi.disconnect();   // Отключаем WIFI
  WiFi.mode(WIFI_AP);   // Меняем режим на режим точки доступа
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // Задаем настройки сети
  // Включаем WIFI в режиме точки доступа с именем и паролем хранящихся в переменных _ssidAP _passwordAP
  if (WiFi.softAP(ssidAP.c_str(), passwordAP.c_str())) return true;
  else return false;
}

/*static const char* ssdpTemplate =
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u/</URLBase>"
    "<device>"
      "<deviceType>upnp:rootdevice</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>index.html</presentationURL>"
      "<serialNumber>%u</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>http://www.espressif.com</modelURL>"
      "<manufacturer>Espressif Systems</manufacturer>"
      "<manufacturerURL>http://www.espressif.com</manufacturerURL>"
      "<UDN>uuid:38323636-4558-4dda-9188-cda0e6%02x%02x%02x</UDN>"
    "</device>"
  "</root>\r\n"
  "\r\n";

void init_SSDP(void) {
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(SSDP_Name);
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("SSDP-LW");
  SSDP.setModelNumber("000000000001");
  SSDP.setModelURL("https://led-lightwell.eu/");
  SSDP.setManufacturer("LIGHTWELL");
  SSDP.setManufacturerURL("https://led-lightwell.eu");
  SSDP.begin();
  HTTP.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest *request) {
      StreamString output;
      if(output.reserve(1024)) {
        //uint32_t ip = WiFi.localIP();
#ifdef ESP32
        uint32_t chipId = ESP.getEfuseMac();
#else
        uint32_t chipId = ESP.getChipId();    
#endif  
Serial.print("chipId "); Serial.println(chipId);        
        output.printf(ssdpTemplate,
          WiFi.localIP().toString(), //IP2STR(&ip),
          SSDP_Name,
          chipId,
          modelName,
          nVersion,
          (uint8_t) ((chipId >> 16) & 0xff),
          (uint8_t) ((chipId >>  8) & 0xff),
          (uint8_t)   chipId        & 0xff
        );
        request->send(200, "text/xml", (String)output);
      } else {
        request->send(500);
      }
  });  
}*/