/**
 * 
 * 2020 by Raymond Blum <raygeeknyc@gmail.com>.
 * GPL license V3 and later
 */
// define _DEBUG for serial output
#define _DEBUG

#include <Adafruit_Microbit.h>

#define R_SENSOR_PIN 1
#define L_SENSOR_PIN 2

#define BUTTON_A_PIN  5
#define BUTTON_B_PIN 11  

// These are set through observation of driest and wettest conditions
#define SENSOR_MAX 820
#define SENSOR_MIN 0

#define DISPLAY_TIERS 5
// The range of sensor values isn't evenly distributed: the higher values have lower granularity than the lower values
const int SENSOR_TIER_RANGES[DISPLAY_TIERS] = {300, 200, 170, 100, 50};   // These should sum to approx (SENSOR_MAX - SENSOR_MIN) 

const int SENSOR_RAW_RANGE = SENSOR_MAX + 1;
const int SENSOR_RANGE = SENSOR_MAX - SENSOR_MIN + 1;
const float SENSOR_SCALE_FACTOR = (float)SENSOR_RANGE / SENSOR_RAW_RANGE;

#define SENSOR_SAMPLE_COUNT 5
#define SAMPLE_POLLING_DELAY_MS 100

#define DISPLAY_DELAY_MS 2000

// There's some language issues with using min and max functions - this is an easy workaround
inline int min_i(int a,int b) {return ((a)<(b)?(a):(b)); }
inline int max_i(int a,int b) {return ((a)>(b)?(a):(b)); }

Adafruit_Microbit_Matrix microbit;

int min_l_reading, min_r_reading;
int max_l_reading, max_r_reading;

int smooth(int array[], int len) {
  /**
    Return the average of the array without the highest and lowest values.
  **/
  int low = SENSOR_MAX;
  int high = -1;
  int total = 0;
  for (int s = 0; s < len; s++) {
    total += array[s];
    low = min(array[s], low);
    high = max(array[s], high);
  }
  total -= low;
  total -= high;
  return total / (len - 2);
}

int getSensorLevel(const int sensor_pin) {
  // Return the median reading from SENSOR_SAMPLE_COUNT samples of the analog sensor
  // at sensor_pin
  int samples[SENSOR_SAMPLE_COUNT];
  for (int sample = 0; sample < SENSOR_SAMPLE_COUNT; sample++) {
    samples[sample] = analogRead(sensor_pin);
    delay(SAMPLE_POLLING_DELAY_MS);
  }
  return smooth(samples, SENSOR_SAMPLE_COUNT);
}

int scaleToDisplay(const int raw_value) {
  if (raw_value < SENSOR_MIN) {
    return 1;
  } else if (raw_value >= SENSOR_MAX) {
    return DISPLAY_TIERS;
  }
  int display_tier = 0;
  /* Linear scaling didn't match what we saw from the sensors
  int scaled_value = (float) raw_value * SENSOR_SCALE_FACTOR;
  int display_tier = min(int(value_pct * DISPLAY_TIERS)+1, DISPLAY_TIERS);
  */
  int tier_ceiling = 0;
  for (int i=0; i<DISPLAY_TIERS; i++) {
    tier_ceiling += SENSOR_TIER_RANGES[i];
    if (raw_value < tier_ceiling) {
      display_tier = i+1;
      break;
    }
  }
  #ifdef _DEBUG
    Serial.print("value: ");
    Serial.print(raw_value);
    Serial.print(" tier: ");
    Serial.print(display_tier);
  #endif

  return display_tier;
}

void setup() {
  #ifdef _DEBUG
    Serial.begin(9600);
    Serial.println("setup");
    
    Serial.print("Sensor range:");
    Serial.print(SENSOR_RANGE);
    Serial.print(" scale factor: ");
    Serial.println(SENSOR_SCALE_FACTOR);
  #endif
  
  microbit.begin();
  pinMode(BUTTON_A_PIN, INPUT);
  pinMode(BUTTON_B_PIN, INPUT);

  pinMode(R_SENSOR_PIN, INPUT);
  pinMode(L_SENSOR_PIN, INPUT);
  
  #ifdef _DEBUG
    Serial.println("LED check");
    microbit.fillScreen(LED_ON);
    delay(1000);
    displayBar(0,1);
    delay(500);
    displayBar(1,2);
    delay(500);
    displayBar(2,3);
    delay(500);
    displayBar(3,4);
    delay(500);
    displayBar(4,5);
    delay(1000);
  #endif
  
  microbit.fillScreen(LED_OFF);
  #ifdef _DEBUG
    Serial.println("/setup");
  #endif
}

void displayBar(const int column, const int value) {
  microbit.drawLine(column, DISPLAY_TIERS-1, column, 0, LED_OFF);
  if (value > 0) {  
    microbit.drawLine(column, DISPLAY_TIERS-1, column, DISPLAY_TIERS-value, LED_ON);
  }
}

void loop() {
  int scaled_r;
  int scaled_l;

  int l_sensor_reading = getSensorLevel(L_SENSOR_PIN);
  int r_sensor_reading = getSensorLevel(R_SENSOR_PIN);

  min_r_reading = min(min_r_reading, r_sensor_reading);
  min_l_reading = min(min_l_reading, l_sensor_reading);
  max_r_reading = max(max_r_reading, r_sensor_reading);
  max_l_reading = max(max_l_reading, l_sensor_reading);
  
  if (! digitalRead(BUTTON_A_PIN)) {
    #ifdef _DEBUG
      Serial.println("BUTTON A");
    #endif
    scaled_l = scaleToDisplay(min_l_reading);
    scaled_r = scaleToDisplay(min_r_reading);
    displayBar(2, 0);
  } else if (! digitalRead(BUTTON_B_PIN)) {
    #ifdef _DEBUG
      Serial.println("BUTTON B");
    #endif
    scaled_l = scaleToDisplay(max_l_reading);
    scaled_r = scaleToDisplay(max_r_reading);
    displayBar(2, DISPLAY_TIERS);
  } else {    
    scaled_l = scaleToDisplay(l_sensor_reading);
    scaled_r = scaleToDisplay(r_sensor_reading);
    displayBar(2, 0);
    microbit.drawPixel(2, 2, LED_ON);
  }
  #ifdef _DEBUG
    Serial.print("L sensor: ");
    Serial.print(l_sensor_reading);
    Serial.print(" scaled: ");
    Serial.println(scaled_l);
    Serial.print("R sensor: ");
    Serial.print(r_sensor_reading);
    Serial.print(" scaled: ");
    Serial.println(scaled_r);
  #endif

  displayBar(0, scaled_l);
  displayBar(1, scaled_l);
  displayBar(3, scaled_r);
  displayBar(4, scaled_r);

  delay(DISPLAY_DELAY_MS);
}
