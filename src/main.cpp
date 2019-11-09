#include <Arduino.h>

#include "config.h" 
#include "configFile.h"
#include "WIFI-SSDP.h"
#include "timeFunc.h" 
#include "firmWareUpd.h"
#include "Sensor.h"
#include "Weather.h"
#include "httpServer.h" 
#include "Display.h"
#include "MQTT.h"
#include "Thingspeak.h"

void setup() {
    Serial.begin(115200);
    char host[16];
#ifdef ESP32
    snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
#else
    snprintf(host, 12, "ESP%08X", ESP.getChipId());    
#endif  
    if(!SPIFFS.begin()) { // Initialize SPIFFS
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    } 
    loadConfig(); Serial.println(F("FileConfig init"));
    init_WIFI(); Serial.println(F("Wi-Fi init"));
    // if (WiFi.status() == WL_CONNECTED)  {
    //     init_SSDP(); Serial.println(F("SSDP init"));
    // }
    //MDNS.begin(host);
    //MDNS.addService("http", "tcp", 80);
    Serial.printf("Ready! Open http://%s.local in your browser\n", host); 
    init_Time(); Serial.println(F("Start Time"));  
    init_firmWareUpdate(); Serial.println(F("Start init FirmWare update"));
    //init_SSDP(); Serial.println(F("Start SSDP"));
    init_HTTPServer(); Serial.println(F("HTTPServer init"));
    init_Display(); Serial.println(F("Start screen"));
    if (WiFi.status() != WL_CONNECTED)  {
        String sText = logo + " " + WiFi.softAPIP().toString();
        sText.toCharArray(buf, 256);
        P.displayScroll(buf, PA_LEFT, PA_SCROLL_LEFT, 50);
        Serial.println(sText);
    }
    else {
        String sText = logo + " " +  WiFi.localIP().toString();
        sText.toCharArray(buf, 256);
        P.displayScroll(buf, PA_LEFT, PA_SCROLL_LEFT, 50);
        Serial.println(sText);
    }
    for (uint8_t i=0; i<ARRAY_SIZE(catalog); i++)  {
        //catalog[i].speed *= P.getSpeed();
        //catalog[i].speed *= speedTicker;
        catalog[i].pause *= 500;
    }
    init_mqtt(); Serial.println(F("Start MQTT"));
#if AKA_CLOCK == true
    init_sensor(); Serial.println(F("Start Sensor"));
    pinMode(ALARM_PIN, INPUT_PULLUP);
    pinMode(BUZ_PIN, OUTPUT);  
    digitalWrite(BUZ_PIN, LOW); 
#endif  
    while (!P.displayAnimate()) {
        delay(100);  
    }
    if (WiFi.status() == WL_CONNECTED && (isLedWeather || isLedForecast || isLedSea))  {
        if (isLedWeather) {strWeather = GetWeather(); delay(1000);}
        if (isLedForecast) {strWeatherFcast = GetWeatherForecast(); delay(1000);}
        if (isLedSea) strSea = GetSea();
    }
}

uint8_t oldModeShow = 0; //****************************
float oldnowtime = 111; //****************************

void loop() {
    if (statusUpdateNtpTime && (millis() - lastNtpTime >= TIME_UPDATE)) { //Обновление времени штатно
        lastNtpTime = millis();
        Serial.println("Update time");
        timeSynch();    
    }  
    if (!statusUpdateNtpTime && (millis() - lastNtpTime >= NTP_UPDATE)) { //При обновления с NTP при отсутствии ответа сервера пытаемся еще раз синхронизироваться
        lastNtpTime = millis();
        statusUpdateNtpTime = 1;
        Serial.println("Try sync NTP");
        timeSynch();    
    }  
#if AKA_CLOCK == true
    workAlarms();
    if(mqttOn && WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected() && (millis() - ReconnectTime > MQTT_CONNECT)) {
            reconnect();
            ReconnectTime = millis();
        }
        mqttClient.loop(); 
        if (millis() - mqttDhtTime > MQTT_SEND_INT) {
            sendMQTT();
            mqttDhtTime = millis();
            Serial.println("Send to MQTT");
        }
    }
    if(tspeakOn && WiFi.status() == WL_CONNECTED) {
        if (millis() - tspeakDhtTime > TSPEAK_SEND_INT) {
            sendTspeak();
            tspeakDhtTime = millis();
            Serial.println("Send to TSPEAK");
        }    
    }
#else
    if(mqttOn && WiFi.status() == WL_CONNECTED) {
        if (!mqttClient.connected() && (millis() - ReconnectTime > MQTT_CONNECT)) {
            reconnect();
            ReconnectTime = millis();
        }
        mqttClient.loop(); 
    }  
