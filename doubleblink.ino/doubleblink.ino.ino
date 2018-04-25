/*
 * @Auth Prasad
 * @Date 15/12/2017
 */
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <RtcDS3231.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Arduino.h>
#include <Scheduler.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <TimeScheduler.h>
ClientExecuter clientExecuter;
void setup() {
  pinMode(BUILTIN_LED2, OUTPUT); // Initialize the BUILTIN_LED2 pin as an output
  /*RELY*/
  pinMode(RELAY_OUTPUT1,OUTPUT);
  pinMode(RELAY_OUTPUT2,OUTPUT);
  pinMode(RELAY_OUTPUT3,OUTPUT);
  pinMode(RELAY_OUTPUT4,OUTPUT);
  
  digitalWrite(RELAY_OUTPUT1,LOW);
  digitalWrite(RELAY_OUTPUT2,LOW); 
  digitalWrite(RELAY_OUTPUT3,LOW);  
  digitalWrite(RELAY_OUTPUT4,LOW); 
  /*LED*/
  digitalWrite(BUILTIN_LED2,LOW);
  Serial.begin(115200);
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  Rtc.Begin();
  /*RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  time_rest_on(compiled);*/
  lcd_setup();
  loadExternalConfig();
  WifiSetup();
  relayScheduler.setData(loadSchedulers());
  Scheduler.start(&clientExecuter);
  Scheduler.start(&relayScheduler);
  Scheduler.begin();
  
}
void loop() {
  /*server.handleClient();
  if(!Rtc.IsDateTimeValid()){Serial.println("RTC lost confidence in the DateTime!");}
  RtcDateTime rtc_now = Rtc.GetDateTime();
  printDateTime(rtc_now);*/
}
