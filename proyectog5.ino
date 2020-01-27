#include <ArduinoJson.h>

#include <FirebaseArduino.h>

#include <ESP8266WiFi.h>



#define FIREBASE_HOST "pryt-pst-g5.firebaseio.com"
#define FIREBASE_AUTH "YxwA6o8DQY5jMTicwmpLRgSv5HeQnop8bf4pnX9x"

#define WIFI_SSID "HUAWEI P20 lite"
#define WIFI_PASSWORD "amoakarla"

//definiciones para el sensor TDS

#define TdsSensorPin A0
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;
int c = 0;

void setup()
  {
        Serial.begin(9600);
      
        delay(2000);
        
        Serial.println('\n');
        
        wifiConnect();
      
        Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      
        delay(10);

        //setup sensor TDS

        pinMode(TdsSensorPin,INPUT);
  }

void loop()
  {  
      
      static unsigned long analogSampleTimepoint = millis();
       if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
       {
         analogSampleTimepoint = millis();
         analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
         analogBufferIndex++;
         if(analogBufferIndex == SCOUNT) 
             analogBufferIndex = 0;
       }   
       static unsigned long printTimepoint = millis();
       if(millis()-printTimepoint > 800U)
       {
          printTimepoint = millis();
          for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
            analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
          averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
          float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
          float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
          tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
          //Serial.print("voltage:");
          //Serial.print(averageVoltage,2);
          //Serial.print("V   ");
          Serial.print("TDS Value:");
          Serial.print(tdsValue,0);
          Serial.println("ppm");
          
          Firebase.setFloat("ppm/"+String(c)+"/valor",tdsValue);
          c++;
          if (Firebase.failed()) {
              Serial.print("pushing tds value failed:");
              Serial.println(Firebase.error());  
          return;
          }
          Serial.print("pushed: tds value");
       }
      
    
      delay(10);
      
      if(WiFi.status() != WL_CONNECTED)
      {
        wifiConnect();
      }
      delay(100);
      
  }

void wifiConnect()
  {
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);             // Connect to the network
      Serial.print("Connecting to ");
      Serial.print(WIFI_SSID); Serial.println(" ...");
    
      int teller = 0;
      while (WiFi.status() != WL_CONNECTED)
      {                                       // Wait for the Wi-Fi to connect
        delay(1000);
        Serial.print(++teller); Serial.print(' ');
      }
    
      Serial.println('\n');
      Serial.println("Connection established!");  
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer
  }

 int getMedianNum(int bArray[], int iFilterLen) 
  {
        int bTab[iFilterLen];
        for (byte i = 0; i<iFilterLen; i++)
        bTab[i] = bArray[i];
        int i, j, bTemp;
        for (j = 0; j < iFilterLen - 1; j++) 
        {
        for (i = 0; i < iFilterLen - j - 1; i++) 
            {
          if (bTab[i] > bTab[i + 1]) 
              {
          bTemp = bTab[i];
              bTab[i] = bTab[i + 1];
          bTab[i + 1] = bTemp;
           }
        }
        }
        if ((iFilterLen & 1) > 0)
      bTemp = bTab[(iFilterLen - 1) / 2];
        else
      bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
        return bTemp;
  }
