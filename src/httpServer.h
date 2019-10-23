void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

/*String translateEncryptionType(wifi_auth_mode_t encryptionType) {
   switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}*/

String translateEncryptionType(uint8_t encryptionType) {
   switch (encryptionType) {
    case (7):
      return "Open";
    case (5):
      return "WEP";
    case (2):
      return "WPA_PSK";
    case (4):
      return "WPA2_PSK";
    case (8):
      return "WPA_WPA2_PSK";
    default:
      return "UNKNOWN"; 
  }
}

String processor(const String& var){ //For only LIST of Wi-Fi
    String listWIFI="";
    //Serial.println(var);
    if(var == "LISTWIFI") {
        if (WiFi.status() != WL_CONNECTED) {
            listWIFI="<strong>LIST of Wi-Fi networks:</strong></br>";
            int n = WiFi.scanComplete();
            if(n == -2) {
                WiFi.scanNetworks(true);
            }
            else
                if(n) {
                    for (int i = 0; i < n; ++i) {
                        listWIFI += "ssid: <strong>"+WiFi.SSID(i)+"</strong> ";
                        listWIFI += "rssi: <strong>"+String(WiFi.RSSI(i))+"</strong> ";
                        //listWIFI += ",\"bssid\":\""+WiFi.BSSIDstr(i)+"\"";
                        //listWIFI += ",\"channel\":"+String(WiFi.channel(i));
                        listWIFI += "secure: "+String(translateEncryptionType(WiFi.encryptionType(i)));
                        listWIFI += "</br>";
                    }
                WiFi.scanDelete();
                if(WiFi.scanComplete() == -2) {
                    WiFi.scanNetworks(true);
                }
            }    
    }
    return listWIFI;
  }
  return String();
}

