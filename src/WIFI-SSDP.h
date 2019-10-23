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

/*void init_SSDP(void) {
  // SSDP дескриптор
  HTTP.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });
  //Если версия  2.0.0 закаментируйте следующую строчку
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
}*/