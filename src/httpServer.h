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
        else if(request->getParam("lang")->value() == "BG" && lang !=1) lang = 1; 
        else if(request->getParam("lang")->value() == "EN" && lang !=2) lang = 2; 
        else {request->send(200, "text/html", "OK"); return;} 
        Serial.print("Set lang: ");    Serial.println(request->getParam("lang")->value());
        saveConfig();
        if (isLedWeather || isLedForecast || isLedSea) {
            if (isLedWeather) {strWeather = GetWeather(); delay(1000);}
            if (isLedForecast) {strWeatherFcast = GetWeatherForecast(); delay(1000);}
            if (isLedSea) strSea = GetSea();
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

    HTTP.on("/texts", HTTP_GET, [](AsyncWebServerRequest *request){
        P.displaySuspend(true);
        strText0 = request->getParam("ledText0")->value(); 
        strText1 = request->getParam("ledText1")->value(); 
        strText2 = request->getParam("ledText2")->value(); 
        strText3 = request->getParam("ledText3")->value(); 
        request->getParam("isTxtOn0")->value().toInt()==1?isTxtOn0=true:isTxtOn0=false;
        request->getParam("isTxtOn1")->value().toInt()==1?isTxtOn1=true:isTxtOn1=false;
        request->getParam("isTxtOn2")->value().toInt()==1?isTxtOn2=true:isTxtOn2=false;
        request->getParam("isTxtOn3")->value().toInt()==1?isTxtOn3=true:isTxtOn3=false;
        txtFrom0 = request->getParam("txtFrom0")->value().toFloat(); txtTo0 = request->getParam("txtTo0")->value().toFloat();
        txtFrom1 = request->getParam("txtFrom1")->value().toFloat(); txtTo1 = request->getParam("txtTo1")->value().toFloat();
        txtFrom2 = request->getParam("txtFrom2")->value().toFloat(); txtTo2 = request->getParam("txtTo2")->value().toFloat();
        txtFrom3 = request->getParam("txtFrom3")->value().toFloat(); txtTo3 = request->getParam("txtTo3")->value().toFloat();
        request->getParam("isCrLine0")->value().toInt()==1?isCrLine0=true:isCrLine0=false;
        request->getParam("isCrLine1")->value().toInt()==1?isCrLine1=true:isCrLine1=false;
        request->getParam("isCrLine2")->value().toInt()==1?isCrLine2=true:isCrLine2=false;
        request->getParam("isCrLine3")->value().toInt()==1?isCrLine3=true:isCrLine3=false;
        global_start = request->getParam("global_start")->value().toFloat();
        global_stop = request->getParam("global_stop")->value().toFloat();
        saveConfig();                 // Функция сохранения данных во Flash
        Serial.print("strText ");Serial.print(strText0);Serial.println(strText1);Serial.println(strText2);Serial.println(strText3);
        Serial.print("isTxtOn0 ");Serial.print(isTxtOn0);Serial.println(txtFrom0);Serial.println(txtTo0);Serial.println(isCrLine0);
        Serial.print("isTxtOn1 ");Serial.print(isTxtOn1);Serial.println(txtFrom1);Serial.println(txtTo1);Serial.println(isCrLine1);
        Serial.print("isTxtOn2 ");Serial.print(isTxtOn2);Serial.println(txtFrom2);Serial.println(txtTo2);Serial.println(isCrLine2);
        Serial.print("isTxtOn3 ");Serial.print(isTxtOn3);Serial.println(txtFrom3);Serial.println(txtTo3);Serial.println(isCrLine3);
        request->send(200, "text/html", "OK"); 
        P.displaySuspend(false);
    }); 

    HTTP.on("/ledsetup", HTTP_GET, [](AsyncWebServerRequest *request){
        speedTicker = 28 - request->getParam("speed_d")->value().toInt(); 
        brightd = request->getParam("brightd")->value().toInt();
        brightn = request->getParam("brightn")->value().toInt();
        dmodefrom = request->getParam("dmodefrom")->value().toInt();
        dmodeto = request->getParam("dmodeto")->value().toInt();
        fontUsed = request->getParam("fontUsed")->value().toInt();
        (fontUsed==0?P.setFont(_6bite_rus):fontUsed==1?P.setFont(_5bite_rus):fontUsed==2?P.setFont(_font_rus):P.setFont(_6bite_rus));
        saveConfig();
        P.setIntensity(brightd);
        Serial.print("speedTicker ");Serial.println(speedTicker); Serial.print("brightd ");Serial.println(brightd);Serial.print("brightn ");Serial.println(brightn);
        Serial.print("dmodefrom ");Serial.println(dmodefrom);Serial.print("dmodeto ");Serial.println(dmodeto);
        request->send(200, "text/html", "OK"); 
    });

    HTTP.on("/weather", HTTP_GET, [](AsyncWebServerRequest *request){
        W_URL = request->getParam("weatherHost")->value().c_str();
        CITY_ID = request->getParam("city_code")->value().c_str(); 
        W_API = request->getParam("w_api")->value().c_str();  
        saveConfig();                 
        strWeather = GetWeather(); delay(1000); strWeatherFcast = GetWeatherForecast();
        Serial.println("W_URL: " + W_URL + ", CITY_ID: " + CITY_ID + ", W_API: " + W_API);
        request->send(200, "text/html", "OK"); 
    });    

    HTTP.on("/weatherUpdate", HTTP_GET, [](AsyncWebServerRequest *request){
        if(request->getParam("update")->value() == "ok") {
            strWeather = GetWeather();
            delay(1000);
            strWeatherFcast = GetWeatherForecast();
        }
        request->send(200, "text/html", "OK"); 
    });   
#if AKA_CLOCK == true
    HTTP.on("/mqttSet", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("mqttOn")->value().toInt()==1?mqttOn=true:mqttOn=false;
        mqtt_server = request->getParam("mqtt_server")->value().c_str();
        mqtt_port = request->getParam("mqtt_port")->value().toInt();
        mqtt_user = request->getParam("mqtt_user")->value().c_str();
        mqtt_pass = request->getParam("mqtt_pass")->value().c_str();
        mqtt_name = request->getParam("mqtt_name")->value().c_str();
        mqtt_sub_crline = request->getParam("mqtt_sub_crline")->value().c_str();
        mqtt_pub_temp = request->getParam("mqtt_pub_temp")->value().c_str();
        mqtt_pub_hum = request->getParam("mqtt_pub_hum")->value().c_str();
        mqtt_pub_press = request->getParam("mqtt_pub_press")->value().c_str();
        saveConfig();  
        Serial.println("mqtt_server: " + mqtt_server + ", mqtt_user: " + mqtt_user + ", mqtt_name: " + mqtt_name);
        request->send(200, "text/html", "OK"); 
    }); 

    HTTP.on("/tspeakSet", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("tspeakOn")->value().toInt()==1?tspeakOn=true:tspeakOn=false;
        tspeak_server = request->getParam("tspeak_server")->value().c_str();
        tspeak_channal = request->getParam("tspeak_channal")->value().toInt();
        tspeak_wapi = request->getParam("tspeak_wapi")->value().c_str();
        saveConfig();  
        Serial.println("tspeak_server: " + tspeak_server + ", tspeak_channal: " + tspeak_channal + ", tspeak_wapi: " + tspeak_wapi);
        request->send(200, "text/html", "OK"); 
    });

    HTTP.on("/tspeakOn", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("tspeakOn")->value().toInt()==1?tspeakOn=true:tspeakOn=false;
        saveConfig();   
        Serial.println("tspeakOn: " + String(tspeakOn));
        request->send(200, "text/html", "OK"); 
    });   

    HTTP.on("/Sensor", HTTP_GET, [](AsyncWebServerRequest *request){
    #if USE_DHT == true 
        strTHP = onboard[lang] + getTempDHT() + hum[lang] + getHumDHT() + "%";
    #endif   
    #if USE_BME280 == true
        strTHP = onboard[lang] + getTempBME280() + hum[lang] + getHumBME280() + pres[lang] + getPressBME280() + "mm";
    #endif  
        Serial.println(strTHP);
        lastTimePHT = millis();    
        request->send(200, "text/html", "OK"); 
    });

    HTTP.on("/setalarm", HTTP_GET, [](AsyncWebServerRequest *request){
        myAlarm[0].alarm_h = request->getParam("alarm1_h")->value().toInt();
        myAlarm[0].alarm_m = request->getParam("alarm1_m")->value().toInt(); 
        myAlarm[0].alarm_stat = request->getParam("alarm1_stat")->value().toInt();  
        myAlarm[1].alarm_h = request->getParam("alarm2_h")->value().toInt();
        myAlarm[1].alarm_m = request->getParam("alarm2_m")->value().toInt(); 
        myAlarm[1].alarm_stat = request->getParam("alarm2_stat")->value().toInt();     
        saveConfig();
        Serial.println("alarm1_h: " + String(myAlarm[0].alarm_h) + ", alarm1_m: " + String(myAlarm[0].alarm_m) + ", alarm1_stat: " + String(myAlarm[0].alarm_stat));
        Serial.println("alarm2_h: " + String(myAlarm[1].alarm_h) + ", alarm2_m: " + String(myAlarm[1].alarm_m) + ", alarm2_stat: " + String(myAlarm[1].alarm_stat));
        request->send(200, "text/html", "OK"); 
    });      
#else
    HTTP.on("/mqttSet", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("mqttOn")->value().toInt()==1?mqttOn=true:mqttOn=false;
        mqtt_server = request->getParam("mqtt_server")->value().c_str();
        mqtt_port = request->getParam("mqtt_port")->value().toInt();
        mqtt_user = request->getParam("mqtt_user")->value().c_str();
        mqtt_pass = request->getParam("mqtt_pass")->value().c_str();
        mqtt_name = request->getParam("mqtt_name")->value().c_str();
        mqtt_sub_crline = request->getParam("mqtt_sub_crline")->value().c_str();
        saveConfig();                 
        Serial.println("mqtt_server: " + mqtt_server + ", mqtt_user: " + mqtt_user + ", mqtt_name: " + mqtt_name);
        request->send(200, "text/html", "OK"); 
    }); 
#endif  

    HTTP.on("/mqttOn", HTTP_GET, [](AsyncWebServerRequest *request){
        request->getParam("mqttOn")->value().toInt()==1?mqttOn=true:mqttOn=false;
        saveConfig();   
        Serial.println("mqttOn: " + String(mqttOn));
        request->send(200, "text/html", "OK"); 
    });   

    HTTP.onNotFound(notFound);
    HTTP.begin();
}


    

    