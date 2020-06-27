#define I2C_ADDR 0x3F
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,2,1,0,4,5,6,7,3, POSITIVE);
#define esp8266 Serial1
#define speed8266 115200

String statusChWriteKey = "";  

// Reset do esp 8266
#define HARDWARE_RESET 2

//sensor dht
#include "dht.h"
#include <dht.h>

#include <stdlib.h>
#define dht_apin A0
dht DHT;
int airTemp = 0 ;
int airHum = 0;


// sensor humidade do solo
#define pino_sinal_analogico A4
int soilHum = 0;

// Variables to be used with timers
long writeTimingSeconds = 17; // ==> tempo para enviar data ao thingspeak
long readTimingSeconds = 10; // ==> tempo para receber data do thingspeak
long startReadTiming = 0;
long elapsedReadTime = 0;
long startWriteTiming = 0;
long elapsedWriteTime = 0;

//rele
int rele = 3;
 
boolean error;

void setup()
{
  lcd.begin(16,2);
  Serial.begin(9600);
  Serial1.begin(115200); // Comunicacao com Modulo WiFi
  pinMode(soilHum, INPUT); 
  pinMode(rele,OUTPUT);
  pinMode(HARDWARE_RESET,OUTPUT);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0, 0);
  lcd.print("WATERING CAN");
  lcd.setCursor(6, 1);
  lcd.print("INICIANDO");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(1, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(2, 1);
  lcd.print(".");
  delay(500);   
  digitalWrite(rele, HIGH); //o módulo relé é ativo em LOW
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(500);
  }

void loop()
{
  DHT.read11(dht_apin);
  start: //label 
  error=0;
  elapsedWriteTime = millis()-startWriteTiming; 
  elapsedReadTime = millis()-startReadTiming; 

 if (elapsedReadTime > (readTimingSeconds*1000)) 
  {
    ESPcheck();
    startReadTiming = millis();   
  }
  
  if (elapsedWriteTime > (writeTimingSeconds*1000)) 
  {
    ESPcheck();//executar antes de qualquer leitura ou gravação
    readSensors();
    writeThingSpeak();
    startWriteTiming = millis();   
  }
  
  if (error==1) //Reevio da informação se a mesma não foi completada
  {       
    goto start; //go to label "start"
  }
}

