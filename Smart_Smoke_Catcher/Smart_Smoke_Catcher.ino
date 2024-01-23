#define vibrator 33
#define in1 25
#define relayHV 32
#define ledBiru 17
#define ledMerah 16

//BLYNK
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "TMPLa315kdur"
#define BLYNK_TEMPLATE_NAME "Mii"
#define BLYNK_AUTH_TOKEN "CzjkN9w0i7HvQGliK2MPGCWw31iPZETD"
#define BLYNK_PRINT Serial

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Mii";
char pass[] = "aa123456";

BlynkTimer timer;

//LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//MQ135IN
#include <MQUnifiedsensor.h>
#define placa "ESP-32"
#define Voltage_Resolution 5
#define pin_mq 39
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6
//#define calibration_button 13 //Pin to calibrate your sensor
MQUnifiedsensor MQ135OUT(placa, Voltage_Resolution, ADC_Bit_Resolution, pin_mq, type);


//vibrator
#include <analogWrite.h>
int vibState = LOW;
unsigned long previousMillis = 0;
long OnTime = 0;
long OffTime = 120000;

void setup() {
  Serial.begin(9600);
  pinMode(vibrator, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(relayHV, OUTPUT);
  pinMode(ledBiru, OUTPUT);
  pinMode(ledMerah, OUTPUT);

  //BLINK
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  //LCD
  lcd.init();
  lcd.backlight();

  float calcR0 = 0;

  //MQ135OUT
  MQ135OUT.setRegressionMethod(1);
  MQ135OUT.init();

  Serial.print("Calibrating please wait.");
  for (int i = 1; i <= 10; i ++)
  {
    MQ135OUT.update();
    calcR0 += MQ135OUT.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135OUT.setR0(calcR0 / 10);
  Serial.println("  done!.");

  if (isinf(calcR0)) {
    Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1);
  }

  //  MQ135OUT.serialDebug(true);

  //start
  digitalWrite(relayHV, LOW);
  digitalWrite(ledBiru, LOW);
  digitalWrite(ledMerah, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("-----WELCOME-----");
  delay(2000);
  //  lcd.clear();
}

void loop() {
  //MQ135OUT
  MQ135OUT.update();

  MQ135OUT.setA(110.47); MQ135OUT.setB(-2.862);
  float CO2OUT = MQ135OUT.readSensor() + 419;

  Serial.print("CO2 OUT = "); Serial.println(CO2OUT);
  Serial.println("");

  lcd.setCursor(0, 1);
  lcd.print("CO2 : ");
  lcd.setCursor(6, 1);
  lcd.print(CO2OUT);
  lcd.setCursor(13, 1);
  lcd.print("PPM");

  delay(500);
  
  //vibrator
  analogWrite(vibrator, 150);
  unsigned long currentMillis = millis();

  if ((vibState == HIGH) && (currentMillis - previousMillis >= OnTime)) {
    vibState = LOW;
    previousMillis = currentMillis;
    digitalWrite(in1, vibState);
    Serial.println("VIBRATOR MATI");
  }
  else if ((vibState == LOW) && (currentMillis - previousMillis >= OffTime)) {
    vibState = HIGH;
    previousMillis = currentMillis;
    digitalWrite(in1, vibState);
    Serial.println("VIBRATOR HIDUP");
  }

  //BLYNK
  Blynk.run();
  timer.run();

  Blynk.virtualWrite(V1, CO2OUT);

}

// RelayHV
BLYNK_WRITE(V2) {
  int btn = param.asInt();
  Serial.print("relayHV:"); Serial.println(btn);
  if (btn == 1) {
    digitalWrite(relayHV, HIGH);
    digitalWrite(ledBiru, HIGH);
    digitalWrite(ledMerah, LOW);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Plasma Menyala");
    OnTime = 3000;

  }
  if (btn == 0) {
    digitalWrite(relayHV, LOW);
    digitalWrite(ledBiru, LOW);
    digitalWrite(ledMerah, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Plasma Mati");
    OnTime = 0;
  }
}
