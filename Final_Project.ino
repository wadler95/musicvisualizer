
#include <Adafruit_NeoPixel.h>

#define N_PIXELS  30  // Number of pixels
#define MIC_PIN   A0  // Microphone input
#define LED_PIN    1  // NeoPixel LED strand input
#define DC_OFFSET  0  // DC offset in mic signal
#define NOISE     100  // Noise/hum/interference in mic signal
#define SAMPLES   100  // Length of buffer for dynamic level adjustment 
//playing around with the noise and samples is hugely important towards creating smooth looking transitions 
#define TOP       (N_PIXELS +1) // Allow dot to go slightly off scale
#define POT_PIN    A8  // potentiometer input for brightness

byte
peak      = 0,      // Used for falling dot
dotCount  = 0,      // Frame counter for delaying dot-falling speed
volCount  = 0;      // Frame counter for storing past volume data

int
vol[SAMPLES],       // Collection of prior volume samples
    lvl       = 10,     // Current "dampened" audio level
    minLvlAvg = 0,      // For dynamic adjustment of graph low & high
    maxLvlAvg = 512;

Adafruit_NeoPixel  strip = Adafruit_NeoPixel(N_PIXELS, LED_PIN, NEO_GRBW + NEO_KHZ800);
long colorcycle = 0;
void setup() {
  
  //memset(vol, 0, sizeof(vol));
  memset(vol, 0, sizeof(int)*SAMPLES);
  randomSeed(analogRead(0));
  strip.begin();
}
void loop() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  int      n, height;
  n   = analogRead(MIC_PIN);                 // Raw reading from mic
  n   = abs(n - 512 - DC_OFFSET);            // Center on zero
  n   = (n <= NOISE) ? 0 : (n - NOISE);      // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;    // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);

  if (height < 0L)       height = 0;     // Clip output
  else if (height > TOP) height = TOP;
  if (height > peak)     peak   = height; // Keep 'peak' dot at top

  uint8_t bright = 255;
#ifdef POT_PIN
  bright = analogRead(POT_PIN);  // Read pin (0-255) (adjust potentiometer
  //   to give 0 to Vcc volts
#endif
  strip.setBrightness(255);    // Set LED brightness (if POT_PIN at top
  //  define commented out, will be full)

  colorcycle++;
  if (colorcycle >= 4000) {
    colorcycle = 0;
  }

  for (i = 0; i < N_PIXELS; i++) {
    if (i >= height) {
      strip.setPixelColor(i, 0, 0, 0, bright);
    }
    else {
      strip.setPixelColor(i, randColor(colorcycle));
    }
  }

  strip.show(); // Update strip


  vol[volCount] = n;
  if (++volCount >= SAMPLES) volCount = 0;

  // Get volume range of prior frames
  minLvl = maxLvl = vol[0];
  for (i = 1; i < SAMPLES; i++) {
    if (vol[i] < minLvl)      minLvl = vol[i];
    else if (vol[i] > maxLvl) maxLvl = vol[i];
  }
  if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6;
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6;
}

// Input a value 0 to 255 to get a color value.
// The colors are r - g - b - w

uint32_t randColor(long colorcycle) {
  if (colorcycle > 4000) {
    return strip.Color(0, 255, 0, 0); //green
  } else if (colorcycle <= 4000 && colorcycle > 3600) {
    return strip.Color(0, 0, 255, 0); //blue
  } else if (colorcycle <= 3200 && colorcycle > 2800) {
    return strip.Color(255, 0, 0, 0); //red
  } else if (colorcycle <= 2800 && colorcycle > 2400) {
    return strip.Color(0, 0, 0, 255); //white
  } else if (colorcycle <= 2400 && colorcycle > 2000) {
    return strip.Color(32, 255, 7, 0); //lime green
  } else if (colorcycle <= 2000 && colorcycle > 1600) {
    return strip.Color(255, 255, 0, 0); //yellow
  } else if (colorcycle <= 1600 && colorcycle > 1200) {
    return strip.Color(255, 154, 0, 0); //orange
  } else if (colorcycle <= 1200 && colorcycle > 800) {
    return strip.Color(0, 255, 222, 0); //teal
  } else if (colorcycle <= 800 && colorcycle > 400) {
    return strip.Color(255, 6, 223, 0); //purple
  } else if (colorcycle < 400) {
    return strip.Color(167, 0, 255, 0); //blacklight
  }
  else {
    return strip.Color(0, 0, 0, 0);
  }
}
