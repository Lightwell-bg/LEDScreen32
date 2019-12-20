#define SECS_PER_HOUR 60*60
const int daylightOffset_sec = 0*SECS_PER_HOUR;
String sNtpServerName2 = "pool.ntp.org";
String sNtpServerName3 = "time.nist.gov";

void timeSynch(void);

void init_Time() {
#if USE_RTC == true
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  } 
#endif 
  timeSynch();
}

time_t getNtpTime() {
  time_t t;  
  uint8_t count = 0;
  struct tm *timeinfo;
  if (WiFi.status() == WL_CONNECTED) {
      configTime(timezone*SECS_PER_HOUR, daylightOffset_sec, sNtpServerName.c_str(), sNtpServerName2.c_str(), sNtpServerName3.c_str()); // enable NTP
      Serial.println("\nWaiting for time");
      t = time(NULL);
      timeinfo = localtime(&t);
      while (timeinfo->tm_year < (2017 - 1900) && count < 10) {
          t = time(NULL);
          timeinfo = localtime(&t);
          Serial.print(".");
          Serial.print("t "); Serial.println(t);
          Serial.print("timeinfo->tm_year "); Serial.println(timeinfo->tm_year);
          count ++;
          delay(500);
      }
      if (timeinfo->tm_year > (2017 - 1900)) {
        Serial.println("Time NTP ready.");
        Serial.print("timeinfo: "); Serial.printf("%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min); Serial.println();
        return t;             
      }
      else return 0;
  }
  else return 0;
}

#if USE_RTC == true
void getRtcTime() {
    DateTime rtcnow;
    struct timeval now;
    time_t t;
    rtcnow = rtc.now();
    t = rtcnow.unixtime();// - timezone * SECS_PER_HOUR - daylightOffset_sec;; 
    now = { .tv_sec = t };
    settimeofday(&now, NULL);
}
#endif

#if USE_RTC == true  
void timeSynch() {
  time_t gotTime, tn;
  struct tm *tm;
  if (useRTC) {
    getRtcTime();
    Serial.println("ITime Ready RTS!");       
  }
  else {
    if (WiFi.status() == WL_CONNECTED) { //только если есть подключение
      gotTime = getNtpTime();
      if (gotTime > 0) { //Если время синхронизировано, устанавливаем RTC
        tn = time(NULL);
        tm = localtime(&tn);
        rtc.adjust(DateTime(tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec));  
      }
      else {
        statusUpdateNtpTime = 0;        
      }
    }
    else {
      getRtcTime();
      statusUpdateNtpTime = 0; 
    }
  }
  //lastNtpTime = millis();
}
#else 
void timeSynch() {
  time_t gotTime;
  if (WiFi.status() == WL_CONNECTED) { //только если есть подключение
      gotTime = getNtpTime();
      if (gotTime<=0) {
        statusUpdateNtpTime = 0;        
      } 
      else {
        Serial.println("timeSynch() Ready NTP!");
      }
  }
  else statusUpdateNtpTime = 0;
}
#endif
// Получение текущего времени
String GetTime(bool s) { //s - показывать секунды
    struct tm *tm;
    time_t tn = time(NULL);
    tm = localtime(&tn);
    String Time;
    if (s) {
      Time = String(tm->tm_hour)+":"+ (tm->tm_min < 10 ? "0"+String(tm->tm_min) : String(tm->tm_min)) +":"+ String(tm->tm_sec);  
    }
    else {
      Time = String(tm->tm_hour)+":"+ (tm->tm_min < 10 ? "0"+String(tm->tm_min) : String(tm->tm_min));
    }
    //Serial.println("GetTime() "+Time);
    return Time; // Возврашаем полученное время
}

void getTimeDisp(char *psz, bool f) { // Code for reading clock time for displey only
  struct tm *tm;
  time_t tn = time(NULL);
  tm = localtime(&tn);
  sprintf(psz, "%02d%c%02d", tm->tm_hour, (f ? ':' : ' '), tm->tm_min);
}

// Получение даты
String GetDate() {
    String Date;
    struct tm *tm;
    time_t tn = time(NULL);
    tm = localtime(&tn);
    Date = String(tm->tm_mday)+" "+month_table[lang][tm->tm_mon] +" "+String(tm->tm_year+1900) + ", " + day_table[lang][tm->tm_wday];
    //Serial.println(Date);
    Date.toLowerCase();
    if(Date.length() > 50) Date = ""; //for correct JsON when time is not set
    return Date; // Возврашаем полученную дату
}

bool compTimeInt(float tFrom, float tTo, float tNow) { //Сравнение времени. Чтобы правильно отображать от 18.00 до 8.00
  if (tFrom < tTo)  {
    if ((tFrom <= tNow) && (tTo >= tNow)) return true; else return false;
  }
  else if (tFrom > tTo) {
    if (tNow <= 23.59 && tFrom <= tNow)  return true;
    else if (tNow >= 0 && tTo >= tNow)  return true;
    else return false;  
  }
  else {
    return false;
  }
}

#if AKA_CLOCK == true
void workAlarms() {
  static bool checkStatus = 1; //don't set digitalWrite(BUZ_PIN, LOW) all time
  static unsigned long lastTimeAlarm;
  static bool onAlarm; 
  uint8_t bitOn = 0; 
  time_t tn = time(NULL); 
  struct tm* tm = localtime(&tn); 
  if (digitalRead(ALARM_PIN)) {
    //Serial.println("Button H click.");
    stopAlarm = true;
    timeStopAlarm = millis();
  }  
  if (stopAlarm && (millis()-timeStopAlarm > 1000*60)) { //через 1 min надо отключить кнопку остановки будильника
    stopAlarm = false;  
  }
  if (checkStatus && (stopAlarm || !((tm->tm_hour==myAlarm[0].alarm_h && tm->tm_min==myAlarm[0].alarm_m) || (tm->tm_hour==myAlarm[1].alarm_h && tm->tm_min==myAlarm[1].alarm_m)))) {
    digitalWrite(BUZ_PIN, LOW);
    checkStatus = 0;  
  } 
  bitOn = (day_byte[tm->tm_wday]) & myAlarm[0].alarm_stat;
  //Serial.print("bitOn "); Serial.println(bitOn);
  if ((bitOn > 0) && tm->tm_hour==myAlarm[0].alarm_h && tm->tm_min==myAlarm[0].alarm_m && !stopAlarm) {
    if (millis()-lastTimeAlarm > 500) {
      digitalWrite(BUZ_PIN, onAlarm);
      lastTimeAlarm = millis();
      onAlarm = !onAlarm;
    }
    checkStatus = 1;
  }  
  bitOn = (day_byte[tm->tm_wday]) & myAlarm[1].alarm_stat;    
  if ((bitOn > 0) && tm->tm_hour==myAlarm[1].alarm_h && tm->tm_min==myAlarm[1].alarm_m && !stopAlarm) {
    if (millis()-lastTimeAlarm > 1000) {
      digitalWrite(BUZ_PIN, onAlarm);
      lastTimeAlarm = millis();
      onAlarm = !onAlarm;
    }
    checkStatus = 1;
  }              
}
#endif