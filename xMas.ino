#define ESP32
#include <pins_arduino.h>
#include <config.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "utility/MPU9250.h"
#include "xmas_cfg.h"

extern const unsigned char gImage_logoM5[];
extern const unsigned char m5stack_startup_music[];

#ifndef min
  #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

MPU9250 IMU;

void startupLogo() {
    static uint8_t brightness, pre_brightness;
    uint32_t length = strlen((char*)m5stack_startup_music);
    M5.Lcd.setBrightness(0);
    M5.Lcd.drawPngFile(SD,"/splash.png", 0,0);

    for(int i=0; i<length; i++) {
        dacWrite(SPEAKER_PIN, m5stack_startup_music[i]>>2);
        delayMicroseconds(43);
        brightness = (i/157);
        if(pre_brightness != brightness) {
            pre_brightness = brightness;
            M5.Lcd.setBrightness(brightness);
        }
    }

    M5.Speaker.mute();
    M5.Speaker.end();
    dacWrite(SPEAKER_PIN, 0);

    while(!M5.BtnC.wasPressed())
    {
        M5.update();
        delay(100);
    }
    


    for(int i=255; i>=0; i--) {
        M5.Lcd.setBrightness(i);
        delay(5);
    }

    M5.Lcd.fillScreen(BLACK);
    delay(800);
    for(int i=0; i<=100; i++) {
        M5.Lcd.setBrightness(i);
        delay(2);
    }
}



// the setup routine runs once when M5Stack starts up
void setup() {
    
    // initialize the M5Stack object
    M5.begin();

    /*
      Power chip connected to gpio21, gpio22, I2C device
      Set battery charging voltage and current
      If used battery, please call this function in your project
    */
    M5.Power.begin();
    M5.Power.setPowerBtnEn(true);
    M5.Power.setPowerBoostSet(true);


    WiFi.scanNetworks(true);

    startupLogo();
    
    // Lcd display
    M5.Lcd.setBrightness(50);

    M5.Lcd.textsize = 2;

    const char* ssid = XMAS_CFG_SSID;
    const char* password = XMAS_CFG_PW;
    const char* url = XMAS_CFG_OPENWEATHERURL;

    
    WiFi.setAutoReconnect(true);
    WiFi.begin(ssid,password);
    M5.Lcd.print("Suche Wifi ");
    M5.Lcd.print(ssid);
    
    int timeout = 0;

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        M5.Lcd.print(".");
        timeout++;
        if(timeout == 15)
        {
            timeout = 0;
            WiFi.begin(ssid,password);
        }
        M5.update();
        if(M5.BtnC.wasPressed())
            break;
    }  

    M5.Lcd.clearDisplay();
    M5.Lcd.setCursor(0,0);
    if ((WiFi.status() == WL_CONNECTED)) 
    { //Check the current connection status
 
        HTTPClient http;
    
        http.begin(url); //Specify the URL
        int httpCode = http.GET();  //Make the request
    
        if (httpCode > 0) { //Check for the returning code
    
            String payload = http.getString();
            DynamicJsonDocument JSONBuffer(2048); 
            deserializeJson(JSONBuffer,payload);
            
            double temp = JSONBuffer["main"]["temp"];
            double tempmin = JSONBuffer["main"]["temp_min"];
            double tempmax = JSONBuffer["main"]["temp_max"];
            double feels = JSONBuffer["main"]["feels_like"];
            String name = JSONBuffer["name"];
            
            temp -= 273.15;
            tempmin -= 273.15;
            tempmax -= 273.15;
            feels -= 273.15;



            M5.Lcd.print("Wetterdaten fuer ");
            M5.Lcd.println();
            M5.Lcd.print(name);
            M5.Lcd.println();
            M5.Lcd.println();

            M5.Lcd.print("Temperatur: ");
            M5.Lcd.print(temp);
            M5.Lcd.print("'C");
            M5.Lcd.println();
            
            M5.Lcd.setTextColor(BLUE);
            M5.Lcd.print("Min: ");
            M5.Lcd.print(tempmin);
            M5.Lcd.print("'C");
            M5.Lcd.println();
            
            M5.Lcd.setTextColor(RED);
            M5.Lcd.print("Max: ");
            M5.Lcd.print(tempmax);
            M5.Lcd.print("'C");
            M5.Lcd.println();

            M5.Lcd.setTextColor(GREEN);
            M5.Lcd.print("Feels Like: ");
            M5.Lcd.print(tempmax);
            M5.Lcd.print("'C");
            M5.Lcd.println();

        }
    
        else {
        M5.Lcd.print("Http request failed with status: ");
        M5.Lcd.print(httpCode);
        }
    

        http.end(); //Free the resources

        while(!M5.BtnC.wasPressed())
        {
            M5.update();
            delay(100);
        }

        M5.Lcd.setBrightness(0);
        M5.Lcd.drawPngFile(SD,"/gutschein.png", 0,0);
        M5.Lcd.setBrightness(50);
  }


}

// the loop routine runs over and over again forever
void loop(){
    delay(1000);

}