#endif
    time_t tn = time(NULL); 
    struct tm* tm = localtime(&tn); 
    float nowtime = tm->tm_hour+float(tm->tm_min)/100;
    if (oldnowtime != nowtime) {Serial.println("nowtime: "+ String(nowtime)); oldnowtime = nowtime;} //****************************
    if (compTimeInt(global_start, global_stop, nowtime)) { 
        if (modeShow > maxModeShow) modeShow = 1;
        if (oldModeShow != modeShow) {Serial.println("modeShow: "+ String(modeShow)); oldModeShow = modeShow; displayClockCount =millis(); displayClockFirst = true;} //****************************
        uint8_t rnd = random(0, ARRAY_SIZE(catalog));
        if (compTimeInt(dmodefrom, dmodeto, nowtime)) P.setIntensity(brightd); else P.setIntensity(brightn);
        if (modeShow == 1) {
            if (isLedClock && compTimeInt(clockFrom, clockTo, nowtime)) {
                //showText(GetTime(), PA_CENTER, catalog[rnd].speed*speedTicker, 5000, catalog[rnd].effect, catalog[rnd].effect);
                displayTime(false);
                if (millis()-displayClockCount >=6000) {
                    displayTime(true);
                    modeShow++;
                    P.displayReset();        
                }
            }
            else {
                modeShow++;  
            }
        }
    
        if (modeShow == 2) {
            //Serial.print("txtFrom0: ");Serial.print(txtFrom0);Serial.print(" nowtime: ");Serial.println(nowtime);
            if (isTxtOn0 && compTimeInt(txtFrom0, txtTo0, nowtime)) {
                if (isCrLine0) showText(strText0, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  
                else  showText(strText0, PA_CENTER, catalog[rnd].speed*speedTicker, catalog[rnd].pause*3, catalog[rnd].effect, catalog[rnd].effect);
            }
            else {
                modeShow++;
            }
        }
    
        if (modeShow == 3) {
            if (isLedDate && compTimeInt(dateFrom, dateTo, nowtime)) 
                showText(GetDate(), PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
            else  modeShow++;  
        } 
  
        if (modeShow == 4) {
            //Serial.print("txtFrom[0]: ");Serial.print(txtFrom[0]);Serial.print(" nowtime: ");Serial.println(nowtime);
            if (isTxtOn1 && compTimeInt(txtFrom1, txtTo1, nowtime)) {
                if (isCrLine1) showText(strText1, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  
                else showText(strText1, PA_CENTER, catalog[rnd].speed*speedTicker, catalog[rnd].pause, catalog[rnd].effect, catalog[rnd].effect);
            }
            else modeShow++;
        }  
  
        if (modeShow == 5) {
            if (isLedForecast && compTimeInt(fcastFrom, fcastTo, nowtime)) {
                if (millis() - lastTimeWeatherFcast > PER_GET_WEATHER_FCAST) { //чтобы не грузить сервер и часы
                strWeatherFcast = GetWeatherForecast();
                lastTimeWeatherFcast = millis();
                }    
            showText(strWeatherFcast, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
            }
            else   modeShow++;  
        }      
    
        if (modeShow == 6) {
            //Serial.print("txtFrom[0]: ");Serial.print(txtFrom[0]);Serial.print(" nowtime: ");Serial.println(nowtime);
            if (isTxtOn2 && compTimeInt(txtFrom2, txtTo2, nowtime)) {
                if (isCrLine2) showText(strText2, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  
                else   showText(strText2, PA_CENTER, catalog[rnd].speed*speedTicker, catalog[rnd].pause, catalog[rnd].effect, catalog[rnd].effect);
            }
            else modeShow++;
        } 

        if (modeShow == 7) {
            //Serial.print("txtFrom[0]: ");Serial.print(txtFrom[0]);Serial.print(" nowtime: ");Serial.println(nowtime);
            if (isTxtOn3 && compTimeInt(txtFrom3, txtTo3, nowtime)) {
                if (isCrLine3) showText(strText3, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  
                else  showText(strText3, PA_CENTER, catalog[rnd].speed*speedTicker, catalog[rnd].pause, catalog[rnd].effect, catalog[rnd].effect);
            }
            else       modeShow++;
        }   
        if (modeShow == 8) {
            if (isLedWeather && compTimeInt(weathFrom, weathTo, nowtime)) {
                if (millis() - lastTimeWeather > PER_GET_WEATHER) { //чтобы не грузить сервер и часы
                    //Serial.print(" PER_GET_WEATHER: "); Serial.println(PER_GET_WEATHER);
                    strWeather = GetWeather();
                    lastTimeWeather = millis();
                }    
                showText(strWeather, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
            }
            else  modeShow++;  
        }

        if (modeShow == 9) { //String from MQTT
            showText(crLine, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        }

        if (modeShow == 10) { //SEA Temp
            if (isLedSea && compTimeInt(seaFrom, seaTo, nowtime)) {
                if (millis() - lastTimeSea > PER_GET_SEA) { //чтобы не грузить сервер и часы
                    //Serial.print(" PER_GET_WEATHER: ");Serial.println(PER_GET_WEATHER);
                    strSea = GetSea();
                    lastTimeSea = millis();
                }    
                showText(strSea, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
            }
            else  modeShow++;  
        }            
#if AKA_CLOCK == true
        if (modeShow == 11) { //Temp, hum, press
            static uint8_t modeShowUP = 1;
            static String tempUP, humUp, pressUP;
            if (isLedTHP&& compTimeInt(thpFrom, thpTo, nowtime)) {
                if (millis() - lastTimePHT > PER_GET_THP) { //чтобы не грузить
    #if USE_DHT == true 
                    strTHP = onboard[lang] + getTempDHT() + hum[lang] + getHumDHT() + "%";
                    tempUP = getTempDHT()+"\xC3"; humUp = getHumDHT()+"%";
    #endif   
    #if USE_BME280 == true
                    //strTHP = onboard[lang] + getTempBME280() + hum[lang] + getHumBME280() + pres[lang] + getPressBME280() + "mm";
                    tempUP = getTempBME280()+"\xC3"; humUp = getHumBME280()+"%"; pressUP = getPressBME280()+"\xC4";
    #endif  
                    Serial.println(strTHP);
                    lastTimePHT = millis();
                } 
    #if USE_DHT == true 
                if (modeShowUP > 4) modeShowUP = 1;
                if (modeShowUP==1) modeShowUP +=showTextUP(temperIn[lang], 5*speedTicker, 500, PA_SCROLL_UP, PA_SCROLL_UP, false); 
                if (modeShowUP==2) modeShowUP +=showTextUP(tempUP, 5*speedTicker, 1750, PA_SCROLL_UP, PA_SCROLL_UP_LEFT, false); 
                if (modeShowUP==3) modeShowUP +=showTextUP(humIn[lang], 5*speedTicker, 500, PA_SCROLL_UP, PA_SCROLL_UP, false);
                if (modeShowUP==4) modeShowUP +=showTextUP(humUp, 5*speedTicker, 1750, PA_SCROLL_UP, PA_SCROLL_UP_RIGHT, true);         
                //showText(strTHP, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    #endif 
    #if USE_BME280 == true 
                if (modeShowUP > 6) modeShowUP = 1;
                if (modeShowUP==1) modeShowUP +=showTextUP(temperIn[lang], 5*speedTicker, 500, PA_SCROLL_UP, PA_SCROLL_UP, false); 
                if (modeShowUP==2) modeShowUP +=showTextUP(tempUP, 5*speedTicker, 1750, PA_SCROLL_UP, PA_SCROLL_UP_LEFT, false); 
                if (modeShowUP==3) modeShowUP +=showTextUP(humIn[lang], 5*speedTicker, 500, PA_SCROLL_UP, PA_SCROLL_UP, false);
                if (modeShowUP==4) modeShowUP +=showTextUP(humUp, 5*speedTicker, 1750, PA_SCROLL_UP, PA_SCROLL_UP_RIGHT, false); 
                if (modeShowUP==5) modeShowUP +=showTextUP(presIn[lang], 5*speedTicker, 500, PA_SCROLL_UP, PA_SCROLL_UP, false);
                if (modeShowUP==6) modeShowUP +=showTextUP(pressUP, 5*speedTicker, 1750, PA_SCROLL_UP, PA_SCROLL_UP_RIGHT, true);                 
                //showText(strTHP, PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    #endif           
            }
            else  modeShow++;  
        }
#endif    
    }  // END OF if (compTimeInt(global_start, global_stop, nowtime))
    else {
        //if (timeStatus() != timeNotSet && (timeStatus() != timeNeedsSync)) { //Не показывает ничего если не может получить время. Это для индикации что нет интернета, а должен быть
        if (nowtime != 0) {
            P.displayClear();       
        }
        else {
            showText(F("You use NTP server but no Wi-Fi or Internet"), PA_LEFT, 5*speedTicker, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT); 
            useRTC = true; 
        }
    }
    delay(1); //Вроде нужна для восстановления стека
} //LOOP