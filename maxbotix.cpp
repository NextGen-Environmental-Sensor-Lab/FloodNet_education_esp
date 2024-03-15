// Maxbotix-7389 https://maxbotix.com/pages/hrxl-maxsonar-wr-datasheet
// PINS: 1-temp, 2-pulse width, 3-analog volt, 4-ranging start/stop, 5-serial out, 6-V, 7-GND

#include "ets_sys.h"
#include "core_esp8266_features.h"
#include "maxbotix.h"
#include <SoftwareSerial.h>

SoftwareSerial swSerial(D7, D2);  // note: RX,TX the maxbotix only transmitts so D2 is a dummy

void setup_maxbotix() {
  Serial.println("Setting up Maxbotix .... ");

  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);  //trigger line: now off

  swSerial.begin(9600);  //Initialize software serial with 9600 for maxbotix
}

// MAXBOTIX READ SERIAL ONCE, UNFILTERED
// only read the ascii chars returned by Maxbotix in a string
char *maxbotix_readSerialOnce(char *buffer) {
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(40);
  digitalWrite(triggerPin, LOW);

  uint16_t isNow = millis();
  uint16_t dt = 0;
  while (!swSerial.available() && dt < 3000) {
    //Serial.print(".");
    dt = millis() - isNow;
  };
  if (dt >= 3000) {
    Serial.printf("\ntimeout waiting for maxbotix\n");
    strcpy(buffer, "no data\0");
    return buffer;
  }
  int i = 0;
  while (swSerial.available()) {
    buffer[i++] = char(swSerial.read());
    delay(5);  // slow things a little, 1 is too fast
  }
  buffer[i - 1] = 0;  // terminate string without the cr
  // the string will have an 'R' infront and be 0 terminated
  return buffer;
}

// char *maxbotix_readSerialContinuous(char *buffer) {
//   digitalWrite(triggerPin, HIGH);

//   while (1) {
//     uint16_t isNow = millis();
//     uint16_t dt = 0;
//     while (!swSerial.available() && dt < 5000) {
//       Serial.print(".");
//       delay(100);
//       dt = millis() - isNow;
//     };
//     if (dt >= 5000) {
//       Serial.printf("\ntimeout waiting for maxbotix\n");
//       strcpy(buffer, "no data\0");
//       return buffer;
//     }
//     Serial.println();

//     int i = 0;
//     while (swSerial.available()) {
//       char chtmp = char(swSerial.read());
//       Serial.printf("%02x ",chtmp);
//       if (chtmp == 0x52) {  // if got 'R' get string till CR
//         do {
//           //buffer[i++] = chtmp;
//           //delay(4);
//           chtmp = swSerial.read();
//           while (chtmp == 0xff) {
//             chtmp = swSerial.read();
//           }
//           Serial.printf("%02x ",chtmp);
//         } while (chtmp != 0x0D);  // while not CR
//         //buffer[i] = 0;            // terminate string without the cr
//         Serial.println();
//         break;
//       }
//     } // while 1
//   }
//   return buffer;
// }

// MAXBOTIX READ SERIAL CONTINUOUSLY, FILTERED
// only read the ascii chars returned by Maxbotix in a string
char *maxbotix_readSerialContinuous(char *buffer) {
  digitalWrite(triggerPin, HIGH);

  uint16_t isNow = millis();
  uint16_t dt = 0;
  while (!swSerial.available() && dt < 500) {
    Serial.print(".");
    delay(10);
    dt = millis() - isNow;
  };
  if (dt >= 500) {
    Serial.printf("\ntimeout waiting for maxbotix\n");
    strcpy(buffer, "no data\0");
    return buffer;
  }
  Serial.println();

  int i = 0;
  while(swSerial.available()) {
    buffer[i++]=swSerial.read();
  }
  // while (swSerial.available()) {
  //   char chtmp = char(swSerial.read());
  //   Serial.printf("%02x ",chtmp);

  //   if (chtmp == 0x52) {  // if got 'R' get string till CR
  //     do {
  //       buffer[i++] = chtmp;
  //       //delay(4);
  //       chtmp = swSerial.read();
  //       Serial.printf("%02x ",chtmp);

  //       while (chtmp == 0xff) {
  //         chtmp = swSerial.read();
  //         Serial.printf("%02x ",chtmp);
  //       }
  //     } while (chtmp != 0x0D);  // while not CR
  //     Serial.println();
  //     buffer[i] = 0;            // terminate string without the cr
  //     break;
  //   }
  // }
  buffer[i-2]=0;
  return buffer;
}