void init_HTTPServer(void) {
    // Route for root / web page
    HTTP.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
      // send a file when /index is requested
    HTTP.on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", "text/html");
    });
     // Route to load style.css file
    HTTP.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });  
    HTTP.on("/img/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/img/favicon.png", "image/x-icon");
    });      
    HTTP.on("/function.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/function.js", "application/javascript");
    }); 
    HTTP.on("/lang_EN.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/lang_EN.js", "application/javascript");
    });  
    HTTP.on("/lang_RU.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/lang_RU.js", "application/javascript");
    });    
    HTTP.on("/lang_BG.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/lang_BG.js", "application/javascript");
    });  
    HTTP.on("/wifi.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        //request->send(SPIFFS, "/wifi.html", "text/html");
        request->send(SPIFFS, "/wifi.html", String(), false, processor);
    });
    HTTP.on("/time.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/time.html", "text/html");
    });
    HTTP.on("/setup.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/setup.html", "text/html");
    });  
    HTTP.on("/mqtt.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/mqtt.html", "text/html");
    });    
    HTTP.on("/help.html", HTTP_ANY, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/help.html", "text/html");
    }); 
    HTTP.on("/sConfig.json", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/sConfig.json", "application/json");
    });     
    HTTP.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {   //Перезагрузка модуля по запросу вида /restart?device=ok      
        String restart = request->getParam("device")->value(); 
        if (restart == "ok") {                         // Если значение равно Ок
            request->send(200, "text/html", "Restart OK"); 
            ESP.restart();                                // перезагружаем модуль
        }
        else {                                        // иначе
            request->send(200, "text/html", "Restart NO");  // Oтправляем ответ No Reset
        }
    }); 
    HTTP.on("/resetConfig", HTTP_GET, [](AsyncWebServerRequest *request) {   //Reset og configuration file     
        String reset = request->getParam("device")->value(); 
        if (reset == "ok") {  
            //SPIFFS.format();
            SPIFFS.remove(filePath);
            Serial.println("ESP erase Config file");
            delay(3000);                                   // Если значение равно Ок
            request->send(200, "text/html", "Reset OK"); 
            delay(3000);
            ESP.restart();                                // перезагружаем модуль
        }
        else {                                        // иначе
            request->send(200, "text/html", "Reset NO");  // Oтправляем ответ No Reset
        }
    }); 
    HTTP.on("/configs.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        struct tm *tm;
        P.displaySuspend(true);
        time_t tn = time(NULL);
        tm = localtime(&tn);
        String root = "{}";  // Формировать строку для отправки в браузер json формат
        DynamicJsonDocument jsonDoc(5096); //4096
        DeserializationError error =  deserializeJson(jsonDoc, root);
        if (error) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            return;
        }     
        JsonObject json = jsonDoc.as<JsonObject>();    
        // Заполняем поля json
        json["SSDP"] = SSDP_Name;
        json["ssidAP"] = ssidAP;
        json["passwordAP"] = passwordAP;
        json["ssid"] = ssid;
        json["password"] = password;
        json["timezone"] = timezone;
        if (WiFi.status() != WL_CONNECTED)  {
            json["ip"] = WiFi.softAPIP().toString(); 
        }
        else {
            json["ip"] = WiFi.localIP().toString();
        }
        json["time"] = GetTime(true);
        json["date"] = GetDate();
        json["date_day"] = tm->tm_mday;
        json["date_month"] = tm->tm_mon + 1;
        json["date_year"] = tm->tm_year + 1900;
        json["isDayLightSaving"] = (isDayLightSaving?"checked":"");
        json["use_sync"] = (useRTC?"":"checked");
        json["time_h"] = String(tm->tm_hour);
        json["time_m"] = String(tm->tm_min);
        json["time_s"] = String(tm->tm_sec);    
        json["isLedClock"]=(isLedClock?"checked":"");json["isLedDate"]=(isLedDate?"checked":"");
        json["isLedWeather"]=(isLedWeather?"checked":"");json["isLedForecast"]=(isLedForecast?"checked":"");
        json["isLedSea"]=(isLedSea?"checked":"");
        json["weathFrom"] = weathFrom; json["weathTo"] = weathTo;
        json["fcastFrom"] = fcastFrom; json["fcastTo"] = fcastTo;
        json["clockFrom"] = clockFrom; json["clockTo"] = clockTo;
        json["dateFrom"] = dateFrom; json["dateTo"] = dateTo; 
        json["seaFrom"] = seaFrom; json["seaTo"] = seaTo;   
        json["ledText0"] = strText0;
        json["ledText1"] = strText1;
        json["ledText2"] = strText2;
        json["ledText3"] = strText3;
        json["isTxtOn0"]=(isTxtOn0?"checked":"");
        json["isTxtOn1"]=(isTxtOn1?"checked":"");
        json["isTxtOn2"]=(isTxtOn2?"checked":"");
        json["isTxtOn3"]=(isTxtOn3?"checked":"");
        json["txtFrom0"] = txtFrom0;
        json["txtFrom1"] = txtFrom1;
        json["txtFrom2"] = txtFrom2;
        json["txtFrom3"] = txtFrom3;
        json["txtTo0"] = txtTo0;
        json["txtTo1"] = txtTo1;
        json["txtTo2"] = txtTo2;
        json["txtTo3"] = txtTo3;
        json["isCrLine0"]=(isCrLine0?"checked":"");
        json["isCrLine1"]=(isCrLine1?"checked":"");
        json["isCrLine2"]=(isCrLine2?"checked":"");
        json["isCrLine3"]=(isCrLine3?"checked":"");
        json["global_start"] = global_start; json["global_stop"] = global_stop;
        json["fontUsed"] = fontUsed;
        json["brightd"] = brightd; json["brightn"] = brightn;
        json["speed_d"] = 28 - speedTicker;
        json["dmodefrom"] = dmodefrom; json["dmodeto"] = dmodeto;
        json["lang"] = (lang==0?"RU":lang==1?"BG":lang==2?"EN":"EN");
        json["weatherHost"] = W_URL;
        json["city_code"] = CITY_ID;
        json["w_api"] = W_API; 
        json["ntpserver"] = sNtpServerName;  
