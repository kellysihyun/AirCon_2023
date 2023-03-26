
///https://github.copCharacteristic);it_CCS811/blob/master/examples/CCS811_test/CCS811_test.ino
/// BLE Server 기능 : 스위치쪽에서 주기적으로 On/Off 해야할지 물어오는 방식으로 처리.: 아두이노 예제 : BLE_Multiconnect

// #include "OLED.h"
// OLED myOled;

// #include "myBleServer.h"
// myBleServer myBLESvr;

#include "myWifi.h"
myWifi myWifiServer;

#include "myPMS7003.h"
myPMS7003 myPMS;

/// 온습도 측정 : DHT22
#include <DHT22.h>
int pinDTH = 5;
DHT22 dht(pinDTH); 

/// 조도센서
int pinILLUM = 33;

/// CO2, TVOC
#include <Adafruit_CCS811.h>
Adafruit_CCS811 ccs;

/// Value 기준
int CO2_BASE = 1000;
int VOC_BASE = 100;
int HUM_BASE = 70;
int TMP_BASE = 30;
int ILM_BASE = 3000;
int DUST_SUM_BASE = 100;

int gTemp = 0;
int gHum = 0;
int gIllum = 0;
int gCo2 = 0;
int gVoc = 0;
int gDust = 0;


/// LED and Servo
int pinLED = 4;   // red
int pinLED2 = 2;  // green
bool gLedOn = false;
int pinBuzzer = 23;
bool gBuzzerOn = false;

#include <ESP32Servo.h>
#include <analogWrite.h>

//#include <tone.h>
//#include <ESP32Tone.h>
#include <ESP32PWM.h>
int pinServo = 19;
Servo servo;

int pinPan = 18;
String _sensor_values = "";

void deviceCtl(float _temp, float _hum, int _illum, int _co2, int _tvoc, int _dust)
{
  if(_illum >= ILM_BASE)
  {
    Serial.println("too dark, so sleep... ");
    gLedOn = false;
    gBuzzerOn = false;
    //myOled.displaySensorData("SiHyun", "Good Night !!");
  }
  else{
    String showMsg = "NORMAL";
    if(_co2 >= CO2_BASE || _tvoc >= VOC_BASE) {
      showMsg = "CO2: " + String(_co2) + "  VOC: " + String(_tvoc);
    } 
    if(_dust >= 5)
      showMsg = "Dust: " + String(_co2);

    if((int)_temp >= TMP_BASE || (int)_hum >= HUM_BASE || _co2 >= CO2_BASE || _tvoc >= VOC_BASE || _dust >= DUST_SUM_BASE)
    {
      gLedOn = true;
      gBuzzerOn = true;
      //myOled.displaySensorData("BAD", showMsg.c_str());
    }
    else {
      gLedOn = false;
      gBuzzerOn = false;
      //myOled.displaySensorData("GOOD", showMsg.c_str());
    }
  }
}

/////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  pinMode(pinLED, OUTPUT);
  pinMode(pinLED2, OUTPUT);
  pinMode(pinBuzzer, OUTPUT);

  pinMode(pinServo, OUTPUT);
  servo.attach(pinServo);
  servo.write(90);     
  delay(100);
  pinMode(pinServo, INPUT); //전원 차단 효과

  pinMode(pinPan, OUTPUT);
  
  pinMode(pinDTH, INPUT);
  dht.begin();
  
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  // Wait for the sensor to be ready
  while(!ccs.available());
  
  // myOled.Init();
  // myOled.displaySensorData("SiHyun", "Welcom !!");

  //myBLESvr.BLEStart();       /// Bluetooth와 wifi 동시 연결은 불가.
  //myWifiServer.begin();   ///  wifi 연결
  bool _bConnected = false;
  //while(!_bConnected)
  { 
    _bConnected = myWifiServer.tryConnect();   ///  wifi 연결
  }

  myPMS.begin();
};

bool bLedTogle = true;
void LedTogle() {
   if(gLedOn && bLedTogle) {
      digitalWrite(pinLED, HIGH);
      bLedTogle = false;
   }
   else {
      digitalWrite(pinLED, LOW);
      bLedTogle = true;
   }
}

int bBuzzerTogle = true;
void BuzzerTogle() {
    if(gBuzzerOn && bBuzzerTogle) {
      //digitalWrite(pinBuzzer, HIGH);
      analogWrite(pinBuzzer, 400);
      bBuzzerTogle = false;
   }
   else {
      //digitalWrite(pinBuzzer, LOW);
      analogWrite(pinBuzzer, 0);
      bBuzzerTogle = true;
   }
}

void ServoOnOff()
{
  if(gLedOn) {      // 스위치는 ON
      digitalWrite(pinLED2, LOW);
      pinMode(pinServo, OUTPUT);
      servo.write(120);   
  }
  else {
      digitalWrite(pinLED2, HIGH);   
      pinMode(pinServo, OUTPUT);
      servo.write(50);   
  }
  
  pinMode(pinServo, INPUT);
}

void CheckSensorValue()
{
  int _temp = round(dht.getTemperature());  // 변수 t에 온도 값을 저장
  if(_temp > -50 && _temp < 100)
    gTemp = _temp;
  int _Hum = gHum = round(dht.getHumidity());     // 변수 h에 습도 값을 저장 
  if(_Hum > 0 && _Hum < 100)
    gHum = _Hum;

  gIllum = analogRead(pinILLUM);
  if(!isnan(gHum) && !isnan(gTemp)) {
    //Serial.print(F("Temp = "));  Serial.print(_temp);
    //Serial.print(F("'C  Humi = "));  Serial.print(_hum);  
    //Serial.print(F("% Illum = "));  Serial.println(_illum);   
  }  
  else 
    Serial.println("Failed to read from DHT sensor!" );  

  gCo2 = 0;
  gVoc = 0;
  if(ccs.available()){
    if(!ccs.readData()){
      gCo2 = ccs.geteCO2();
      gVoc = ccs.getTVOC();
      
      //Serial.print("CO2: ");     Serial.print(_co2);
      //Serial.print("ppm, TVOC: ");   Serial.println(_tvoc);
    }
    else{
      Serial.println("ERROR!");
      while(1);
    }
  } 

  myPMS.getDust();
  int* dustValues = myPMS.getDustValues();
  myWifiServer.sendAirConditionData("myDevice001", gTemp, gHum, gIllum, gCo2, gVoc, dustValues[0], dustValues[1], dustValues[2]);
  gVoc = dustValues[0] + dustValues[1] + dustValues[2];

  _sensor_values = "T:" + String(gTemp) + " H:" + String(gHum) + " I:" + String(gIllum) + " C:" + String(gCo2) + " V:" + String(gVoc);
  Serial.println(_sensor_values.c_str());
}


unsigned int timeCount = 0;  // 1 ms 마다 카운팅 증가
void loop() {
  timeCount++;
  delay(1);
  
  if(timeCount % 10000 == 0)   /// 10초에 한번.
  {
    CheckSensorValue();
    
    //NotifySensorValues(_sensor_values);   /// 주기적으로 전송.
    //myBleServer.BLECheck();
  }
  if(timeCount % 500 == 0)     /// 초마다 한번
  {
      LedTogle();
      ServoOnOff();
  }
  if(timeCount % 1000 == 0)     /// 0.1초마다 한번
  {
     BuzzerTogle();
  }
  if(timeCount % 5000 == 0)     /// 5초마다 한번
  {
      deviceCtl(gTemp, gHum, gIllum, gCo2, gVoc, gDust);
  }
}
