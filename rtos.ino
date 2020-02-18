#include <AFMotor.h>
#include <MQUnifiedsensor.h>
#include <SoftwareSerial.h>
#include <Arduino_FreeRTOS.h>
#include <WiFiEsp.h>
#include <ThingSpeak.h>

AF_DCMotor waterPump(3);
AF_DCMotor fan(1);
AF_DCMotor motor(2);

int i;
boolean windowOpened = true;

String agAdi = "cafesessiz1";                 //Ağımızın adını buraya yazıyoruz.    
String agSifresi = "elvangazozu";           //Ağımızın şifresini buraya yazıyoruz.
String durum = "20";

int rxPin = 50;                                               //ESP8266 RX pini
int txPin = 51; 

String ip = "184.106.153.149";

const int buzzer = 37;

int gasSensor;
int flameSensor;


SoftwareSerial esp(rxPin, txPin);
void setup() {
  Serial.begin(9600);
  
  Serial.println("Started");
  esp.begin(115200);                                          //ESP8266 ile seri haberleşmeyi başlatıyoruz.
  esp.println("AT");                                          //AT komutu ile modül kontrolünü yapıyoruz.
  Serial.println("AT Yollandı");
  while(!esp.find("OK")){                                     //Modül hazır olana kadar bekliyoruz.
    esp.println("AT");
    Serial.println("ESP8266 Bulunamadı.");
  }
  Serial.println("OK Komutu Alındı");
  esp.println("AT+CWMODE=1");                                 //ESP8266 modülünü client olarak ayarlıyoruz.
  while(!esp.find("OK")){                                     //Ayar yapılana kadar bekliyoruz.
    esp.println("AT+CWMODE=1");
    Serial.println("Ayar Yapılıyor....");
  }
  Serial.println("Client olarak ayarlandı");
  Serial.println("Aga Baglaniliyor...");
  esp.println("AT+CWJAP=\""+agAdi+"\",\""+agSifresi+"\"");    //Ağımıza bağlanıyoruz.
  while(!esp.find("OK"));                                     //Ağa bağlanana kadar bekliyoruz.
  Serial.println("Aga Baglandi.");


  

  


  pinMode(buzzer, OUTPUT);
  waterPump.setSpeed(255);
  waterPump.run(RELEASE);
  motor.setSpeed(255);
  motor.run(RELEASE);
  fan.setSpeed(60);
  fan.run(RELEASE);

 xTaskCreate(vTask2, //gorev fonksiyon adı
                "Task2", // görev adı sadece hata ayıklamada kullanılır
                300, // yığın derinliği
                NULL, // parametre gönderimi - parametre yok
                3, //öncelik sıralaması
                NULL);

  xTaskCreate(vTask4, //gorev fonksiyon adı
                "Task4", // görev adı sadece hata ayıklamada kullanılır
                300, // yığın derinliği
                NULL, // parametre gönderimi - parametre yok
                2, //öncelik sıralaması
                NULL);
                vTaskStartScheduler();
}


void vTask2(void *pvParameters) {
  
  for(;;){
  TickType_t xLastWakeTime;
  xLastWakeTime=xTaskGetTickCount();
  vTaskDelayUntil(&xLastWakeTime,100);
    int32_t flameSensor = analogRead(A8);
    int32_t gasSensor = analogRead(A9); 
  if(flameSensor < 900) {
     while(1){
      
     flameSensor = analogRead(A8); 
     Serial.println(flameSensor); Serial.print("yangın");
     waterPump.run(FORWARD);
     waterPump.setSpeed(255);
     tone(buzzer, 1000);
     if(windowOpened == true) {
     motor.run(FORWARD);
     motor.setSpeed(255);
     vTaskDelay(1000/portTICK_PERIOD_MS);
     //vTaskDelayUntil(&xLastWakeTime,xDelay3ms);                             //Bağlantıyı kapatıyoruz     
     motor.setSpeed(0);
     windowOpened = false;
    }
     if(flameSensor > 900){
       Serial.println("no prob2");
       windowOpened = true;
       waterPump.setSpeed(0);
       noTone(buzzer);
       motor.run(BACKWARD);
      motor.setSpeed(255);
      vTaskDelay(1000/portTICK_PERIOD_MS);
      motor.setSpeed(0);
      
       break;
     }
   
  }}
   
    if(gasSensor > 200) {
    while(1){
    gasSensor = analogRead(A9);
    Serial.println(gasSensor);
    tone(buzzer, 1000);
    fan.run(FORWARD);
    fan.setSpeed(60);
    if(windowOpened == true) {
     motor.run(FORWARD);
     motor.setSpeed(255);
     vTaskDelay(1000/portTICK_PERIOD_MS);
     motor.setSpeed(0);
     windowOpened = false; 
    }
    
    if(gasSensor < 200){
      Serial.println("noprob1");
      fan.setSpeed(0);
      noTone(buzzer);
      motor.run(BACKWARD);
      motor.setSpeed(255);
      vTaskDelay(1000/portTICK_PERIOD_MS);
      motor.setSpeed(0);
      windowOpened = true;
      
      break;
      
    }
    
    }

  }}
}

void vTask4(void *pvParameters){
  TickType_t xLastWakeTime;
  xLastWakeTime=xTaskGetTickCount();
  for(;;){
  vTaskDelayUntil(&xLastWakeTime,300);
  
  int32_t flameSensor1 = analogRead(A8);
  int32_t gasSensor1 = analogRead(A9);  
  esp.println("AT+CIPSTART=\"TCP\",\""+ip+"\",80");           //Thingspeak'e bağlanıyoruz.
  if(esp.find("Error")){                                      //Bağlantı hatası kontrolü yapıyoruz.
    Serial.println("AT+CIPSTART Error");
  }
      String veri = "GET https://api.thingspeak.com/update?api_key=3Z8QNJAXZEVY9TC9";
  veri += "&field1=";
  veri += String("20");
  veri += "&field2=";
  veri += String("10");                                        //Göndereceğimiz nem değişkeni
  veri += "\r\n\r\n";
  esp.print("AT+CIPSEND=");                                   //ESP'ye göndereceğimiz veri uzunluğunu veriyoruz.
  esp.println(veri.length()+2);
  vTaskDelay(2000/portTICK_PERIOD_MS);
                                            //ESP8266 hazır olduğunda içindeki komutlar çalışıyor.
    esp.print(veri);                                          //Veriyi gönderiyoruz.
    Serial.println(veri);
    Serial.println("Veri gonderildi.");
   vTaskDelay(2000/portTICK_PERIOD_MS);
  
  Serial.println("Baglantı Kapatildi.");
  esp.println("AT+CIPCLOSE");}
  
}

void loop() {
  for(;;){
  }
}
