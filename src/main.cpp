#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <DHT.h>

// ================= DHT22 =================
#define DHTPIN 4
#define DHTTYPE DHT22

// ================= ANALOG POTS =================
#define PH_PIN        32
#define MOISTURE_PIN  34
#define N_PIN         35
#define P_PIN         33
#define K_PIN         36

// ================= TFT =================
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  27

#define TFT_SCLK 18
#define TFT_MOSI 23

// ================= LEDs =================
#define LED1 25
#define LED2 16
#define LED3 17
#define LED4 13

// ================= BUZZER =================
#define BUZZER 26

DHT dht(DHTPIN, DHTTYPE);

Adafruit_ST7735 tft = Adafruit_ST7735(
  TFT_CS,
  TFT_DC,
  TFT_RST
);


// =================================================
// SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  // DHT
  dht.begin();

  // SPI
  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // TFT
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);

  // LEDs
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(BUZZER, LOW);

  // Starting screen
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(2);

  tft.setCursor(15, 30);
  tft.println("CROP");

  tft.setCursor(15, 55);
  tft.println("HEALTH");

  tft.setCursor(15, 80);
  tft.println("SCANNER");

  delay(2000);
}


// =================================================
// LOOP
// =================================================

void loop() {

  // ================= DHT22 =================

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("DHT22 Error!");

    temperature = 0;
    humidity = 0;
  }


  // ================= READ POTS =================

  int phRaw = analogRead(PH_PIN);
  int moistureRaw = analogRead(MOISTURE_PIN);
  int nRaw = analogRead(N_PIN);
  int pRaw = analogRead(P_PIN);
  int kRaw = analogRead(K_PIN);


  // ================= CONVERT =================

  float pH = map(phRaw, 0, 4095, 0, 140) / 10.0;

  int moisture =
    map(moistureRaw, 0, 4095, 0, 100);

  int nitrogen =
    map(nRaw, 0, 4095, 0, 100);

  int phosphorus =
    map(pRaw, 0, 4095, 0, 100);

  int potassium =
    map(kRaw, 0, 4095, 0, 100);


  // =================================================
  // CROP HEALTH CONDITIONS
  // =================================================

  bool moistureOK =
    (moisture >= 40 && moisture <= 80);

  bool phOK =
    (pH >= 5.5 && pH <= 7.5);

  bool npkOK =
    (nitrogen >= 40 && nitrogen <= 80 &&
     phosphorus >= 40 && phosphorus <= 80 &&
     potassium >= 40 && potassium <= 80);

  bool temperatureOK =
    (temperature >= 15 && temperature <= 35);


  bool overallOK =
    moistureOK &&
    phOK &&
    npkOK &&
    temperatureOK;


  // =================================================
  // LED INDICATION
  // =================================================

  // LED 1 → Moisture
  digitalWrite(LED1, moistureOK ? HIGH : LOW);

  // LED 2 → pH
  digitalWrite(LED2, phOK ? HIGH : LOW);

  // LED 3 → NPK
  digitalWrite(LED3, npkOK ? HIGH : LOW);

  // LED 4 → Overall health
  digitalWrite(LED4, overallOK ? HIGH : LOW);


  // =================================================
  // BUZZER
  // =================================================

  if (overallOK) {

    digitalWrite(BUZZER, LOW);

  }
  else {

    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
  }


  // =================================================
  // SERIAL MONITOR
  // =================================================

  Serial.println();
  Serial.println("===== CROP HEALTH =====");

  Serial.print("Temperature : ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity    : ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("pH          : ");
  Serial.println(pH);

  Serial.print("Moisture    : ");
  Serial.print(moisture);
  Serial.println(" %");

  Serial.print("Nitrogen    : ");
  Serial.println(nitrogen);

  Serial.print("Phosphorus  : ");
  Serial.println(phosphorus);

  Serial.print("Potassium   : ");
  Serial.println(potassium);

  Serial.print("Overall     : ");

  if (overallOK)
    Serial.println("HEALTHY");
  else
    Serial.println("CHECK REQUIRED");


  // =================================================
  // TFT DISPLAY
  // =================================================

  tft.fillScreen(ST77XX_BLACK);

  // Title
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);

  tft.setCursor(5, 5);
  tft.println("CROP HEALTH");


  // Sensor values
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.setCursor(5, 30);
  tft.print("Temperature: ");
  tft.print(temperature, 1);
  tft.println(" C");

  tft.setCursor(5, 45);
  tft.print("Humidity: ");
  tft.print(humidity, 0);
  tft.println(" %");

  tft.setCursor(5, 60);
  tft.print("pH: ");
  tft.println(pH, 1);

  tft.setCursor(5, 75);
  tft.print("Moisture: ");
  tft.print(moisture);
  tft.println(" %");

  tft.setCursor(5, 90);
  tft.print("N: ");
  tft.print(nitrogen);

  tft.print("  P: ");
  tft.print(phosphorus);

  tft.setCursor(5, 105);
  tft.print("K: ");
  tft.println(potassium);


  // Status
  tft.setCursor(5, 125);

  if (overallOK) {

    tft.setTextColor(ST77XX_GREEN);
    tft.println("STATUS: HEALTHY");

  }
  else {

    tft.setTextColor(ST77XX_RED);
    tft.println("STATUS: CHECK!");

  }


  delay(2000);
}