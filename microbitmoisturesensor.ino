/**
 * 
 * 2020 by Raymond Blum <raygeeknyc@gmail.com>.
 */
// define _DEBUG for serial output
#define _NODEBUG

#include <Adafruit_Microbit.h>

#define R_SENSOR_PIN 1
#define L_SENSOR_PIN 2

#define SENSOR_MAX 1023
#define SENSOR_MIN 50
const int SENSOR_RAW_RANGE = SENSOR_MAX + 1;
const int SENSOR_RANGE = SENSOR_MAX - SENSOR_MIN + 1;
const int SENSOR_SCALE_FACTOR = SENSOR_RANGE / SENSOR_RAW_RANGE;

#define SENSOR_SAMPLE_COUNT 5
#define SAMPLE_POLLING_DELAY_MS 100

#define DISPLAY_TIERS 5

// There's some language issues with using min and max functions - this is an easy workaround
inline int min_i(int a,int b) {return ((a)<(b)?(a):(b)); }
inline int max_i(int a,int b) {return ((a)>(b)?(a):(b)); }

Adafruit_Microbit_Matrix microbit;

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
    return 0;
  } else if (raw_value > SENSOR_MAX) {
    return DISPLAY_TIERS;
  }
  int scaled_value = raw_value * SENSOR_SCALE_FACTOR;
  int value_pct = scaled_value / SENSOR_RANGE;
  int display_tier = max(int(value_pct * DISPLAY_TIERS)+1, DISPLAY_TIERS);

  return display_tier;
}

void setup() {
  pinMode(R_SENSOR_PIN, INPUT);
  pinMode(L_SENSOR_PIN, INPUT);
  microbit.begin();
  
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
  microbit.fillScreen(LED_OFF);
}

void displayBar(const int column, const int value) {
  microbit.drawLine(column, 4, 1, 5, LED_OFF);
  microbit.drawLine(column, 4, 1, value, LED_ON);
}

void loop() {
}
