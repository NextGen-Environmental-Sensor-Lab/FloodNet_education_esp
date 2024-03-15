
#include <Adafruit_NeoPixel.h>
#include "maxbotix.h"
#include <SoftwareSerial.h>
#include "Provisioning.h"

#define PIN D6         // on Wemos d1 mini
#define NUMPIXELS 144  // for 1000 mm strip -

extern SoftwareSerial swSerial;
extern settings_t settings;
extern HTTPSRedirect* client;
extern char* host;
extern String url;
extern String payload_prefix;
extern String payload_suffix;
extern String payload;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
float mmPerpix = 1000 / (1.0 * NUMPIXELS);
int m, floorDist;

void setup() {

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(10000);
  Serial.println(__FILE__);

  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)

  Serial.println("\nFloodNet edu sensor to Google Sheets via wifi.");
  Serial.println("Solid light = Starting");
  Serial.println("Fast blink = 15s to go into Provisioning by joining wifi AP 'csl-clearEEPROMxxx'");
  Serial.println("Slow blink = Provisioning. Join wifi 'csl_espxxx', open webpage at 192.168.4.1");
  Serial.println("Off = Connected to wifi\n");

  clearSettings();
  connectToWifi();

  m = 0;
  setup_maxbotix();
  // get dist to floor. Resets every time
  floorDist = dist(10);  // 10 readings averaged

  url = String("/macros/s/") + String(settings.saved_gsid) + "/exec?cal";

  delete client;
  client = nullptr;
  clientConnect();
  delay(1000);
  while (swSerial.available() > 0) swSerial.read();  // clean out the serial
}

void loop() {
  static int error_count = 0;
  static uint32_t loopCount =0;
  
  static char dataArray[16] = "";
  static char postArray[32] = "";

  // int mdist = dist(1);        // take one reading and return as int
  //maxbotix_readSerialOnce(dataArray);
  maxbotix_readSerialContinuous(dataArray);
  int mdist = atoi(dataArray+1);
  int mdepth = depth(mdist);  // returns depth by subtract from foorDist
  int pdepth = pixDepth(mdist); //

  // turn pixels on from far end to wired end
  if (pdepth > m) {
    for (int i = m; i <= pdepth; i++) {  // For each pixel...
      pixels.setPixelColor(NUMPIXELS - i, pixels.Color(32, 32, 16));
      pixels.show();  // Send the updated pixel colors to the hardware.
    }
  } else {
    for (int i = m; i >= pdepth; i--) {  // For each pixel...
      pixels.setPixelColor(NUMPIXELS - i, pixels.Color(0, 0, 0));
      pixels.show();  // Send the updated pixel colors to the hardware.
    }
  }
  m = pdepth;

  //  Serial.println("POST append data to spreadsheet:");
  char payload_main[64];
  sprintf(payload_main, "%s,%d,%d,%d,,%s", dataArray, mdist, mdepth, pdepth, postArray);
  //Serial.println(payload_main);
  payload = payload_prefix + String(payload_main) + payload_suffix;

  itoa(loopCount, postArray, 10);
  Serial.printf("url: %s, host: %s, payload: %s\n", url.c_str(), host, payload.c_str());

  if (!client->POST(url, host, payload)) {  // POST to gscript
    Serial.printf("Error-count while connecting: %u\n", ++error_count);
  }

  if (error_count > 3) {  // after 3 errors reconnect
    strcpy(postArray, ",,reconnected\0");
    Serial.println("Reconnecting to host");
    delete client;
    client = nullptr;
    if (!clientConnect()) {  // if can't connect goto sleep
      Serial.printf("Going to reboot\n");
      delay(5000);
      ESP.restart();
      //ESP.deepSleep(0);
    } else {
      error_count = 0;
      strcpy(postArray, "");
    }
  }
  loopCount++;
  delay(10);
}

// Do readSerialOnce and retun unfiltered reading as int
int dist(int n) {
  char charArray[32];
  int acc = 0;
  for (int j = 0; j < n; j++) {
    maxbotix_readSerialOnce(charArray);
    acc += atoi(charArray + 1); // the +1 is to skip the 'R'
  }
  acc = (int)(1.0 * acc / n);
  return acc;
}

// 
int depth(int d) {
  return floorDist - d;
}

//
int pixDepth(int d) {
  float ret = (floorDist - d) / mmPerpix;
  // Serial.printf("dist: %d, floorDist: %d, mmPerpix: %f, ret: %f int ret: %d\n", atoi(charArray + 1), floorDist, mmPerpix, ret, (int)ret);
  if ((int)ret < 0)
    return 0;
  else
    return (int)ret;
}