/********* Read Sensors value *************/
void readSensors(void)
{
  airTemp = DHT.temperature;
  Serial.print("Temperatura = ");
  Serial.println(airTemp); 
  airHum = DHT.humidity;
  Serial.print("Umidade do ar = ");
  Serial.println(airHum);       
  soilHum = map(analogRead(pino_sinal_analogico), 1023, 0, 0, 100); 
  Serial.print("Umidade do solo =  ");
  Serial.println(soilHum); 

 
  EspHardwareReset(); //Reset do Modulo WiFi
  connectWiFi();

  lcd.clear();  
  lcd.print("WATERING CAN");
  delay(1000);
  lcd.setCursor(5, 1);
  lcd.print("MONITORANDO ");
  lcd.setCursor(0, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(1, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(2, 1);
  lcd.print(".");
  delay(500);
  lcd.clear();
 for(int contador = 0; contador<1; contador++){    
 if (soilHum >= 0 && soilHum < 30)
  {
    Serial.println("Status: Solo seco");
    lcd.setCursor(0, 0); 
    lcd.print("SOLO SECO");
    delay(1000);
   
    lcd.setCursor(7,1);
    lcd.print("IRRIGANDO");
    Serial.println("Irrigando");
    
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(500);
    lcd.setCursor(1, 1);
    lcd.print(".");
    delay(500);
    lcd.setCursor(2, 1);
    lcd.print(".");
    delay(500);
    lcd.setCursor(3, 1);
    lcd.print(".");
    delay(500);
    lcd.setCursor(4, 1);
    lcd.print(".");
    delay(500);
    lcd.setCursor(5, 1);
    lcd.print(".");
    delay(500);    
    digitalWrite(rele, LOW);
    delay(8000);    
    digitalWrite(rele, HIGH);
    delay(500);
    lcd.clear();
    lcd.print("UMIDADE DO SOLO");
    lcd.setCursor(3, 1);
    lcd.print(soilHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000);  
    lcd.clear();
    lcd.print("HUMIDADE DO AR");
    lcd.setCursor(3, 1);
    lcd.print(airHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.print("TEMPERATURA");
    lcd.setCursor(3, 1);
    lcd.print(airTemp); 
    lcd.setCursor(6, 1);
    lcd.print("C");
    delay(500);
    lcd.clear();

  }
 
  //Solo com umidade moderada, acende led amarelo
  else if (soilHum > 30 && soilHum < 80)
  {
    Serial.println(" Status: Umidade moderada");
    lcd.clear();
    lcd.print("SOLO NORMAL");
    delay(1000);
    lcd.clear();
    lcd.print("UMIDADE DO SOLO");
    lcd.setCursor(3, 1); 
    lcd.print(soilHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000);  
    lcd.clear();
    lcd.print("UMIDADE DO AR");
    lcd.setCursor(3, 1);
    lcd.print(airHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.print("TEMPERATURA");
    lcd.setCursor(3, 1);
    lcd.print(airTemp); 
    lcd.setCursor(6, 1);
    lcd.print("C");
    delay(500);
    lcd.clear();
  }
 
  else if (soilHum > 80)
  {
    Serial.println("Status: Solo umido");
    lcd.clear();
    lcd.print("SOLO UMIDO");
    delay(1000); 
    lcd.clear();
    lcd.print("UMIDADE DO SOLO");
    lcd.setCursor(3, 1);
    lcd.print(soilHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000); 
    lcd.clear();
    lcd.print("UMIDADE DO AR");
    lcd.setCursor(3, 1);
    lcd.print(airHum);
    lcd.setCursor(6, 1);
    lcd.print("%");
    delay(2000);
    lcd.clear();
    lcd.print("TEMPERATURA");
    lcd.setCursor(3, 1);
    lcd.print(airTemp); 
    lcd.setCursor(6, 1);
    lcd.print("C");    
    delay(500);
    lcd.clear();  
  }  
  else 
  Serial.println("erro");
  }
}


/********* Conexao com TCP com Thingspeak *******/
void writeThingSpeak(void)
{
  startThingSpeakCmd();
  lcd.clear();
  lcd.print("INICIANDO");
  lcd.setCursor(10, 1);
  lcd.print("UPLOAD");
  lcd.setCursor(0, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(1, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(2, 1);
  lcd.print(".");
  delay(500);
  int val = 0;
  int b = airHum;
  int c = airTemp;
  
  val+= soilHum;  
  // preparacao da string GET
  String getStr = "GET /update?api_key=";
  getStr += statusChWriteKey;
//  getStr +="&field1=";
//  getStr += String(pump);
  getStr +="&field2=";
  getStr += String(c);
  getStr +="&field3=";
  getStr += String(val);
  getStr +="&field4=";
  getStr += String(b);
  //getStr +="&field5=";
  //getStr += String(spare);
  getStr += "\r\n\r\n";
  sendThingSpeakGetCmd(getStr); 
}

/********* Echo Command *************/
boolean echoFind(String keyword)
{
 byte current_char = 0;
 byte keyword_length = keyword.length();
 long deadline = millis() + 5000; // Tempo de espera 5000ms
 while(millis() < deadline){
  if (Serial1.available()){
    char ch = Serial1.read();
    Serial.write(ch);
    if (ch == keyword[current_char])
      if (++current_char == keyword_length){
       Serial.println();
       return true;
    }
   }
  }
 return false; // Tempo de espera esgotado
}

/********* Reset ESP *************/
void EspHardwareReset(void)
{
  Serial.println("Reseting......."); 
  digitalWrite(HARDWARE_RESET, LOW); 
  delay(500);
  digitalWrite(HARDWARE_RESET, HIGH);
  delay(7000);//Tempo necessário para começar a ler 
  Serial.println("RESET"); 
}

/********* Check ESP *************/
boolean ESPcheck(void)
{
  Serial1.println("AT"); // Send "AT+" command to module
  if (echoFind("OK")) 
  {
   Serial.println("ESP ok");
    return true; 
  }
  else //Freeze ou Busy
  {
   Serial.println("ESP Freeze ******************************************************");
   EspHardwareReset();
   return false;  
  }
}

/********* Start communication with ThingSpeak*************/
void startThingSpeakCmd(void)
{
  Serial1.flush();//limpa o buffer antes de começar a gravar
  String cmd = "AT+CIPSTART=\"TCP\",\"";                             
  cmd += "184.106.153.149"; // Endereco IP de api.thingspeak.com
  cmd += "\",80";
  Serial1.println(cmd);
  Serial.print("Tentar conectar ==> conexao cmd: ");
  Serial.println(cmd);
  if(Serial1.find("Error"))
  {
    Serial.println("Erro no comando AT+CIPSTART, erro ao conectar no thingspeak");
    return;
  }
}

/********* send a GET cmd to ThingSpeak *************/
String sendThingSpeakGetCmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  Serial1.println(cmd);
  Serial.print("enviado ==> tamanho cmd: ");
  Serial.println(cmd);
  if(Serial1.find((char *)">"))
  {
    Serial1.print(getStr);
    Serial.print("enviado ==> get: ");
    Serial.println(getStr);
    delay(500);//tempo para processar o GET, sem este delay apresenta busy no próximo comando
    String messageBody = "";
    while (Serial1.available()) 
    {
      String line = Serial1.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = Serial1.readStringUntil('\n');
      }
    }
    Serial.print("Messagem recebida: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    Serial1.println("AT+CIPCLOSE");     // alert user
    Serial.println("Erro no comando CIPSEND: reenviando..."); //Resend...
    error=1;
    return "error";
  } 
}

/***************************************************
* Connect WiFi
****************************************************/
void connectWiFi(void)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONECTANDO");
  delay(500);
  lcd.setCursor(3, 1);
  lcd.print("AO THINGSPEAK");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(1, 1);
  lcd.print(".");
  delay(500);
  lcd.setCursor(2, 1);
  lcd.print(".");
  delay(500);
  
  sendData("AT+RST\r\n", 2000, 0); // reset
  sendData("AT+CWJAP=\"Oliveira\",\"Pvd5r3qj7k8aM\"\r\n", 2000, 0); //Connect network
  sendData("AT+CWMODE=1\r\n", 1000, 0);
  sendData("AT+CIFSR\r\n", 1000, 0); // Show IP Adress
  Serial.println("Conectado a rede local");
  lcd.clear();
  delay(100);
}

/***************************************************
* Send AT commands to module
****************************************************/

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  Serial1.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial1.available())
    {
      // The esp has data so display its output to the serial window
      char c = Serial1.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
