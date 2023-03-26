#ifndef myPMS7003_h
#define myPMS7003_h

#include "Arduino.h"
#include <SoftwareSerial.h>

// VCC to V5, GND to GND, RX(PMS7003) to D6(Arduino), TX(PMS7003) to D7(Arduino)
#define PIN_TX_PMS7003 16  // PIN matched with TX of PMS7003  
#define PIX_RX_PMS7003 17  // PIN matched with RX of PMS7003  

#define HEAD_1 0x42
#define HEAD_2 0x4d

#define PMS7003_BAUD_RATE 9600 // Serial Speed of PMS7003

SoftwareSerial pmsSerial(PIN_TX_PMS7003,PIX_RX_PMS7003); // RX, TX of Arduino UNO
unsigned char pmsbytes[32]; // array for 32 bytes stream from PMS7003

class myPMS7003 {
  public: 
    int PM1_0_val = 0;
    int PM2_5_val = 0;
    int PM10_val = 0;

  public:
    myPMS7003() {}; // 생성자
    void begin() {
      pmsSerial.begin(PMS7003_BAUD_RATE);
    } // 라이브러리 초기화 함수
    
    int* getDustValues() {
      static int arr[] = {PM1_0_val, PM2_5_val, PM10_val};
      return arr;
    }
    
    void getDust() {
      if(pmsSerial.available()>=32){
        int i=0;

        //initialize first two bytes with 0x00
        pmsbytes[0] = 0x00;
        pmsbytes[1] = 0x00;
        
        for(i=0; i<32 ; i++){
          pmsbytes[i] = pmsSerial.read();

          //check first two bytes - HEAD_1 and HEAD_2, exit when it's not normal and read again from the start
          if( (i==0 && pmsbytes[0] != HEAD_1) || (i==1 && pmsbytes[1] != HEAD_2) ) {
            break;
          }
        }

        if(i>2) { // only when first two stream bytes are normal
          if(pmsbytes[29] == 0x00) {  // only when stream error code is 0
            int _val = (pmsbytes[10]<<8) | pmsbytes[11]; // pmsbytes[10]:HighByte + pmsbytes[11]:LowByte => two bytes
            //if(_val >= 0 && _val < 10000)    /// 유효 범위의 값이 아니면 이전값으로 유지..
              PM1_0_val = _val;
            int _val2 = (pmsbytes[12]<<8) | pmsbytes[13]; // pmsbytes[12]:HighByte + pmsbytes[13]:LowByte => two bytes
            //if(_val2 >= 0 && _val2 < 10000)
              PM2_5_val = _val2;
            int _val3 = (pmsbytes[14]<<8) | pmsbytes[15]; // pmsbytes[14]:HighByte + pmsbytes[15]:LowByte => two bytes
            //if(_val3 >= 0 && _val3 < 10000)
              PM10_val = _val3;
            
            //String _dust = String(PM1_0_val) + ":" + String(PM2_5_val) + ":" + String(PM10_val);
            //Serial.println(_dust);
            //return _dust;
          } else {
            Serial.println("Error skipped..");
          }
        } else {
          Serial.println("Bad stream format error");
        }
      }
    } // 라이브러리에서 사용할 함수

  private:
    // 프라이빗 변수나 함수는 이곳에 선언합니다.
};

#endif


