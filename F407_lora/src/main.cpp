#include <HardwareSerial.h>
#include <LoRa.h>
#include <SPI.h>

// setup stm32f407vgt6
#define SCK     PB13    // GPIO5  -- SX1278's SCK
#define MISO    PB14   // GPIO19 -- SX1278's MISnO
#define MOSI    PB15   // GPIO27 -- SX1278's MOSI
#define SS      PB12   // GPIO18 -- SX1278's CS
#define RST     PA0   // GPIO14 -- SX1278's RESET
#define DI0     PA1   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  915E6

int counter = 0;
HardwareSerial Serial2(PD6, PD5);
void setup() {
//initialize Serial Monitor
Serial.begin(9600);
Serial2.begin(9600);
Serial2.print("tes");
while (!Serial);
Serial2.println("LoRa Sender");
LoRa.setSPIFrequency(2E6);
pinMode(SS, OUTPUT);
digitalWrite(SS, HIGH);
//setup LoRa transceiver module
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  SPI.setSCLK(SCK);
  SPI.setSSEL(SS);
LoRa.setPins(SS, RST, DI0);

// replace the LoRa.begin(---E-) argument with your location's frequency
// 433E6 for Asia
// 866E6 for Europe
// 915E6 for North America

//LoRa.setFrequency(915E6);
while (!LoRa.begin(915E6)) {
Serial2.println("Lora Failed");
delay(500);
}
// Change sync word (0xF3) to match the receiver
// The sync word assures you don't get LoRa messages from other LoRa transceivers
// ranges from 0-0xFF
// LoRa.setSyncWord(0xF3);
Serial2.println("LoRa Initializing OK!");
}

void loop() {
  Serial2.print("Sending packet: ");
  Serial2.println(counter);
  // send packet
  LoRa.beginPacket();
  LoRa.print("hello");
  LoRa.print(counter);
  LoRa.endPacket();
  counter++;
  delay(1000);
}