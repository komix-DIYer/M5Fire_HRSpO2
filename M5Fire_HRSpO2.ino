/*
 * Requirements
 * - Mini Heart Rate Unit (MAX30100) Pulse Oximeter
 *   https://m5stack-store.myshopify.com/products/mini-heart-unit
 * - oxullo/Arduino-MAX30100 library
 *   https://github.com/oxullo/Arduino-MAX30100
 * 
 * ToDo
 * - 音を鳴らすとHR/SpO2 が取得できない
 * 
 * LCD funcs
 * https://github.com/m5stack/M5Stack/blob/master/src/M5Stack.h
 * LCD color
 * https://github.com/m5stack/M5Stack/blob/master/src/utility/In_eSPI.h
 */

#include <M5Stack.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <time.h>

// SpO2/HR
#define REPORTING_PERIOD_MS     100
PulseOximeter pox;
uint32_t tsLastReport = 0;
uint8_t SpO2 = 0;
float HR = 0.0;

// Sound
#define pi 3.1415926535
const uint16_t sampleRate = 3000;
const int amplitude = 256;
int Volume = 1;
double VolAmp = 0.1;

// Wi-Fi
char ssid[] = "SSID";
char pass[] = "PASSWORD";

// Time
char ntpServer[] = "ntp.jst.mfeed.ad.jp";
const long gmtOffset_sec = 9 * 3600;
const int  daylightOffset_sec = 0;
struct tm timeinfo;
String dateStr, yerStr, monStr, datStr;
String timeStr, horStr, minStr, secStr;

// SD - File IO
File file;
const char* fname = "/check.txt";
bool SD_EXIST = false;
bool SD_REC = false;
int flag_onTask = 0;

int g_idx = 0, d_idx = 0;



void playBeep(int freq, int duration, double v)
{
  int n = duration*sampleRate/1000;
  uint8_t MusicData[n];
  makeBeep(MusicData, freq, n, sampleRate, v);
  M5.Speaker.playMusic(MusicData, sampleRate);
}

void makeBeep(uint8_t* MusicData, int freq, int n, int sampleRate, double v)
{
  for(int i=0;i<n;i++){
    MusicData[i] = 127+128*v*sin(2*pi*freq*i/sampleRate);
  }
}
  
void onBeatDetected()
{
    // Serial.println("Beat!");
    if (SpO2<80) SpO2 = 80.0;
    int freq = 440 - 30 * (96 - SpO2);
    playBeep(freq, 100, VolAmp);
}

void getTimeFromNTP(){
  // NTPサーバと時刻を同期
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }
}

void getTime(){
  // 時刻の取得と表示
  getLocalTime(&timeinfo);
  /*
  dateStr = (String)(timeinfo.tm_year + 1900)
          + "/" + (String)(timeinfo.tm_mon + 1)
          + "/" + (String)timeinfo.tm_mday;
  timeStr = (String)timeinfo.tm_hour
          + ":" + (String)timeinfo.tm_min
          + ":" + (String)timeinfo.tm_sec;
  */
  
  yerStr = (String)(timeinfo.tm_year + 1900);
  monStr = (String)(timeinfo.tm_mon + 1);
  if (timeinfo.tm_mon + 1 < 10) monStr = "0" + (String)(timeinfo.tm_mon + 1);
  datStr = (String)timeinfo.tm_mday;
  if (timeinfo.tm_mday < 10) datStr = "0" + (String)timeinfo.tm_mday;
  horStr = (String)timeinfo.tm_hour;
  if (timeinfo.tm_hour < 10) horStr = "0" + (String)timeinfo.tm_hour;
  minStr = (String)timeinfo.tm_min;
  if (timeinfo.tm_min < 10) minStr = "0" + (String)timeinfo.tm_min;
  secStr = (String)timeinfo.tm_sec;
  if (timeinfo.tm_sec < 10) secStr = "0" + (String)timeinfo.tm_sec;

  dateStr = yerStr + "/" + monStr + "/" + datStr;
  timeStr = horStr + ":" + minStr + ":" + secStr;

  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print(dateStr + " " + timeStr);
}


