#include <Wire.h>
#include <LiquidCrystal_I2C.h>//D4 d3
#include "ESP8266WiFi.h"
#include <ESP8266HTTPClient.h>
#include "max6675.h"
#include "aREST.h"

aREST rest = aREST();

LiquidCrystal_I2C lcd(0x27, 16, 2); //0x27 is the i2c address, while 16 = columns, and 2 = rows.


WiFiClient client;
HTTPClient http;

const char* ssid = "Domiburguer";
const char* password = "Ventanalimpia";

//pines del max6675
int thermoDO = D5;
int thermoCS = D6;
int thermoCLK = D7;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

int zum = D1;//zumbador

float temperatura = 0; //temperatura
String hora;

String payload ;// varible de la hora

unsigned long int  timpo_de_inicio = 1;

void setup() {
  pinMode(zum, OUTPUT);//zumbador

  Serial.begin(9600);
  Serial.println("hello word");

  Wire.begin(2, 0); // gpio 2 and gpio 0 which are D4, and D3

  lcd.init();                 //Init the LCD
  lcd.backlight();            //Activate backlight
  lcd.home();
  lcd.setCursor(0, 0);
  lcd.print("saludos humanos");

  //init variable Api
  rest.variable("temperatura", &temperatura);
  rest.variable("hora", &hora);

  //name id
  rest.set_id("2");
  rest.set_name("termometroFreidoraDomi");

  //connecten to wifi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }

  Serial.println("Wifi conectado");

  lcd.setCursor(0, 11);
  lcd.print(pedirHora());

  lcd.setCursor(0, 0);
  lcd.print("C: ");


}

void loop() {

  if (millis() > 60000 * timpo_de_inicio) {//pedimos la hora cada 60 segundos
    lcd.setCursor(0, 11);
    lcd.print(pedirHora());
    timpo_de_inicio = timpo_de_inicio + 1;
    escribirMensage();

  }
  escribirTemperatura();
  alerta();
  mandarTemperatura();
}

String pedirHora() {
  if (http.begin(client, "http://iot-domiburguer.herokuapp.com/api/timeHour/timeHourParced/")) {  // HTTP

    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        return payload;
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

}

void escribirTemperatura() {

  if (temperatura > thermocouple.readCelsius()) {
    lcd.setCursor(0, 4);
    temperatura = thermocouple.readCelsius();//variable que queremos mandar    lcd.setCursor(0, 5);
    lcd.print(temperatura);
    lcd.print("+");
  } else if (temperatura < thermocouple.readCelsius()) {
    lcd.setCursor(0, 4);
    temperatura = thermocouple.readCelsius();//variable que queremos mandar    lcd.setCursor(0, 5);
    lcd.print(temperatura);
    lcd.print("-");
  }

}

void mandarTemperatura() {

  http.begin(client, "http://iot-domiburguer.herokuapp.com/api/IOT/DOMIBURGER/FREIDORA/thermometer"); //HTTP
  http.addHeader("Content-Type", "application/json");

  // start connection and send HTTP header and body
  int httpCode = http.POST("{\"temperature\":" + String(temperatura) + " }");

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      const String& payload = http.getString();
      Serial.println(payload);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

void escribirMensage() {
  if (http.begin(client, "http://iot-domiburguer.herokuapp.com/api/messageApp")) {  // HTTP

    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = http.getString();
        lcd.setCursor(1, 0);
        lcd.print(payload);
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

}

void alerta() {
  if (temperatura > 164 && temperatura < 174) {
    digitalWrite(zum, LOW);
    delay(60);
    digitalWrite(zum, HIGH);
  } else if (temperatura > 174 ) {
    digitalWrite(zum, HIGH);
  }
}
