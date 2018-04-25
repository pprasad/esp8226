#ifndef TIMESCHEDULER_H
#define TIMESCHEDULER_H

#define DS3231_I2C_ADDRESS 0x68
#define RtcSquareWavePin 19
#define RtcSquareWaveInterrupt 4 
#define countof(a) (sizeof(a) / sizeof(a[0]))
DynamicJsonBuffer jsonBuffer;//(40000)
/*Constant Declarations*/
const int BUILTIN_LED1 = 2; //GPIO2
const int BUILTIN_LED2 = 16;//GPIO16
/*Reply Board*/
const int RELAY_OUTPUT1=14;
const int RELAY_OUTPUT2=12;
const int RELAY_OUTPUT3=13;
const int RELAY_OUTPUT4=15;
/*WIfi Connection Details*/
//const char* SSID_NAME="LathaHome";
//const char* PASSWORD="latha&123456";
const char* SSID_NAME="esp8266";
const char* PASSWORD="esp8266123";
const char* FILE_NAME="/esp8266.json";
volatile int seconds = 0;
/*IP Configurarion*/
IPAddress ip(192, 168,0,2);
IPAddress gateway(192, 168,0, 1);
IPAddress subnet(255, 255, 255, 0);
/*Time Configuration*/
RtcDS3231<TwoWire> Rtc(Wire);
ESP8266WebServer server(80); //Initialize the server on Port 80
LiquidCrystal_I2C lcd(0x3F,16,2);
String getConfig(String property,int deviceId){
   Serial.println("********Request Property***************");
   Serial.println(property);
    Serial.println("********Device Id***************");
   Serial.println(deviceId); 
   String response="null";
   File configFile = SPIFFS.open(FILE_NAME,"r");
   if(configFile){
     size_t size = configFile.size();
     Serial.println("********************************File Size**************************");
     Serial.println(size);
     std::unique_ptr<char[]> buf (new char[size]);
     configFile.readBytes(buf.get(), size);
     JsonObject& root = jsonBuffer.parseObject(buf.get());
     Serial.println("***********Root Information***************");
     root.prettyPrintTo(Serial);
     if(root.success()){
           Serial.println("************Fetching Configurtion Details**************");
           root.prettyPrintTo(Serial);
           if(root.containsKey("devices") && property=="devices"){
                response="";
                JsonArray& devices=root["devices"];
                devices.prettyPrintTo(Serial);
                devices.prettyPrintTo(response);
           }else if(root.containsKey("scheduler") && property=="scheduler"){
                response=""; 
                JsonArray& filter=jsonBuffer.createArray();
                JsonArray& schedulers=root["scheduler"];
                for(JsonArray::iterator it=schedulers.begin(); it!=schedulers.end(); ++it) {
                   JsonObject& scheduler=it->as<JsonObject>();
                   if(scheduler["deviceId"]==deviceId){
                       filter.add(scheduler);
                   }
                }
                filter.prettyPrintTo(response);
           }
      }
   }else{
       Serial.println("***********Failure Fetching in Configuration Details******************");
   }
   configFile.close();
   return response;
}
bool save(JsonObject& json){
  File configFile = SPIFFS.open(FILE_NAME, "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  json.printTo(configFile);
  configFile.close();
  return true;
}
/*create configuration info into config.json file*/
bool createDevice(String nodeName,JsonObject& root,JsonObject& json){
    Serial.println("**** Enter into CreateDevice Method*******");
    json.prettyPrintTo(Serial);
    JsonArray& jsonDevices=root.createNestedArray(nodeName);
    jsonDevices.add(json);
    jsonDevices.prettyPrintTo(Serial);
    return true;
}
/*create or update configuration info into config.json file*/
bool updateDevice(File configFile,JsonObject& json){
      bool flag=false;
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf (new char[size]);
      configFile.readBytes(buf.get(), size);
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      if(!root.success()) {
        Serial.println("Failed to parse config file");
        return false;
      }else{
        if(root.containsKey("devices")){
              Serial.println("\n Data Exist");
              bool flag=false;
              JsonArray& devices=root["devices"];
              int index=0;
              for(JsonArray::iterator it=devices.begin(); it!=devices.end(); ++it) {
                   JsonObject& device=it->as<JsonObject>();
                   if(device["deviceMode"]==json["deviceMode"]){
                       devices.set(index,json);
                       flag=true;
                       break;     
                   }
                   index=index+1;
              }
              if(!flag){
                  devices.add(json);
              }
              root.printTo(Serial);
        }else{
            if(createDevice("devices",root,json)){
                root.printTo(Serial);
            }
        }
    }
    flag=save(root);
    return flag;
}
/*create or update scheduler*/
bool updateScheduler(File configFile,JsonObject& json){
      bool flag=false;
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf (new char[size]);
      configFile.readBytes(buf.get(), size);
      JsonObject& root = jsonBuffer.parseObject(buf.get());
      if(!root.success()) {
        Serial.println("Failed to parse config file");
        return false;
      }else{
        if(root.containsKey("scheduler")){
              Serial.println("\n Data Exist");
              bool flag=false;
              JsonArray& schedulers=root["scheduler"];
              int index=0;
              for(JsonArray::iterator it=schedulers.begin(); it!=schedulers.end(); ++it) {
                   JsonObject& scheduler=it->as<JsonObject>();
                   if(scheduler["id"]==json["id"]){
                       schedulers.set(index,json);
                       flag=true;
                       break;     
                   }
                   index=index+1;
              }
              if(!flag){
                  schedulers.add(json);
              }
              root.printTo(Serial);
        }else{
            if(createDevice("scheduler",root,json)){
                root.printTo(Serial);
            }
        }
    }
    flag=save(root);
    return flag;
}
/*Save new configuration info into config.json file*/
bool saveConfig(JsonObject& json,String property) {
  File configFile;
  size_t size=0;
  Serial.println("**** Add New Device*******");
  json.prettyPrintTo(Serial);
  bool flag=false;
  bool isExist = SPIFFS.exists(FILE_NAME);
  if(isExist){
        Serial.println("\n***Reading Configuration file*******");
        configFile = SPIFFS.open(FILE_NAME,"r");
        Serial.println(configFile);
        if(!configFile) {
            Serial.println("Failed to open config file for writing");
            return false;
        }
        Serial.println("\n***Completed Reading file*******");
       size = configFile.size();
  }
  Serial.println("File Size ");
  Serial.println(size);
  if(size==0){
     Serial.println("File is Empty");
     JsonObject& root=jsonBuffer.createObject();
     if(createDevice(property,root,json)){
         root.printTo(Serial);
         flag=save(root);
     }
  }else if(property=="scheduler"){
      flag=updateScheduler(configFile,json);
  }else{
     flag=updateDevice(configFile,json);
  }
  configFile.close();
  return flag;
}
void loadExternalConfig(){
   if(!SPIFFS.begin()) {
       Serial.println("Failed to mount file system");
       return;
  }
}
/*Custom Scheduler classes*/
class ClientExecuter:public Task{
  public:
     void static printDateTime(const RtcDateTime& dt)
    {
            char datestring[20];
            snprintf_P(datestring, 
                    countof(datestring),
                    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
                    dt.Month(),
                    dt.Day(),
                    dt.Year(),
                    dt.Hour(),
                    dt.Minute(),
                    dt.Second() );
            Serial.print(datestring);
    }  
    static String sysTime(){
        char datestring[20];
        RtcDateTime rtc=Rtc.GetDateTime();
        snprintf_P(datestring,countof(datestring),PSTR("%02u:%02u"),rtc.Hour(),rtc.Minute());
        return datestring;
    }
     String static dateTime(const RtcDateTime& dt)
    {
            char datestring[20];
            snprintf_P(datestring, 
                    countof(datestring),
                    PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
                    dt.Month(),
                    dt.Day(),
                    dt.Year(),
                    dt.Hour(),
                    dt.Minute(),
                    dt.Second() );
           return datestring;
    }  
  protected:
    void loop()  {
	  	server.handleClient();
        delay(1000);  
    }
};
JsonVariant loadSchedulers(){
   JsonVariant filter=NULL;
   File configFile = SPIFFS.open(FILE_NAME,"r");
   if(configFile){
     size_t size = configFile.size();
     std::unique_ptr<char[]> buf (new char[size]);
     configFile.readBytes(buf.get(), size);
     JsonObject& root = jsonBuffer.parseObject(buf.get());
     if(root.success() && root.containsKey("scheduler")){
            filter=root["scheduler"];
      }
   }
   configFile.close();
   return filter;
}
class SchedulerTask:public Task {
 JsonVariant scheduler=NULL;
public:
      void setData(JsonArray& scheduler){
           this->scheduler=scheduler;
      }
protected:
    void loop()  {
	     Serial.println("*****Scheduler Started***************");
       RtcDateTime rtc=Rtc.GetDateTime();
       Serial.println("Current Time  "+ClientExecuter::sysTime());
       delay(1000);
       if(this->scheduler!=NULL){
           JsonArray& schedulers=this->scheduler.as<JsonArray&>();
           for(JsonArray::iterator it=schedulers.begin(); it!=schedulers.end(); ++it) {
                   JsonObject& scheduler=it->as<JsonObject>();
                   if(scheduler["status"]==1){
                      if(scheduler["startTime"]==ClientExecuter::sysTime() && scheduler["isRunning"]==0){
                            digitalWrite(scheduler["deviceId"],HIGH);  
                            scheduler["isRunning"]=1;
                      }else if(scheduler["endTime"]==ClientExecuter::sysTime()){
                            digitalWrite(scheduler["deviceId"],LOW);
                            scheduler["isRunning"]=0;
                      }
                   }
            }
       }
       delay(1000);
       Serial.println("*********End Scheduler***************");
    }
};
SchedulerTask relayScheduler;
/*Time Setting into RTC Device*/
void time_rest_on(RtcDateTime compiled){
  RtcDateTime rtc_now = Rtc.GetDateTime();
  if (!Rtc.IsDateTimeValid()){Rtc.SetDateTime(compiled);}
  //if (rtc_now<compiled){Rtc.SetDateTime(compiled);} 
  Rtc.SetDateTime(compiled);
  if (!Rtc.GetIsRunning()){Rtc.SetIsRunning(true);}
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}
/*WIFI Manager Setup*/
void deviceCommunication(){
   server.send(200,"text/plain","Success");
}
void getDeviceInfo(){
  String device=server.arg("device");
  String isExisting=server.arg("isExisting");
  Serial.println("*********IsExisting Flag**********************");
  Serial.println(isExisting);
  if(device.length()!=0){
     Serial.println("********Request Payload**************");
     Serial.println(device);
     JsonObject& root = jsonBuffer.parseObject(device);
     root.prettyPrintTo(Serial);
     if(saveConfig(root,"devices")){
          server.send(200,"text/plain","Success");
     }else{
          server.send(500,"text/plain","Failure");
     }
  }else if(isExisting=="true"){
     Serial.println("*************************Fetching Configuration List**************************"); 
     String response="{\"status\": \"success\",\"devices\":"+getConfig("devices",0)+"}";
     server.send(200,"application/json;charset=utf-8",response);
  }else{
    String response="{\"status\": \"success\",\"devices\": [{\"deviceId\": \"DEVICE1\",\"deviceMode\": 2},";
    response+="{\"deviceId\": \"DEVICE2\",\"deviceMode\": 12},{\"deviceId\": \"DEVICE3\",\"deviceMode\": 13},";
    response+="{\"deviceId\": \"DEVICE4\",\"deviceMode\": 14},{\"deviceId\": \"DEVICE5\",\"deviceMode\": 15},";
    response+="{\"deviceId\": \"DEVICE6\",\"deviceMode\": 16}]}";
    server.send(200,"application/json;charset=utf-8",response);
  }
}
void manualConfig(){
   String state=server.arg("state");
   int pinmode=server.arg("pinmode").toInt();
   if(state=="true"){digitalWrite(pinmode,HIGH);} 
   else if(state=="false"){digitalWrite(pinmode,LOW);}
   server.send(200, "text/plain", "Led is now("+state+")");
}
void synctime(){
   String _date=server.arg("date");
   String _time=server.arg("time");
   if(_date.length()!=0 && _time.length()!=0){
       RtcDateTime compiled = RtcDateTime(_date.c_str(),_time.c_str());
       time_rest_on(compiled);
       server.send(200,"text/plain","Success");
   }else{
      RtcDateTime rtc_now = Rtc.GetDateTime();
      server.send(200,"text/plain",ClientExecuter::dateTime(rtc_now));
   }
}
void schedulerConfig(){
  Serial.println("***Invoking SchedulerConfig********************");
  String scheduler=server.arg("scheduler");
  int deviceId=server.arg("deviceId").toInt();
  if(scheduler.length()!=0){
     Serial.println("********Request Payload**************");
     JsonObject& root = jsonBuffer.parseObject(scheduler);
     if(saveConfig(root,"scheduler")){
          relayScheduler.setData(loadSchedulers());
          server.send(200,"text/plain","Success");
     }else{
          server.send(500,"text/plain","Failure");
     }
  }else if(deviceId!=0){
     Serial.println("Response");
     Serial.println(getConfig("scheduler",deviceId));
     String response="{\"status\": \"success\",\"scheduler\":"+getConfig("scheduler",deviceId)+"}";
     server.send(200,"application/json;charset=utf-8",response);
  }
}
void WifiSetup(){
  //WiFi.config(ip, gateway, subnet);
  WiFi.mode(WIFI_AP);
  //WiFi.mode(WIFI_STA);
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(SSID_NAME,PASSWORD);
  Serial.println(WiFi.localIP());
  server.on("/verify",deviceCommunication); 
  server.on("/deviceinfo",getDeviceInfo);
  server.on("/manualconfig",manualConfig);
  server.on("/schedulerconfig",schedulerConfig);
  server.on("/synctime",synctime);
  Serial.println("IP Address"+WiFi.localIP());
  server.begin();
}
/*End WIFI Manager Complete*/
void lcd_setup(){
  lcd.begin(16,2);
  lcd.init();
  // Turn on the backlight.
  lcd.backlight();
  // Move the cursor characters to the right and
  // zero characters down (line 1).
  lcd.setCursor(5, 0);
  // Print HELLO to the screen, starting at 5,0.
  lcd.print("HELLO");
  // Move the cursor to the next line and print
  // WORLD.
  lcd.setCursor(5, 1);      
  lcd.print("WORLD");
}
#endif
