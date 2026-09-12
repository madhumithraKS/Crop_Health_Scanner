#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ================= OLED =================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= DHT =================

#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ================= SENSOR PINS =================

#define PH_PIN        32
#define MOISTURE_PIN  34
#define N_PIN         35
#define P_PIN         33
#define K_PIN         36

// ================= TRAFFIC LIGHT =================

#define GREEN_LED  16
#define YELLOW_LED 17
#define RED_LED    18

// ================= OTHER =================

#define WHITE_LED 13
#define BUZZER    12


// =================================================
// Convert parameter value into HEALTH SCORE
// =================================================

float parameterScore(
  float value,
  float normalMin,
  float normalMax,
  float poorLow,
  float poorHigh)
{
  // NORMAL
  if (value >= normalMin && value <= normalMax)
  {
    return 100.0;
  }

  // VERY LOW
  if (value <= poorLow)
  {
    return 0.0;
  }

  // VERY HIGH
  if (value >= poorHigh)
  {
    return 0.0;
  }

  // BETWEEN LOW LIMIT AND NORMAL
  if (value < normalMin)
  {
    return ((value - poorLow) /
            (normalMin - poorLow)) * 100.0;
  }

  // BETWEEN NORMAL AND HIGH LIMIT
  return ((poorHigh - value) /
          (poorHigh - normalMax)) * 100.0;
}


// =================================================
// SET TRAFFIC LIGHT
// =================================================

void setTrafficLight(float health)
{
  // Turn everything OFF first
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // HEALTHY
  if (health >= 80)
  {
    digitalWrite(GREEN_LED, HIGH);
  }

  // MEDIUM
  else if (health >= 60)
  {
    digitalWrite(YELLOW_LED, HIGH);
  }

  // POOR
  else
  {
    digitalWrite(RED_LED, HIGH);

    // Buzzer ON
    digitalWrite(BUZZER, HIGH);
  }
}


// =================================================
// SETUP
// =================================================

void setup()
{
  Serial.begin(115200);

  // DHT
  dht.begin();

  // OLED
  Wire.begin(21, 22);

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDR))
  {
    Serial.println("OLED ERROR!");
    while (1);
  }

  // LED pins
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(WHITE_LED, OUTPUT);

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  // =========================================
  // SYSTEM START
  // =========================================

  digitalWrite(WHITE_LED, HIGH);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Startup OLED
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(15, 10);
  display.println("CROP");

  display.setCursor(15, 35);
  display.println("HEALTH");

  display.display();

  delay(1500);
}


// =================================================
// MAIN LOOP
// =================================================

