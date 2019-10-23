void init_Display(void) {
  P.begin();
  P.setIntensity(brightd);
  (fontUsed==0?P.setFont(_6bite_rus):fontUsed==1?P.setFont(_5bite_rus):fontUsed==2?P.setFont(_font_rus):P.setFont(_6bite_rus));
  P.setInvert(false);
  MP.begin();
}

void displayTime(bool lastShow) { //lastShow чтобы при посл показе был эффект
  static bool flasher = true;
  static uint32_t  lastDot = 0;
  if (!lastShow) {
    if (P.displayAnimate()) {  
      P.displayText(szTime, PA_CENTER, 75, 0, PA_PRINT, PA_NO_EFFECT);
      if (millis() - lastDot >= 750) {
        lastDot = millis();
        getTimeDisp(szTime, flasher);
        flasher = !flasher;
        if (displayClockFirst) {displayClockCount =millis(); displayClockFirst = false;}
        P.displayReset();     
      }  
    }
  }
  else { 
    P.displayText(szTime, PA_CENTER, 75, 0, PA_PRINT, PA_SCROLL_UP_RIGHT);        
  }
}

//Возвращаем true если текущая строка проигралась
bool showText(String sText, textPosition_t t_pos, uint16_t t_speed, uint16_t t_pause, textEffect_t  t_effectBegin, textEffect_t  t_effectEnd) {
  if  (P.displayAnimate()) {
    Serial.print("sText: "); Serial.println(sText);
    Serial.print("sText.length(): "); Serial.println(sText.length());
    if (sText.length() > 0) {
      sText.toCharArray(buf, 256);
      P.displayReset();
      P.displayText(buf, t_pos, t_speed, t_pause, t_effectBegin, t_effectEnd);
    }
    modeShow++;
    return true;
  }
  else {
    return false;
  }
}

uint8_t showTextUP(String sText, uint16_t t_speed, uint16_t t_pause, textEffect_t  t_effectBegin, textEffect_t  t_effectEnd, bool last) {
  if  (P.displayAnimate()) {
    Serial.print("sText: "); Serial.println(sText);
    Serial.print("sText.length(): "); Serial.println(sText.length());
    if (sText.length() > 0) {
      sText.toCharArray(buf, 256);
      P.displayReset();
      P.displayText(buf, PA_CENTER, t_speed, t_pause, t_effectBegin, t_effectEnd);
    }
    if (last) modeShow++;
    return 1;
  }
  return 0;
}  