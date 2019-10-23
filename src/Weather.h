String GetWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    return "No connection to weather server (No Wi-Fi)";  
  }  
  Serial.print("connecting to "); Serial.println(W_URL);
  String wStrURL = String("GET /data/2.5/weather?id=") + CITY_ID; 
  wStrURL += "&units=metric&appid=" + W_API;
  switch (lang) {
    case 0:
      wStrURL += "&lang=ru";
    break;
    case 1:
      wStrURL += "&lang=bg";
    break;
    case 2:
      wStrURL += "&lang=en";
    break;     
    default:
      wStrURL += "&lang=en";
    break;
  }  
  wStrURL += "\r\nHost: " + W_URL + "\r\nUser-Agent: ArduinoWiFi/1.1\r\nConnection: close\r\n\r\n";
  Serial.println(wStrURL);
  /*if (ESPclient.connect(W_URL.c_str(), 80)) {
    ESPclient.println(String("GET /data/2.5/weather?id=") + CITY_ID + "&units=metric&appid=" + W_API + "&lang="+ weatherLang + "\r\n" +
                "Host: " + W_URL + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                "Connection: close\r\n\r\n");
  }  */
  if (ESPclient.connect(W_URL.c_str(), 80)) {
    ESPclient.println(wStrURL);
  }
  else {
    Serial.println("connection weather server failed");
    //return "Weather server connection failed";
    return "";
  }
  String line;
  uint8_t repeatCounter = 0;
  while (!ESPclient.available() && repeatCounter < 10) {
    //Serial.print("ESPclient.available() "); Serial.println(ESPclient.available());
    delay(500);
    Serial.println("w.");
    repeatCounter++;
  }
  //Serial.print("ESPclient.available() "); Serial.println(ESPclient.available());Serial.print("ESPclient.connected() "); Serial.println(ESPclient.connected());
  //while (ESPclient.connected() && ESPclient.available()) {
  while (ESPclient.available()) {  
    char c = ESPclient.read(); 
    if (c == '[' || c == ']') c = ' ';
    line += c;
  }
  ESPclient.stop();
  Serial.println(line);
  DynamicJsonDocument jsonDoc(1024);
  DeserializationError error =  deserializeJson(jsonDoc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());    
    Serial.println("Json parsing failed!");
    //return "Weather error (Json parsing failed!)";
    return "";
  }
  JsonObject root = jsonDoc.as<JsonObject>();
  String weatherString;
  weatherString = overboard[lang];
  weatherString += root["weather"]["description"].as<String>();
  //weatherDescription.toLowerCase();
  //  weatherLocation = root["name"].as<String>();
  //  country = root["sys"]["country"].as<String>();
  weatherString += temper[lang];
  float temp = root["main"]["temp"]; //.as<float>();
  weatherString += String(temp,1);
  uint8_t humidity = root["main"]["humidity"]; //.as<String>();
  uint16_t pressure = root["main"]["pressure"];
     //double p = root["main"]["pressure"].as<double>()/1.33322;
     //tape += String((int)p);  
  weatherString += hum[lang];  weatherString += String(humidity);
  weatherString += pres[lang]; weatherString += String(pressure/1.3332239,0);
  weatherString += wind[lang];
  weatherString += root["wind"]["speed"].as<String>();
  weatherString += windsp[lang];    
  double deg = root["wind"]["deg"];
  if (deg >22.5 && deg <=67.5) weatherString += windir_table[lang][0];
  else if (deg >67.5 && deg <=112.5) weatherString += windir_table[lang][1];
  else if (deg >112.5 && deg <=157.5) weatherString += windir_table[lang][2];
  else if (deg >157.5 && deg <=202.5) weatherString += windir_table[lang][3];
  else if (deg >202.5 && deg <=247.5) weatherString += windir_table[lang][4];
  else if (deg >247.5 && deg <=292.5) weatherString += windir_table[lang][5];
  else if (deg >292.5 && deg <=337.5) weatherString += windir_table[lang][6];
  else weatherString += windir_table[lang][7];
  uint8_t clouds = root["clouds"]["all"];
  weatherString += cloudstxt[lang] + String(clouds) + "% ";
  Serial.println(weatherString);  
  return weatherString;
}

