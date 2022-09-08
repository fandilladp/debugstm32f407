#include <Arduino.h>
#include <ArduinoJson.h>
#include <SIM800L.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

HardwareSerial monitor(PD6, PD5);

// sim800 setup
static const int SIM800_RX_PIN = PA9, SIM800_TX_PIN = PA10, SIM800_RST_PIN = PA2;
// #define SIM800_RST_PIN 6
const char APN[] = "M2M";
const char URL[] = "http://postman-echo.com/post";
const char CONTENT_TYPE[] = "application/json";
const char PAYLOAD[] = "{\"name\": \"morpheus\", \"job\": \"leader\"}";
SIM800L *sim800l;
static const uint32_t simBaud = 9600;
// SoftwareSerial serial;

// df robot setup
static const int RXPin = PC10, TXPin = PC11;
char databuffer[35];
double temp;

static const uint32_t dfBaud = 9600;
SoftwareSerial df(RXPin, TXPin);
void getBuffer() // Get weather status data
{
  int index;
  for (index = 0; index < 35; index++)
  {
    if (df.available())
    {
      databuffer[index] = df.read();
      if (databuffer[0] != 'c')
      {
        index = -1;
      }
    }
    else
    {
      index--;
    }
  }
}

int transCharToInt(char *_buffer, int _start, int _stop) // char to int）
{
  int _index;
  int result = 0;
  int num = _stop - _start + 1;
  int _temp[num];
  for (_index = _start; _index <= _stop; _index++)
  {
    _temp[_index - _start] = _buffer[_index] - '0';
    result = 10 * result + _temp[_index - _start];
  }
  return result;
}

int WindDirection() // Wind Direction
{
  return transCharToInt(databuffer, 1, 3);
}

float WindSpeedAverage() // air Speed (1 minute)
{
  temp = 0.44704 * transCharToInt(databuffer, 5, 7);
  return temp;
}

float WindSpeedMax() // Max air speed (5 minutes)
{
  temp = 0.44704 * transCharToInt(databuffer, 9, 11);
  return temp;
}

float Temperature() // Temperature ("C")
{
  temp = (transCharToInt(databuffer, 13, 15) - 32.00) * 5.00 / 9.00;
  return temp;
}

float RainfallOneHour() // Rainfall (1 hour)
{
  temp = transCharToInt(databuffer, 17, 19) * 25.40 * 0.01;
  return temp;
}

float RainfallOneDay() // Rainfall (24 hours)
{
  temp = transCharToInt(databuffer, 21, 23) * 25.40 * 0.01;
  return temp;
}

int Humidity() // Humidity
{
  return transCharToInt(databuffer, 25, 26);
}

float BarPressure() // Barometric Pressure
{
  temp = transCharToInt(databuffer, 28, 32);
  return temp / 10.00;
}

void setupModule()
{
  // Wait until the module is ready to accept AT commands
  while (!sim800l->isReady())
  {
    monitor.println(F("Problem to initialize AT command, retry in 1 sec"));
    delay(1000);
  }

  monitor.println(F("Setup Complete!"));

  // Wait for the GSM signal
  uint8_t signal = sim800l->getSignal();
  while (signal <= 0)
  {
    delay(1000);
    signal = sim800l->getSignal();
    monitor.println(signal);
  }
  monitor.print(F("Signal OK (strenght: "));
  monitor.print(signal);
  monitor.println(F(")"));
  delay(1000);

  // Wait for operator network registration (national or roaming network)
  NetworkRegistration network = sim800l->getRegistrationStatus();
  while (network != REGISTERED_HOME && network != REGISTERED_ROAMING)
  {
    delay(1000);
    network = sim800l->getRegistrationStatus();
  }
  monitor.println(F("Network registration OK"));
  delay(1000);

  // Setup APN for GPRS configuration
  bool success = sim800l->setupGPRS(APN);
  while (!success)
  {
    success = sim800l->setupGPRS(APN);
    delay(5000);
  }
  monitor.println(F("GPRS config OK"));
}
void setup()
{
  // put your setup code here, to run once:
  // serial.begin(9600);

  // Initialize Serial Monitor for debugging
  // serial.begin(simBaud);
  // sim800l = new SIM800L((Stream *)&serial, SIM800_RST_PIN, 200, 512);
  df.begin(dfBaud);
  SoftwareSerial *serial = new SoftwareSerial(SIM800_RX_PIN, SIM800_TX_PIN);
  serial->begin(9600);
  delay(1000);

  // Initialize SIM800L driver with an internal buffer of 200 bytes and a reception buffer of 512 bytes, debug disabled
  sim800l = new SIM800L((Stream *)serial, SIM800_RST_PIN, 200, 512);
  monitor.begin(9600);
  setupModule();
}

void loop()
{
  monitor.println("start ....");
  getBuffer(); // Begin!
  StaticJsonDocument<200> doc;
  doc["windDirection"] = WindDirection();
  doc["averageWindSpeed"] = WindSpeedAverage();
  doc["maxWindSpeed"] = WindSpeedMax();
  doc["rainFallOneHour"] = RainfallOneHour();
  doc["rainFallOneHour"] = RainfallOneDay();

  doc["temprature"] = Temperature();
  doc["humidity"] = Humidity();
  doc["barometricPressure"] = BarPressure();
  doc["buoy"] = "";
  doc["rx"] = "";

  serializeJsonPretty(doc, monitor);

  // // Establish GPRS connectivity (5 trials)
  bool connected = false;
  for (uint8_t i = 0; i < 5 && !connected; i++)
  {
    delay(1000);
    connected = sim800l->connectGPRS();
  }

  // Check if connected, if not reset the module and setup the config again
  if (connected)
  {
    monitor.print(F("GPRS connected with IP "));
    monitor.println(sim800l->getIP());
  }
  else
  {
    monitor.println(F("GPRS not connected !"));
    monitor.println(F("Reset the module."));
    sim800l->reset();
    setupModule();
    return;
  }

  monitor.println(F("Start HTTP POST..."));

  // Do HTTP POST communication with 10s for the timeout (read and write)
  uint16_t rc = sim800l->doPost(URL, CONTENT_TYPE, PAYLOAD, 10000, 10000);
  if (rc == 200)
  {
    // Success, output the data received on the monitor
    monitor.print(F("HTTP POST successful ("));
    monitor.print(sim800l->getDataSizeReceived());
    monitor.println(F(" bytes)"));
    monitor.print(F("Received : "));
    monitor.println(sim800l->getDataReceived());
  }
  else
  {
    // Failed...
    monitor.print(F("HTTP POST error "));
    monitor.println(rc);
  }

  delay(1000);
  // put your main code here, to run repeatedly:
}