#if AKA_CLOCK == true
        json["mqttOn"]=(mqttOn?"checked":"");
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["mqtt_user"] = mqtt_user;
        json["mqtt_pass"] = mqtt_pass;
        json["mqtt_name"] = mqtt_name;
        json["mqtt_sub_crline"] = mqtt_sub_crline;
        json["mqtt_pub_temp"] = mqtt_pub_temp;
        json["mqtt_pub_hum"] = mqtt_pub_hum;
        json["mqtt_pub_press"] = mqtt_pub_press;
    #if USE_DHT == true
        json["temp_now"] = getTempDHT();
        json["hum_now"] = getHumDHT();
    #endif
    #if USE_BME280 == true
        json["temp_now"] = getTempBME280();
        json["hum_now"] = getHumBME280();
    #endif  
        json["isLedTHP"]=(isLedTHP?"checked":"");
        json["thpFrom"] = thpFrom; json["thpTo"] = thpTo;
        json["tspeakOn"] = (tspeakOn?"checked":"");
        json["tspeak_server"] = tspeak_server;
        json["tspeak_channal"] = tspeak_channal;
        json["tspeak_wapi"] = tspeak_wapi;   
        json["alarm1_h"] = myAlarm[0].alarm_h;
        json["alarm1_m"] = myAlarm[0].alarm_m;
        json["alarm1_stat"] = myAlarm[0].alarm_stat; 
        json["alarm2_h"] = myAlarm[1].alarm_h;
        json["alarm2_m"] = myAlarm[1].alarm_m;
        json["alarm2_stat"] = myAlarm[1].alarm_stat;        
#else
        json["mqttOn"]=(mqttOn?"checked":"");
        json["mqtt_server"] = mqtt_server;
        json["mqtt_port"] = mqtt_port;
        json["mqtt_user"] = mqtt_user;
        json["mqtt_pass"] = mqtt_pass;
        json["mqtt_name"] = mqtt_name;
        json["mqtt_sub_crline"] = mqtt_sub_crline;
        json["mqtt_pub_temp"] = ""; //Чтобы не выводить неопр.значение в форму
        json["mqtt_pub_hum"] = "";
        json["mqtt_pub_press"] = "";
        json["temp_now"] = "";
        json["hum_now"] = "";
        json["isLedTHP"]="";
        json["tspeakOn"] = "";
        json["tspeak_server"] = "";
        json["tspeak_channal"] = "";
        json["tspeak_wapi"] = "";      
#endif 
        json["cVersion"] = cVersion;  
        // Помещаем созданный json в переменную root
        root = "";
        serializeJson(json, root);
        request->send(200, "application/json", root);
        //root = String();
        P.displaySuspend(false);
    });    
    HTTP.on("/lang", HTTP_GET, [](AsyncWebServerRequest *request){    
        if(request->getParam("lang")->value() == "RU" && lang !=0) lang = 0; 
        if(request->getParam("lang")->value() == "BG" && lang !=1) lang = 1; 
        if(request->getParam("lang")->value() == "EN" && lang !=2) lang = 2; 
        else return; 
        Serial.print("Set lang: ");    Serial.println(request->getParam("lang")->value().c_str());
        saveConfig();
        if (isLedWeather || isLedForecast) {
            strWeather = GetWeather(); delay(1000); strWeatherFcast = GetWeatherForecast();
        }
        request->send(200, "text/html", "OK");
    });           
    HTTP.on("/ssid", HTTP_GET, [](AsyncWebServerRequest *request){    // SSID и пароль роутера, AP, SSDP /ssid?ssid=home2&password=12345678&ssidAP=home1&passwordAP=8765439&ssdp?ssdp=proba
        ssid = request->getParam("ssid")->value();    // Получаем значение ssid из запроса сохраняем в глобальной переменной
        password = request->getParam("password")->value();  // Получаем значение password из запроса сохраняем в глобальной переменной
        ssidAP = request->getParam("ssidAP")->value();// Получаем значение ssidAP из запроса сохраняем в глобальной переменной
        passwordAP = request->getParam("passwordAP")->value();// Получаем значение passwordAP из запроса сохраняем в глобальной переменной
        SSDP_Name = request->getParam("ssdp")->value();
        saveConfig();  
          Serial.println("Set SSID: " + ssid + ", Set password: " + password + ", Set SSID AP: " + ssidAP + ", Set AP password: " + passwordAP + ", SSDP: " + SSDP_Name); 
        request->send(200, "text/html", "OK");        
    });  
    HTTP.on("/Time", HTTP_GET, [](AsyncWebServerRequest *request){    
        timeSynch();
        Serial.println(F("timeSynch()")); 
        request->send(200, "text/html", "OK");        
    }); 
    HTTP.on("/TimeZone", HTTP_GET, [](AsyncWebServerRequest *request){    
        timezone = request->getParam("timezone")->value().toInt(); // Получаем значение timezone из запроса конвертируем в int сохраняем в глобальной переменной
        isDayLightSaving = request->getParam("isDayLightSaving")->value().toInt(); 
        saveConfig();
        timeSynch();
        Serial.println("NTP Time Zone: " + String(timezone) + ",  isDayLightSaving: " + String(isDayLightSaving));
        request->send(200, "text/html", "OK");        
    }); 
    HTTP.on("/setntpserver", HTTP_GET, [](AsyncWebServerRequest *request){
        sNtpServerName = request->getParam("ntpserver")->value().c_str(); 
        #if USE_RTC == true
            request->getParam("use_sync")->value().toInt()==1?useRTC=false:useRTC=true;
        #else 
            useRTC=false;
        #endif
        saveConfig();
        timeSynch();
        Serial.println("sNtpServerName: " + sNtpServerName + ", useRTC: " + useRTC);
        request->send(200, "text/html", "OK"); 
    }); 