String GetWeatherForecast() {
  if (WiFi.status() != WL_CONNECTED) {
    return "No connection to weather server (No Wi-Fi)";  
  }  
  Serial.print("connecting to "); Serial.println(W_URL);
  //String wStrURL = String("GET /data/2.5/forecast/daily?id=") + CITY_ID; 
  String wStrURL = String("GET /data/2.5/forecast?id=") + CITY_ID; 
  wStrURL += "&units=metric&appid=" + W_API;
  switch (lang) {
    case 0:
      wStrURL += "&lang=ru";
    break;
    case 1:
      wStrURL += "&lang=bg";
    break;
    case 2:
      wStrURL += "&lang=en";
    break;     
    default:
      wStrURL += "&lang=en";
    break;
  }  
  wStrURL += "&cnt=1";
  wStrURL += "\r\nHost: " + W_URL + "\r\nUser-Agent: ArduinoWiFi/1.1\r\nConnection: close\r\n\r\n";
  Serial.println(wStrURL);
  /*if (ESPclient.connect(W_URL.c_str(), 80)) {
    ESPclient.println(String("GET /data/2.5/weather?id=") + CITY_ID + "&units=metric&appid=" + W_API + "&lang="+ weatherLang + "\r\n" +
                "Host: " + W_URL + "\r\nUser-Agent: ArduinoWiFi/1.1\r\n" +
                "Connection: close\r\n\r\n");
  }  */
  if (ESPclient.connect(W_URL.c_str(), 80)) {
    ESPclient.println(wStrURL);
  }
  else {
    Serial.println("Weather server connection failed (forecast)");
    //return "Weather server connection failed (forecast)";
    return "";
  }
  String line;
  uint8_t repeatCounter = 0;
  while (!ESPclient.available() && repeatCounter < 10) {
    //Serial.print("ESPclient.available() "); Serial.println(ESPclient.available());
    delay(500);
    Serial.println("w.");
    repeatCounter++;
  }
  //Serial.print("ESPclient.available() "); Serial.println(ESPclient.available());Serial.print("ESPclient.connected() "); Serial.println(ESPclient.connected());
  //while (ESPclient.connected() && ESPclient.available()) {
  //bool startJson = false;
  while (ESPclient.available()) {  
    char c = ESPclient.read(); 
    if (c == '[' || c == ']') c = ' ';    
    line += c;
  }
  line += "\0";
  ESPclient.stop();
  Serial.println(line);
  DynamicJsonDocument jsonDoc(1024);
  DeserializationError error =  deserializeJson(jsonDoc, line);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());    
    Serial.println("Json parsing forecast failed!");
    //return "Weather error (Json parsing forecast failed!)";
    return "";
  }
  JsonObject root = jsonDoc.as<JsonObject>();
  String weatherString;
  weatherString = forecast[lang];
  //weatherString += root["city"]["name"].as<String>() + " ";
  weatherString += root["list"]["weather"]["description"].as<String>();
  float tempMin = root["list"]["main"]["temp_min"];
  float tempMax = root["list"]["main"]["temp_max"];  
  uint8_t humidity = root["list"]["main"]["humidity"];
  double pressure = root["list"]["main"]["pressure"].as<double>()/1.3332239;
  weatherString += tempermin[lang];
  weatherString += String(tempMin,1);
  weatherString += tempermax[lang];
  weatherString += String(tempMax,1);
  weatherString += hum[lang];  weatherString += String(humidity);
  weatherString += pres[lang]; weatherString += String(pressure,0);
  weatherString += wind[lang];
  double deg = root["list"]["wind"]["deg"];
  if (deg >22.5 && deg <=67.5) weatherString += windir_table[lang][0];
  else if (deg >67.5 && deg <=112.5) weatherString += windir_table[lang][1];
  else if (deg >112.5 && deg <=157.5) weatherString += windir_table[lang][2];
  else if (deg >157.5 && deg <=202.5) weatherString += windir_table[lang][3];
  else if (deg >202.5 && deg <=247.5) weatherString += windir_table[lang][4];
  else if (deg >247.5 && deg <=292.5) weatherString += windir_table[lang][5];
  else if (deg >292.5 && deg <=337.5) weatherString += windir_table[lang][6];
  else weatherString += windir_table[lang][7];
  weatherString += root["list"]["wind"]["speed"].as<String>();
  weatherString += windsp[lang];  
  Serial.println(weatherString);  
  return weatherString;  
}

String WSea_URL = "worldseatemp.com";
uint8_t SEA_ID = 59;
#ifdef ESP32
HTTPClient http;
String GetSea() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No connection to weather server (No Wi-Fi)");
    return "";  
  }  
  Serial.print("connecting to "); Serial.println(WSea_URL);
  String wStrURL = String("http://")+WSea_URL + String("/en/i/") + String(SEA_ID); 
  wStrURL += "/c/160x86/";
  Serial.println(wStrURL);  
  http.begin(wStrURL);
  int httpCode = http.GET();
  String response;
  if (httpCode > 0) { //Check for the returning code
        response = http.getString();
        Serial.println(httpCode);
        //Serial.println(response);
  }
  else {
      Serial.println("Error on HTTP request");
  }
  http.end(); //Free the resources
  uint16_t firstInt = response.indexOf("temp\">");
  uint16_t secondInt = response.indexOf("&deg",firstInt);
  String tempSea = response.substring(firstInt+6, secondInt);
  if (tempSea!="") return  String(tempersea[lang] + tempSea +"\xC2С");
  else return "";
}
#else
String GetSea() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No connection to weather server (No Wi-Fi)");
    return "";  
  }  
  Serial.print("connecting to "); Serial.println(WSea_URL);
  String wStrURL = String("GET /en/i/") + String(SEA_ID); 
  wStrURL += "/c/160x86/ HTTP/1.1";

  //wStrURL += "\r\nHost: " + WSea_URL + "\r\nUser-Agent: ArduinoWiFi/1.1\r\nConnection: close\r\n\r\n";
  wStrURL += "\r\nHost: " + WSea_URL + "\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.3; WOW64; rv:36.0) Gecko/20100101 Firefox/36.0\r\nConnection: close\r\n\r\n";
  Serial.println(wStrURL);  
  if (ESPclient.connect(WSea_URL.c_str(), 80)) {
      ESPclient.println(wStrURL);
  }
  else {
      Serial.println("Weather server connection failed");
      return "";
  }
  String response = "";
  unsigned startTime = millis();
  delay (2000);
  while (!ESPclient.available() && ((millis()-startTime) < 5000)) {
      Serial.print("w.");
      delay (500);
  }
  Serial.println();
  
  while (ESPclient.available()) { // Get response from server
     char charIn = ESPclient.read(); // Read a char from the buffer.
     response += charIn;     // Append the char to the string response.
  }
  ESPclient.stop();
  uint16_t firstInt = response.indexOf("temp\">");
  uint16_t secondInt = response.indexOf("&deg",firstInt);
  String tempSea = response.substring(firstInt+6, secondInt);
  if (tempSea!="") return  String(tempersea[lang] + tempSea +"\xC2С"); else return "";
}
#endif