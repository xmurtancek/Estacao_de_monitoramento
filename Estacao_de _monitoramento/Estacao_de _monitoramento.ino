#include "WiFiEsp.h" //INCLUSÃO DA BIBLIOTECA
#include "secrets.h"// ALTERE ESSA BIBLIOTECA PAR QUE ELA CONTENHA SUAS INFORMAÇÕES
#include <ThingSpeak.h>

#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

#define ESP_BAUDRATE  115200

//bibliotecas DHT
#include <DHT.h>;

//constantes mq7
const double Intervalo5V = 60000;  // number of millisecs of high voltage
const double Intervalo1_4v = 90000;
const double IntervaloLeitura = 50; // number of millisecs between blinks
const double IntervaloLed = 3000;

////constantes thingspeak
const int delay20s=20000;
const int delay5s=5000;
const int delaycurto=100;

//constantes DHT
const int pinoDHT11 = A0;
const int DHT_Interval = 2000;

//declarando mq7
#define analogMQ7 A2      // Signal 
#define ledPin 7         // Device internal LED      
int MQ7sensorValue = 0;   // value read from the sensor

//declarando dht
#define DHTPIN 9 //PINO DIGITAL UTILIZADO PELO DHT22
#define DHTTYPE DHT22 //DEFINE O MODELO DO SENSOR (DHT22 / AM2302)
 
DHT dht(DHTPIN, DHTTYPE); //PASSA OS PARÂMETROS PARA A FUNÇÃO

//variaveis dht e mq7
int hum=0;
float temp=0;
int leitura=0;
float voltagem_MQ=71.4;
unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned long previous_MQ7_Millis = 0;   // will store last time the LED was updated

//variáveis da conexão com o thingspeak
unsigned long previousWifi_Millis=0;
unsigned long previousSSID_Millis=0;
unsigned long previousCMD_Millis=0;


int status = WL_IDLE_STATUS; //STATUS TEMPORÁRIO ATRIBUÍDO QUANDO O WIFI É INICIALIZADO E PERMANECE ATIVO
//ATÉ QUE O NÚMERO DE TENTATIVAS EXPIRE (RESULTANDO EM WL_NO_SHIELD) OU QUE UMA CONEXÃO SEJA ESTABELECIDA
//(RESULTANDO EM WL_CONNECTED)

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiEspClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

void setup(){
  Serial.begin(9600); //INICIALIZA A SERIAL
  Serial1.begin(9600); //INICIALIZA A SERIAL PARA O ESP8266
  WiFi.init(&Serial1); //INICIALIZA A COMUNICAÇÃO SERIAL COM O ESP8266
  pinMode(analogMQ7, INPUT);
  pinMode(ledPin, OUTPUT);
  dht.begin();
  setEspBaudRate(ESP_BAUDRATE);
  WiFi.init(&Serial1);

 if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
   
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}


void loop(){
  currentMillis = millis();
  MQ7Leitura();
  sendDHT();
  Wifi();
}
void Wifi(){

  if (currentMillis - previousWifi_Millis >= delay20s) {
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect to SSID: ");
      Serial.println(SECRET_SSID);
      while(WiFi.status() != WL_CONNECTED){
        if (currentMillis - previousSSID_Millis >= delay5s)
          WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
          Serial.print(".");
          previousSSID_Millis = currentMillis;
      } 
      Serial.println("\nConnected.");
    }

    // set the fields with the values
    ThingSpeak.setField(1, MQ7sensorValue);
    ThingSpeak.setField(2, temp);
    ThingSpeak.setField(3,hum);
    
    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    previousWifi_Millis = currentMillis;
  }
}

void sendDHT(){
  hum = dht.readHumidity();
  temp=dht.readTemperature();
}

void MQ7Leitura(){
  
  if (voltagem_MQ==71.4){
    if (currentMillis - previous_MQ7_Millis >= Intervalo1_4v) {
      analogWrite(analogMQ7, HIGH);
      previous_MQ7_Millis += Intervalo1_4v;
      voltagem_MQ=255;
      Serial.println("high voltage");
    }
  }
  else {
    if (((currentMillis - previous_MQ7_Millis) >= IntervaloLeitura) && leitura==1){
        MQ7sensorValue = analogRead(analogMQ7);
          
        Serial.print("MQ-7 PPM: ");                       
        Serial.println(MQ7sensorValue);  
    
        leitura++;
      }
    if ((currentMillis - previous_MQ7_Millis) >= Intervalo5V) {
      analogWrite(analogMQ7, 71.4);
      previous_MQ7_Millis += Intervalo5V;
      Serial.println("low voltage");
      voltagem_MQ=71.4;
      leitura = 1;
    }
  }
}

void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");
 
  Serial1.begin(baudrate);
}
