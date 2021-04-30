#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>

const char* ssid = "Davi";
const char* password = "Musubi-11";

const int EchoPin = 14;
const int TriggerPin = 12;

const int sensorPin = A0;

const int ledPin = 2;
// Stores LED state
String ledState;

AsyncWebServer server(80);

String Suelo(){
   float humedad = analogRead(sensorPin);

   byte  porcentajeHumedad =  map(humedad, 0,1023,0,100);

   Serial.print("Humedad en valores reales: ");
   Serial.print(humedad);
   Serial.print("\n");

   Serial.print("Humedad en valores porcentaje: ");
   Serial.print(porcentajeHumedad);
   Serial.print("%");
   Serial.print("\n");
   
   if(porcentajeHumedad < 50)
   {
      Serial.println("Encendido");  
   }
   delay(1000);
   return String(porcentajeHumedad);
 }

String Ultrasonido(int TriggerPin, int EchoPin) {
   long duration, distanceCm;
   
   digitalWrite(TriggerPin, LOW);  //para generar un pulso limpio ponemos a LOW 4us
   delayMicroseconds(4);
   digitalWrite(TriggerPin, HIGH);  //generamos Trigger (disparo) de 10us
   delayMicroseconds(10);
   digitalWrite(TriggerPin, LOW);
   
   duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
   
   distanceCm = duration * 10 / 292/ 2;   //convertimos a distancia, en cm
   byte porcentajeTanque = map(distanceCm, 0, 28, 100,0);
   Serial.print("\n");
   Serial.print(porcentajeTanque);
   Serial.print("% \n");
   return String(porcentajeTanque);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "OFF";
    }
    else{
      ledState = "ON";
    }
    Serial.print(ledState);
    return ledState;
  }
  else if (var == "HUMIDITY"){
    return Suelo();
  }  
  else if (var == "PRESSURE"){
    return Ultrasonido(TriggerPin, EchoPin);
  }  
}

void setup() {
   Serial.begin(115200);
   pinMode(ledPin, OUTPUT);
   pinMode(TriggerPin, OUTPUT);
   pinMode(EchoPin, INPUT);
   
   if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println(WiFi.localIP());


  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Suelo().c_str());
  });

  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Ultrasonido(TriggerPin, EchoPin).c_str());
  });
  server.begin();
}
 
void loop() 
{

}