// void writeData(const char *paramStr) {
void writeData() {
  // SDカードへの書き込み処理（ファイル追加モード）
  // SD.beginはM5.begin内で処理されているので不要
  // file = SD.open(fname, FILE_APPEND);
  // file.println(dateStr + ", " + timeStr + ", " + paramStr);
  file.println((String)(millis()*1000) + ", " + (String)flag_onTask + ", " + (String)SpO2 + ", " + (String)(int)HR);
  // file.close();
}

void setup()
{
    //Serial.begin(115200); // M5Stack Fire does not need it.
    M5.begin();
    
    WiFi.begin(ssid, pass);
    M5.Lcd.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      M5.Lcd.print(".");
    }
    // M5.Lcd.println("\nWiFi connected.");
    
    M5.Lcd.clear(BLACK); // 320x240
    // M5.Lcd.drawRect( 10, 10,300,220,WHITE); // 仮枠
    // M5.Lcd.drawLine( 90, 10, 90,230,WHITE); // 仮枠
    // M5.Lcd.drawLine(230, 10,230,230,WHITE); // 仮枠
    M5.Lcd.drawLine( 10, 30,310, 30,WHITE);
    // M5.Lcd.drawLine( 10,150,310,150,WHITE);
    // M5.Lcd.drawLine( 10,210,310,210,WHITE);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor( 10,  10); M5.Lcd.println("YYYY/MM/DD hh:mm:ss");
    M5.Lcd.setCursor(287,  10); M5.Lcd.println("SD");
    // M5.Lcd.fillCircle(276, 16, 6, RED); // Recアイコン
    // M5.Lcd.drawLine( 282,16,315,16, WHITE); // SDカード読み込みエラーサイン
    M5.Lcd.setTextColor(CYAN);
    M5.Lcd.setCursor( 44,  50); M5.Lcd.println("SpO2 [%]");
    M5.Lcd.setTextSize(10); M5.Lcd.setCursor(31, 80); M5.Lcd.println("---");
    M5.Lcd.setTextColor(GREENYELLOW);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(193,  50); M5.Lcd.println("HR [bpm]");
    M5.Lcd.setTextSize(10); M5.Lcd.setCursor(171, 80); M5.Lcd.println("---");
    // M5.Lcd.fillRect(10, 150, 301, 61, DARKGREY);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setTextSize(2);
//    M5.Lcd.drawRect( 30-4, 220-3, 80, 20, WHITE);
//    M5.Lcd.drawRect(120-0, 220-3, 80, 20, WHITE);
//    M5.Lcd.drawRect(210+4, 220-3, 80, 20, WHITE);
    M5.Lcd.fillRect( 30+1, 220-3, 70, 20, DARKGREY);
    M5.Lcd.fillRect(120+5, 220-3, 70, 20, DARKGREY);
    M5.Lcd.fillRect(210+9, 220-3, 70, 20, DARKGREY);
//    M5.Lcd.setCursor( 70-6, 220); M5.Lcd.println("A");
//    M5.Lcd.setCursor(160-2, 220); M5.Lcd.println("B");
//    M5.Lcd.setCursor(250-0, 220); M5.Lcd.println("C");

//    M5.Lcd.setCursor( 70-25, 220); M5.Lcd.println("Vol+"); // 1 char 12 pix(W)
    M5.Lcd.setCursor( 70-25, 220); M5.Lcd.println("Beep");
    M5.Lcd.setCursor(160-15, 220); M5.Lcd.println("Rec");
//    M5.Lcd.setCursor(250-19, 220); M5.Lcd.println("Vol-");
    M5.Lcd.setCursor(250-19, 220); M5.Lcd.println("Task");

    // Check SD card existance
    Serial.print("Checking SD card existance..");
    file = SD.open("/check.txt", FILE_WRITE);
    Serial.println(file);
    if (file)
    {
      SD_EXIST = true;
      file. close();
    }
    else
    {
      M5.Lcd.drawLine( 282,16,315,16, WHITE); // SDカード読み込みエラーサイン
    }
    
    Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) { // PULSEOXIMETER_DEBUGGINGMODE_NONE/RAW_VALUES/AC_VALUES/PULSEDETECT
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
//    pox.setOnBeatDetectedCallback(onBeatDetected);
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
    
    getTimeFromNTP();
    getTime();
    
    M5.Speaker.begin();
    M5.Speaker.setVolume(Volume);
//    playBeep(1000, 100, 0.1);
}