void loop()
{
  // =========================================
  // READ DHT22
  // =========================================

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // DHT safety
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("DHT reading error!");

    temperature = 25.0;
    humidity = 60.0;
  }


  // =========================================
  // READ ANALOG SENSORS
  // =========================================

  int phRaw = analogRead(PH_PIN);

  int moistureRaw =
      analogRead(MOISTURE_PIN);

  int nRaw = analogRead(N_PIN);

  int pRaw = analogRead(P_PIN);

  int kRaw = analogRead(K_PIN);


  // =========================================
  // CONVERT VALUES
  // =========================================

  // pH = 0 to 14
  float ph =
      (phRaw / 4095.0) * 14.0;

  // Moisture = 0 to 100%
  float moisture =
      (moistureRaw / 4095.0) * 100.0;

  // NPK simulation ranges
  float nitrogen =
      (nRaw / 4095.0) * 6.0;

  float phosphorus =
      (pRaw / 4095.0) * 2.0;

  float potassium =
      (kRaw / 4095.0) * 4.0;


  // =========================================
  // INDIVIDUAL HEALTH SCORES
  // =========================================

  // pH
  float phScore =
      parameterScore(
        ph,
        5.5,
        7.5,
        4.0,
        9.0);


  // Moisture
  float moistureScore =
      parameterScore(
        moisture,
        40,
        80,
        0,
        100);


  // Humidity
  float humidityScore =
      parameterScore(
        humidity,
        50,
        80,
        0,
        100);


  // Temperature
  float temperatureScore =
      parameterScore(
        temperature,
        20,
        30,
        5,
        45);


  // Nitrogen
  float nitrogenScore =
      parameterScore(
        nitrogen,
        2.5,
        4.0,
        0,
        6);


  // Phosphorus
  float phosphorusScore =
      parameterScore(
        phosphorus,
        0.75,
        1.25,
        0,
        2);


  // Potassium
  float potassiumScore =
      parameterScore(
        potassium,
        1.5,
        2.5,
        0,
        4);


  // =========================================
  // CONSOLIDATED HEALTH SCORE
  // =========================================

  float plantHealth =
      (
        phScore +
        moistureScore +
        humidityScore +
        temperatureScore +
        nitrogenScore +
        phosphorusScore +
        potassiumScore
      ) / 7.0;


  // =========================================
  // TRAFFIC LIGHT
  // =========================================

  setTrafficLight(plantHealth);


  // =========================================
  // SERIAL MONITOR
  // =========================================

  Serial.println();
  Serial.println("================================");

  Serial.print("pH Score          : ");
  Serial.println(phScore, 1);

  Serial.print("Moisture Score    : ");
  Serial.println(moistureScore, 1);

  Serial.print("Humidity Score    : ");
  Serial.println(humidityScore, 1);

  Serial.print("Temperature Score : ");
  Serial.println(temperatureScore, 1);

  Serial.print("N Score           : ");
  Serial.println(nitrogenScore, 1);

  Serial.print("P Score           : ");
  Serial.println(phosphorusScore, 1);

  Serial.print("K Score           : ");
  Serial.println(potassiumScore, 1);

  Serial.println("--------------------------------");

  Serial.print("PLANT HEALTH = ");
  Serial.print(plantHealth, 1);
  Serial.println("%");


  // =========================================
  // OLED PAGE 1
  // =========================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("PLANT HEALTH SCANNER");

  display.setCursor(0, 12);
  display.print("Temp : ");
  display.print(temperature, 1);
  display.println(" C");

  display.setCursor(0, 22);
  display.print("Hum  : ");
  display.print(humidity, 1);
  display.println(" %");

  display.setCursor(0, 32);
  display.print("Moist: ");
  display.print(moisture, 1);
  display.println(" %");

  display.setCursor(0, 42);
  display.print("pH   : ");
  display.println(ph, 1);

  display.setCursor(0, 54);
  display.print("HEALTH: ");
  display.print(plantHealth, 1);
  display.println("%");

  display.display();

  delay(1000);


  // =========================================
  // OLED PAGE 2
  // =========================================

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("NPK ANALYSIS");

  display.setCursor(0, 13);
  display.print("N : ");
  display.print(nitrogen, 2);
  display.println("%");

  display.setCursor(0, 25);
  display.print("P : ");
  display.print(phosphorus, 2);
  display.println("%");

  display.setCursor(0, 37);
  display.print("K : ");
  display.print(potassium, 2);
  display.println("%");

  display.setCursor(0, 50);
  display.println("NPK TARGET = 3:1:2");

  display.display();

  delay(1000);


  // =========================================
  // OLED PAGE 3
  // =========================================

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("OVERALL CONDITION");

  display.setTextSize(2);
  display.setCursor(25, 15);

  display.print(plantHealth, 0);
  display.println("%");

  display.setTextSize(1);
  display.setCursor(25, 40);

  if (plantHealth >= 80)
  {
    display.println("NORMAL");
  }
  else if (plantHealth >= 60)
  {
    display.println("MEDIUM");
  }
  else
  {
    display.println("POOR!");
    display.setCursor(15, 52);
    display.println("NEEDS TREATMENT");
  }

  display.display();

  delay(1000);
}