#if USE_RTC == true    
    HTTP.on("/TimeNew", HTTP_GET, [](AsyncWebServerRequest *request){
        struct tm timeman;
        time_t t;
        struct timeval now;
        timeman.tm_hour = request->getParam("time_h")->value().toInt(); 
        timeman.tm_min = request->getParam("time_m")->value().toInt();
        timeman.tm_sec = request->getParam("time_s")->value().toInt();
        timeman.tm_mday = request->getParam("date_day")->value().toInt();
        timeman.tm_mon = request->getParam("date_month")->value().toInt();// - 1;
        timeman.tm_year = request->getParam("date_year")->value().toInt() - 1900;   
        t = mktime(&timeman);
        printf("Setting time: %s", asctime(&timeman));  
        now = { .tv_sec = t };
        settimeofday(&now, NULL);
        rtc.adjust(t); 
        useRTC = true;
        saveConfig();
        timeSynch();
        request->send(200, "text/html", "OK"); 
    }); 
#endif   

    HTTP.on("/ledoption", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("isLedWeather")->value().toInt()==1?isLedWeather=true:isLedWeather=false; 
        request->getParam("isLedForecast")->value().toInt()==1?isLedForecast=true:isLedForecast=false;
        request->getParam("isLedClock")->value().toInt()==1?isLedClock=true:isLedClock=false;
        request->getParam("isLedDate")->value().toInt()==1?isLedDate=true:isLedDate=false;
        request->getParam("isLedSea")->value().toInt()==1?isLedSea=true:isLedSea=false;       
        weathFrom = request->getParam("weathFrom")->value().toFloat(); weathTo = request->getParam("weathTo")->value().toFloat();
        fcastFrom = request->getParam("fcastFrom")->value().toFloat(); fcastTo = request->getParam("fcastTo")->value().toFloat();
        clockFrom = request->getParam("clockFrom")->value().toFloat(); clockTo = request->getParam("clockTo")->value().toFloat();
        dateFrom = request->getParam("dateFrom")->value().toFloat(); dateTo = request->getParam("dateTo")->value().toFloat();
        seaFrom = request->getParam("seaFrom")->value().toFloat(); seaTo = request->getParam("seaTo")->value().toFloat();
#if AKA_CLOCK == true  
        request->getParam("isLedTHP")->value().toInt()==1?isLedTHP=true:isLedTHP=false;  
        thpFrom = request->getParam("thpFrom")->value().toFloat(); thpTo = request->getParam("thpTo")->value().toFloat();
#endif
        saveConfig(); 
        (isLedWeather?strWeather = GetWeather():strWeather ="");
        (isLedForecast?strWeatherFcast = GetWeatherForecast():strWeatherFcast = "");
        (isLedSea?strSea = GetSea():strSea ="");
        request->send(200, "text/html", "OK"); 
    }); 


    HTTP.onNotFound(notFound);
    HTTP.begin();
}


    

    