void loop()
{
    M5.update();
    getTime();
  
    // Make sure to call update as fast as possible
    pox.update();

    // Asynchronously dump heart rate and oxidation levels to the serial
    // For both, a value of 0 means "invalid"
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        SpO2 = pox.getSpO2();
        HR = pox.getHeartRate();
        
        Serial.print("Heart rate:");
        Serial.print(HR);
        Serial.print("bpm / SpO2:");
        Serial.print(SpO2);
        Serial.println("%");
        
        M5.Lcd.setTextSize(10);
        // M5.Lcd.setCursor(115, 80); M5.Lcd.println("0"); // 3桁ならx=31, 2桁ならx=73, 1桁ならたぶんx=115
        // M5.Lcd.setCursor(255, 80); M5.Lcd.println("0"); // 3桁ならx=171, 2桁ならx=213, 1桁ならたぶんx=255
        M5.Lcd.fillRect(10, 70, 300, 80, BLACK);
        int x = 0;
        if (SpO2<10)  {x = 115;}
        else
        if (SpO2<100) {x =  73;}
        else          {x =  31;}
        M5.Lcd.setTextColor(CYAN);
        M5.Lcd.setCursor(x, 80); M5.Lcd.println(SpO2);
        if (HR<10)    {x = 255;}
        else
        if (HR<100)   {x = 213;}
        else          {x = 171;}
        M5.Lcd.setTextColor(GREENYELLOW);
        M5.Lcd.setCursor(x, 80); M5.Lcd.println((int)HR);
        
        // グラフ描画
        // M5.Lcd.fillRect(10, 150, 301, 61, DARKGREY);
        g_idx = g_idx + 10;
        if (g_idx > 311-10) g_idx = 10;
        if (SpO2 > 80 && SpO2 <= 100)
        {
          M5.Lcd.fillRect( g_idx, 210-3*(SpO2-81), 10, 3*(SpO2-81), CYAN);
        }
        else
        {
          M5.Lcd.fillRect( g_idx, 210-1, 10, 1, CYAN);
        }
//        M5.Lcd.fillRect( g_idx, 210-3*(90-81), 10, 3*(90-81), CYAN);
        d_idx = g_idx + 10;
        if (d_idx > 311-10) d_idx = 10;
        M5.Lcd.fillRect( d_idx, 150, 10, 61, BLACK);
        
        tsLastReport = millis();
    }

    // ボタン操作
    if (M5.BtnB.wasPressed()) {
      // ボタン押下チェックの前に M5.update() が必要
      
      SD_REC = !SD_REC;
      
      if (SD_REC)
      {
        if (SD_EXIST)
        {
          M5.Lcd.fillCircle(276, 16, 6, RED); // Recアイコン
          file = SD.open("/"+yerStr+monStr+datStr+horStr+minStr+secStr+".csv", FILE_WRITE);
        }
        else
        {
          SD_REC = !SD_REC;
        }
      }
      else
      {
        M5.Lcd.fillCircle(276, 16, 6, BLACK); // Recアイコン消去
        file.close();
        flag_onTask = 0;
      }
    }
//    if (M5.BtnA.wasPressed()) if (VolAmp < 1) VolAmp = VolAmp + 0.1;
//    if (M5.BtnC.wasPressed()) if (VolAmp > 0) VolAmp = VolAmp - 0.1;
    if (M5.BtnA.wasPressed()) if (VolAmp > 0) { VolAmp = 0.0; } else { VolAmp = 0.1; }
    if (M5.BtnC.wasPressed() && SD_REC) 
    {
      if (flag_onTask == 0)
      {
        flag_onTask = 1;
        M5.Lcd.drawCircle(276, 16, 6, ORANGE); // Taksアイコン
        M5.Lcd.fillCircle(276, 16, 3, RED);
      }
      else
      {
        flag_onTask = 0;
        M5.Lcd.drawCircle(276, 16, 6, RED); // Taksアイコン消去
      }
    }
    
    // ロギング
    if (SD_REC) writeData();
    // {
    //   String dataStr = (String)SpO2 + ", " + (String)(int)HR;
    //   writeData(dataStr.c_str());
    // }
}
