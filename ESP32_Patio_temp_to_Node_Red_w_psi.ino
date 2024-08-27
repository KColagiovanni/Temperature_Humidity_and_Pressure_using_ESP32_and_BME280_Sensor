/***************************************************************************************************
 * See the README for instructions describing how to set up and run this project.
***************************************************************************************************/
#include <WiFi.h>
#include <BME280I2C.h>
#include <Wire.h>
#include <PubSubClient.h>//For sending data to MQTT Server
#include "time.h"

//*****+*+*+*+*+*+*+*+*+*+**************************************************+*+*+*+*+*+*+*+*+*+*****
//*****+*+*+*+*+*+*+*+*+*+*****************Things to modify*****************+*+*+*+*+*+*+*+*+*+*****
//*****+*+*+*+*+*+*+*+*+*+**************************************************+*+*+*+*+*+*+*+*+*+*****
const char* ssid = "<Your network SSID>"; //********** WiFi SSID **********
const char* password =  "<Your network password>"; //********** SSID Password **********
const char* mqttUser = "<Your MQTT username>";
const char* mqttPassword = "<Your MQTT password>";
const char* mqtt_server = "<Your MQTT server IP Address>";
const int mqttPort = 1883;
const char* mqttClientID = "ESP32Client";
WiFiClient espClient; //********** This is the name of the specific ESP32 **********
PubSubClient client(espClient); //********** This is the name of the specific ESP32 **********
const char* topicPrefix = "topic1/topic2"; // This is specifically for home/room or area name/xxxxx
const char* topicRoom = "topic2";
int delayTime = 59901; // This is adjusted so a cycle happens every 60 seconds(60000 ms)
IPAddress local_IP(<IP Address of the ESP32>); // Can be any IP Address that is available and allowed on your network (comma seperated values).
IPAddress gateway(<IP Address of your gateway>); // Typically the IP Address of your router/modem (comma seperated values).
IPAddress subnet(<IP Address of your subnet>); // Typically 255,255,255,0 (comma seperated values)
IPAddress dns1(75,75,75,75); // dns1 and dns2 are needed to get local time.
IPAddress dns2(75,75,76,76); // dns1 and dns2 are needed to get local time.
//*****+*+*+*+*+*+*+*+*+*+**************************************************+*+*+*+*+*+*+*+*+*+*****
//*****+*+*+*+*+*+*+*+*+*+*************End of things to modify**************+*+*+*+*+*+*+*+*+*+*****
//*****+*+*+*+*+*+*+*+*+*+**************************************************+*+*+*+*+*+*+*+*+*+*****


//**********Declaring Variables**********
int d, mo, y;
int WiFiLed = 2;//**********Pin 2 is the on board blue LED**********
int mqttLed = 5;//**********This is the pin that the LED is connected to()**********
int lightVal = 0;
int resetHour, resetMin;
int resetHTime = 23;
int resetMTime = 59;
long lastMsg = 0;
byte mac[6];
int cycleCounter = 0;
int maxL, s, ssec, m, h;
double avgT, maxT, maxTToday, avgTCalc, avgH, maxH, maxHToday, avgHCalc, avgL, avgLCalc, avgP, maxP, avgPCalc, avgRssi, maxRssi, avgRssiCalc;
double minT = 120;
double minH = 100;
double minP = 1100;
double minRssi = -100;
double minTToday = 120;
double minHToday = 100;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -25200;
const int   daylightOffset_sec = 0;
char msgTopic[100], msgPayload[100];
char cycleTopic[100], cycleString[8];
char wDayTopic[100], wDayString[12];
char dateTopic[100], dateStamp[10];
char timeTopic[100], timeStamp[10];
char tempTopic[100], tempString[8];
char avgTempTopic[100], avgTempString[8];
char maxTempTopic[100], maxTempString[8];
char maxTempTodayTopic[100], maxTempTodayString[8];
char minTempTopic[100], minTempString[8];
char minTempTodayTopic[100], minTempTodayString[8];
char humTopic[100], humString[8];
char avgHumTopic[100], avgHumString[8];
char maxHumTopic[100], maxHumString[8];
char maxHumTodayTopic[100], maxHumTodayString[8];
char minHumTopic[100], minHumString[8];
char minHumTodayTopic[100], minHumTodayString[8];
char rssiTopic[100], rssiString[8];
char avgRssiTopic[100], avgRssiString[8];
char maxRssiTopic[100], maxRssiString[8];
char minRssiTopic[100], minRssiString[8];
char elapsedTimeTopic[100], elapsedTimeString[18];
char lightValTopic[100], lightValString[8];
char avgLightValTopic[100], avgLightValString[8];
char maxLightValTopic[100], maxLightValString[8];
char vBattTopic[100], vBattString[8];
char psiTopic[100], psiString[8];
char avgPsiTopic[100], avgPsiString[8];
char maxPsiTopic[100], maxPsiString[8];
char minPsiTopic[100], minPsiString[8];
char minString[4];
char hourString[4];

BME280I2C bme;

// Checking WiFi status. If status is not connected, try to reconnect until a connection is established.
void checkingWifi()
{
  pinMode(WiFiLed, OUTPUT);
  if (!WiFi.config(local_IP, gateway, subnet, dns1, dns2)) 
  {
    Serial.println("STA Failed to configure");
  }  
  
  while (WiFi.status() != WL_CONNECTED)
  {  
    wifiDisconnected++;
    digitalWrite(WiFiLed, LOW);
    Serial.print("\nConnecting to WiFi");
    WiFi.begin(ssid, password);
    for (int i = 0; i < 20; i++)
    {
      WiFi.begin(ssid, password);
      delay(1000); // Wait 1 second and try again.
      Serial.print(".");
      if(WiFi.status() == WL_CONNECTED)
      {
        break;
      }
    }
    if(WiFi.status() != WL_CONNECTED)
    {
      Serial.print("Having trouble connecting to WiFi, it's been 20 seconds. Trying to connect again");
    }
    if(WiFi.status() == WL_CONNECTED)
    {
      // digitalWrite(WiFiLed, HIGH); (Optional, this turns on the blue on board LED when WiFi is connected.
      Serial.print("\tConnected to ");
      Serial.print(ssid);
      Serial.print(" @ ");
      Serial.println(WiFi.gatewayIP()); 
      Serial.print("ESP32 IP Address is ");
      Serial.println(WiFi.localIP());
      wifiConnected++;
    }
  }
}

// Checking WiFi status. If status is not connected, try to reconnect until a connection is established.
void checkingMqtt()
{  
  pinMode(mqttLed, OUTPUT); // Optional LED to turn on when MQTT is connected.
  client.setServer(mqtt_server, mqttPort);//configure the MQTT server with IPaddress and port.
  while(!client.connected())
  {
    mqttDisconnected++;
    client.connect(mqttClientID, mqttUser, mqttPassword, "apt/patio/lastWill", 0, 1, "I'm dying!!", true);
    Serial.print("Connecting to MQTT");
    for (int i = 0; i < 20; i++)
    {
      delay(1000); // Wait 1 second and try again.
      Serial.print(".");
      client.connect(mqttClientID, mqttUser, mqttPassword, "apt/patio/lastWill", 0, 1, "I'm dying!!", true);
      if(client.connected())
      {
        break;
      }
    }
    if(client.connected())
    {
      Serial.print("\tConnected to MQTT server @ ");
      Serial.println(mqtt_server);
      // digitalWrite(mqttLed, HIGH); // Optional LED to turn on when MQTT is connected.
      mqttConnected++;      
    }
   }
}

// Printing MAC Address to console.
void printEspInfo()
{
  WiFi.macAddress(mac);
  Serial.print("This ESP32's MAC Address is: ");
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}

// Printing the topic prefix (ex. home/room) to console 
void printPrefix()
{   
  const char* specificMsgTopic = "/message";
  strcpy(msgTopic, topicPrefix);
  strcat(msgTopic, specificMsgTopic);
  const char* msgStuff1 = "\n~~apt/";
  const char* msgStuff2 = "~~";
  strcpy(msgPayload, msgStuff1);
  strcat(msgPayload, topicRoom);
  strcat(msgPayload, msgStuff2);
  Serial.println(msgPayload);
}

// Calculating the number of data points that have been taken since the begining.
void printLoopCounter()
{
  loopCounter++;
  const char* specificLoopTopic = "/dataPts";
  strcpy(loopTopic, topicPrefix);
  strcat(loopTopic, specificLoopTopic);
  Serial.print("Total Cycles: ");
  dtostrf(loopCounter, 1, 0, loopString);
  Serial.println(loopString);
}  

// Getting the curent day of the week, the current date, and the current time of day.
void printLocalTime()
{
  const char* specificWDayTopic = "/day";
  strcpy(wDayTopic, topicPrefix);
  strcat(wDayTopic, specificWDayTopic);
  const char* specificDateTopic = "/date";
  strcpy(dateTopic, topicPrefix);
  strcat(dateTopic, specificDateTopic);
  const char* specificTimeTopic = "/time";
  strcpy(timeTopic, topicPrefix);
  strcat(timeTopic, specificTimeTopic);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  char secString[4];
  char minString[4];
  char hourString[4];
  char dayString[4];
  char monthString[4];
  char yearString[6];
  String wDayS;
  const char colon[2] = ":";
  const char space[2] = " ";
  const char dash[2] = "-";
  char zero[2] = "0";
  int s, m, h, d, mo, y;
  s = timeinfo.tm_sec;
  itoa(s, secString, 10);
  m = timeinfo.tm_min;
  itoa(m, minString, 10);
  h = timeinfo.tm_hour;
  itoa(h, hourString, 10);
  d = timeinfo.tm_mday;
  itoa(d, dayString, 10);
  mo = timeinfo.tm_mon;
  mo += 1;
  itoa(mo, monthString, 10);
  y = timeinfo.tm_year;
  y  += 1900;
  itoa(y, yearString, 10);
  strcpy(dateStamp, yearString);  
//  strcat(dateStamp, dash);
  if(mo < 10)
  {
    strcat(dateStamp, "0");    
  }
  strcat(dateStamp, monthString);
//  strcat(dateStamp, dash);
  if(d < 10)
  {
    strcat(dateStamp, "0");
  }
  strcat(dateStamp, dayString);  
//  strcat(dateStamp, space);
  if(h < 10)
  {
    strcpy(timeStamp, zero);
    strcat(timeStamp, hourString);
  }
  else
  {
    strcpy(timeStamp, hourString);
  }
//  strcat(timeStamp, colon);
  if(m < 10)
  {
    strcat(timeStamp, zero);
    strcat(timeStamp, minString);
  }
  else
  {
    strcat(timeStamp, minString);
  }
//  strcat(timeStamp, colon);
  if(s < 10)
  {
    strcat(timeStamp, zero);
    strcat(timeStamp, secString);
  }
  else
  {
    strcat(timeStamp, secString);
  }
  int wDay, yDay, isdst;
  wDay = timeinfo.tm_wday;
  switch(wDay)
  {
    case 0:
      wDayS = "'Sunday'";
      break;
    case 1:
      wDayS = "'Monday'";
      break;
    case 2:
      wDayS = "'Tuesday'";
      break;
    case 3:
      wDayS = "'Wednesday'";
      break;
    case 4:
      wDayS = "'Thursday'";
      break;
    case 5:
      wDayS = "'Friday'";
      break;
    case 6:
      wDayS = "'Saturday'";
      break;
  }
  Serial.print("Day: ");
  int len = wDayS.length() + 1;
  wDayS.toCharArray(wDayString, len);// Convert the value to a char array
  Serial.println(wDayString);
  Serial.print("Date: ");
  Serial.println(dateStamp);
  Serial.print("Time: ");
  Serial.println(timeStamp);
}

// Reading the temperature from the sensor, then returning it in Farenheit.
double printTemp()
{
  const char* specificTempTopic = "/temp";
  strcpy(tempTopic, topicPrefix);
  strcat(tempTopic, specificTempTopic);
  float temper(NAN), humi(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(pres, temper, humi, tempUnit, presUnit);
  double f = temper;
  if(isnan(f))
  {
    Serial.println("Temp Sensor not detected");
  }
  if (!isnan(f))
  {
    dtostrf(f, 1, 2, tempString);// Convert the value to a char array
    Serial.print("Temperature: ");
    Serial.print(tempString);//Print the value to the serial port
    Serial.print("*F\t\t");
    return f;
  }
  else
  {
    return avgT;
  }
}

// Calaulating the average temperature, in Farenheit.
void avgTemp(double f)
{
  if (!isnan(f) || tempString == "nan")
  {
    const char* specificTempTopic = "/avgTemp";
    strcpy(avgTempTopic, topicPrefix);
    strcat(avgTempTopic, specificTempTopic);
    avgTCalc = avgTCalc + f;
    if (loopCounter == 1)
    {
      avgT = f;
      avgTCalc = f;     
    }
    else
    {
      avgT = avgTCalc / loopCounter;
    }
    dtostrf(avgT, 1, 2, avgTempString);// Convert the value to a char array
    Serial.print("Avg Temp is: ");
    Serial.print(avgTempString);
    Serial.print("*F\t\t");
  }
}

// Calculating the max temperature, in Farenheit.
void calcMaxTemp(double f)
{
  const char* specificTempTopic = "/maxTemp";
  strcpy(maxTempTopic, topicPrefix);
  strcat(maxTempTopic, specificTempTopic);
  maxT = max(maxT, f);
  dtostrf(maxT, 1, 2, maxTempString);// Convert the value to a char array
  Serial.print("Max Temp is: ");
  Serial.print(maxTempString);
  Serial.print("*F\t\t");
}

// Calculating the the max temperature, in Farenheit, of the current day.
void calcMaxTempToday(double f)
{
  const char* specificTempTopic = "/maxTempToday";
  strcpy(maxTempTodayTopic, topicPrefix);
  strcat(maxTempTodayTopic, specificTempTopic);
  if(resetHour == resetHTime && resetMin == resetMTime)
  {
    maxTToday = 0;
  }
  maxTToday = max(maxTToday, f);
  dtostrf(maxTToday, 1, 2, maxTempTodayString);// Convert the value to a char array
  Serial.print("Max Temp Today is: ");
  Serial.print(maxTempTodayString);
  Serial.print("*F\t");
}

// Calculating the min temperature, in Farenheit.
void calcMinTemp(double f)
{
  if(f != 0)
  {
    const char* specificTempTopic = "/minTemp";
    strcpy(minTempTopic, topicPrefix);
    strcat(minTempTopic, specificTempTopic);
    minT = min(minT, f);
    dtostrf(minT, 1, 2, minTempString);// Convert the value to a char array
    Serial.print("Min Temp is: ");
    Serial.print(minTempString);
    Serial.print("*F\t\t");
  }
}

// Calculating the the min temperature, in Farenheit, of the current day.
void calcMinTempToday(double f)
{
  const char* specificTempTopic = "/minTempToday";
  strcpy(minTempTodayTopic, topicPrefix);
  strcat(minTempTodayTopic, specificTempTopic);
  if(resetHour == resetHTime && resetMin == resetMTime)
  {
    minTToday = 120;
  }
  minTToday = min(minTToday, f);
  dtostrf(minTToday, 1, 2, minTempTodayString);// Convert the value to a char array
  Serial.print("Min Temp Today is: ");
  Serial.print(minTempTodayString);
  Serial.print("*F\t");
}

// Reading the humidity percent from the sensor.
double printHum()
{
  const char* specificHumTopic = "/hum";
  strcpy(humTopic, topicPrefix);
  strcat(humTopic, specificHumTopic);
  float temper(NAN), humi(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(pres, temper, humi, tempUnit, presUnit);
  double h = humi;
  if (!isnan(h))
  {
    dtostrf(h, 1, 2, humString);// Convert the value to a char array
    Serial.print("Humidity: ");
    Serial.print(humString);//Print the value to the serial port
    Serial.print("%\t\t");
    return h;
  }
  else
  {
    return avgH;
  }
}

// Calculating the average humidity percent.
void avgHum(double h)
{
  if (!isnan(h) || humString == "nan")
  {
    const char* specificHumTopic = "/avgHum";
    strcpy(avgHumTopic, topicPrefix);
    strcat(avgHumTopic, specificHumTopic);
    avgHCalc = avgHCalc + h;
    if (loopCounter == 1)
    {
      avgH = h;
      avgHCalc = h;     
    }
    else
    {
      avgH = avgHCalc / loopCounter;
    }
    dtostrf(avgH, 1, 2, avgHumString);// Convert the value to a char array
    Serial.print("Avg Hum is: ");
    Serial.print(avgH);
    Serial.println("%");
  }
}

// Calculating the max humidity percent.
void calcMaxHum(double h)
{
  const char* specificHumTopic = "/maxHum";
  strcpy(maxHumTopic, topicPrefix);
  strcat(maxHumTopic, specificHumTopic);
  maxH = max(maxH, h);
  dtostrf(maxH, 1, 2, maxHumString);// Convert the value to a char array
  Serial.print("Max Hum is: ");
  Serial.print(maxH);
  Serial.println("%");
}

// Calculating the the max humidity percent of the current day.
void calcMaxHumToday(double h)
{
  const char* specificHumTopic = "/maxHumToday";
  strcpy(maxHumTodayTopic, topicPrefix);
  strcat(maxHumTodayTopic, specificHumTopic);
  if(resetHour == resetHTime && resetMin == resetMTime)
  {
    maxHToday = 0;
  }
  maxHToday = max(maxHToday, h);
  dtostrf(maxHToday, 1, 2, maxHumTodayString);// Convert the value to a char array
  Serial.print("Max Hum Today is: ");
  Serial.print(maxHumTodayString);
  Serial.println("%");
}

// Calculating the the min humidity percent.
void calcMinHum(double h)
{
  const char* specificHumTopic = "/minHum";
  strcpy(minHumTopic, topicPrefix);
  strcat(minHumTopic, specificHumTopic);
  minH = min(minH, h);
  dtostrf(minH, 1, 2, minHumString);// Convert the value to a char array
  Serial.print("Min Hum is: ");
  Serial.print(minH);
  Serial.println("%");
}

// Calculating the the max humidity percent of the current day.
void calcMinHumToday(double h)
{
  const char* specificHumTopic = "/minHumToday";
  strcpy(minHumTodayTopic, topicPrefix);
  strcat(minHumTodayTopic, specificHumTopic);
  if(resetHour == resetHTime && resetMin == resetMTime)
  {
    minHToday = 100;
  }
  minHToday = min(minHToday, h);
  dtostrf(minHToday, 1, 2, minHumTodayString);// Convert the value to a char array
  Serial.print("Min Hum Today is: ");
  Serial.print(minHumTodayString);
  Serial.println("%");
}

// Reading the pressure from the sensor, then returning it in hPa.
double printPsi()
{
  const char* specificPsiTopic = "/psi";
  strcpy(psiTopic, topicPrefix);
  strcat(psiTopic, specificPsiTopic);
  float temper(NAN), humi(NAN), pres(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);
  bme.read(pres, temper, humi, tempUnit, presUnit);
  double p = pres;
  if(isnan(p))
  {
    Serial.println("Pressure Sensor not detected");
  }
  if (!isnan(p))
  {
    dtostrf(p, 1, 2, psiString);// Convert the value to a char array
    Serial.print("Pressure: ");
    Serial.print(psiString);//Print the value to the serial port
    Serial.println("hPa");
    return p;
  }
  else
  {
    return avgP;
  }
}

// Calculating the Avg Pressure, in hPa.
void avgPsi(double p)
{
  if (!isnan(p) || psiString == "nan")
  {
    const char* specificPsiTopic = "/avgPsi";
    strcpy(avgPsiTopic, topicPrefix);
    strcat(avgPsiTopic, specificPsiTopic);
    avgPCalc = avgPCalc + p;
    if (cycleCounter == 1)
    {
      avgP = p;
      avgPCalc = p;     
    }
    else
    {
      avgP = avgPCalc / cycleCounter;
    }
    dtostrf(avgP, 1, 2, avgPsiString);// Convert the value to a char array
    Serial.print("Avg Pressure is: ");
    Serial.print(avgPsiString);
    Serial.println("hPa");
  }
}

// Calculating the max pressure, in hPa.
void calcMaxPsi(double p)
{
  const char* specificPsiTopic = "/maxPsi";
  strcpy(maxPsiTopic, topicPrefix);
  strcat(maxPsiTopic, specificPsiTopic);
  maxP = max(maxP, p);
  dtostrf(maxP, 1, 2, maxPsiString);// Convert the value to a char array
  Serial.print("Max Pressure is: ");
  Serial.print(maxPsiString);
  Serial.println("hPa");
}

// Calculating the min pressure, in hPa.
void calcMinPsi(double p)
{
  if(p != 0)
  {
    const char* specificPsiTopic = "/minPsi";
    strcpy(minPsiTopic, topicPrefix);
    strcat(minPsiTopic, specificPsiTopic);
    minP = min(minP, p);
    dtostrf(minP, 1, 2, minPsiString);// Convert the value to a char array
    Serial.print("Min Pressure is: ");
    Serial.print(minPsiString);
    Serial.println("hPa");
  }
}

// Reading the current RSSI.
double printRssi()
{
  const char* specificRssiTopic = "/rssi";
  strcpy(rssiTopic, topicPrefix);
  strcat(rssiTopic, specificRssiTopic);
  double rssi = WiFi.RSSI();
  dtostrf(rssi, 1, 0, rssiString);// Convert the value to a char array
  Serial.print("RSSI: ");
  Serial.print(rssiString);
  Serial.println("dBm");
  return rssi;
}

// Calculating the average RSSI.
void calcAvgRssi(int rssi)
{
  if (!isnan(rssi) || rssiString == "nan")
  {
    const char* specificAvgRssiTopic = "/avgRssi";
    strcpy(avgRssiTopic, topicPrefix);
    strcat(avgRssiTopic, specificAvgRssiTopic);
    avgRssiCalc = avgRssiCalc + rssi;
    if (loopCounter == 1)
    {
      avgRssi = rssi;
      avgRssiCalc = rssi;     
    }
    else
    {
      avgRssi = avgRssiCalc / loopCounter;
    }
    dtostrf(avgRssi, 1, 2, avgRssiString);// Convert the value to a char array
    Serial.print("Avg RSSI is: ");
    Serial.print(avgRssiString);
    Serial.println("dBm");
  }
}

// Calculating the max RSSI.
void calcMaxRssi(double rssi)
{
  const char* specificMaxRssiTopic = "/maxRssi";
  strcpy(maxRssiTopic, topicPrefix);
  strcat(maxRssiTopic, specificMaxRssiTopic);
  maxRssi = min(maxRssi, rssi);
  dtostrf(maxRssi, 1, 0, maxRssiString);// Convert the value to a char array
  Serial.print("Max RSSI is: ");
  Serial.print(maxRssiString);
  Serial.println("dBm");  
}

// Calculating the the min RSSI.
void calcMinRssi(double rssi)
{
  if(rssi != 0)
  {
    const char* specificMinRssiTopic = "/minRssi";
    strcpy(minRssiTopic, topicPrefix);
    strcat(minRssiTopic, specificMinRssiTopic);
    minRssi = max(minRssi, rssi);
    dtostrf(minRssi, 1, 0, minRssiString);// Convert the value to a char array
    Serial.print("Min RSSI is: ");
    Serial.print(minRssiString);
    Serial.println("dBm");
  }
}

// Calculating the elapsed time since data started to be taken.
void printElapsedTime()
{
  s = millis();
  s = s / 1000;
  second = s;
  if(s > 59)
  {
    second = fmod(s, 60);//Use fmod on doubles instead of modulus(%) (modulus only works with int)
  }
  if(s > 59)
  {
    m = s / 60;
    minute = m;
  }
  if(minute > 59)
  {
    minute = minute % 60;
  }
  if(m > 59)
  {
    hour = m / 60;
  }
  String elapsedTime;
  String ss;
  String mm;
  String lz = "0";
  if(second < 10)
  {
    ss.concat(lz);
  }
  else
  {
    ss = "";
  }
  if(minute < 10)
  {
    mm.concat(lz);
  }
  else
  {
    mm = "";
  }
  int len;
  const char* specificElapsedTimeTopic = "/elapsedTime";
  strcpy(elapsedTimeTopic, topicPrefix);
  strcat(elapsedTimeTopic, specificElapsedTimeTopic);
  elapsedTime = hour + mm + minute + ss + second;
  Serial.print("Elapsed Time: ");
  len = elapsedTime.length() + 1;
  elapsedTime.toCharArray(elapsedTimeString, len);// Convert the value to a char array
  Serial.println(elapsedTimeString);
}

// Publishing data to the MQTT server.
void publishToMqtt()
{
  client.publish(msgTopic, msgPayload);
  client.publish(cycleTopic, cycleString);
  client.publish(tempTopic, tempString);
  client.publish(avgTempTopic, avgTempString);
  client.publish(maxTempTopic, maxTempString);
  client.publish(maxTempTodayTopic, maxTempTodayString);
  client.publish(minTempTopic, minTempString);
  client.publish(minTempTodayTopic, minTempTodayString);
  client.publish(humTopic, humString);
  client.publish(avgHumTopic, avgHumString);
  client.publish(maxHumTopic, maxHumString);
  client.publish(maxHumTodayTopic, maxHumTodayString);
  client.publish(minHumTopic, minHumString);
  client.publish(minHumTodayTopic, minHumTodayString);
  client.publish(psiTopic, psiString);
  client.publish(avgPsiTopic, avgPsiString);
  client.publish(maxPsiTopic, maxPsiString);
  client.publish(minPsiTopic, minPsiString);
  client.publish(rssiTopic, rssiString);
  client.publish(avgRssiTopic, avgRssiString);
  client.publish(maxRssiTopic, maxRssiString);
  client.publish(minRssiTopic, minRssiString);
  client.publish(wDayTopic, wDayString);
  client.publish(dateTopic, dateStamp);
  client.publish(timeTopic, timeStamp);
  client.publish(elapsedTimeTopic, elapsedTimeString);
  client.loop();  //Running the PubSub Loop
}

// Reading the current MQTT Server State
void printState()
{
  Serial.print("The current state of the MQTT Client is: ");
  Serial.print(client.state());
  switch (client.state())
  {
    case -4: 
      Serial.println(" MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time");
      break;
    case -3:
      Serial.println(" MQTT_CONNECTION_LOST - the network connection was broken");
      break;
    case -2:
      Serial.println(" MQTT_CONNECT_FAILED - the network connection failed");
      break;
    case -1:
      Serial.println(" MQTT_DISCONNECTED - the client is disconnected cleanly");
      break;
    case 0:
      Serial.println(" MQTT_CONNECTED - the client is connected");
      break;
    case 1:
      Serial.println(" MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT");
      break;
    case 2:
      Serial.println(" MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier");
      break;
    case 3:
      Serial.println(" MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection");
      break;
    case 4:
      Serial.println(" MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected");
      break;
    case 5:
      Serial.println(" MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect");
      break;
  }
}

// Keeping track of how many times wifiConnected has happened.
void printWifiConnected()
{
  const char* specificWifiConTopic = "/wifiCon";
  strcpy(wifiConTopic, topicPrefix);
  strcat(wifiConTopic, specificWifiConTopic);
  Serial.print("WiFi has Conected to the server ");
  dtostrf(wifiConnected, 1, 0, wifiConnectedString);// Convert the value to a char array
  Serial.print(wifiConnectedString);
  if(wifiConnected == 1)
  {
    Serial.println(" time");
  }
  else
  {
    Serial.println(" times");
  }
}

// Keeping track of how many times wifiDisconnected has happened.
void printWifiDisconnected()
{
  const char* specificWifiDisTopic = "/wifiDiscon";
  strcpy(wifiDisTopic, topicPrefix);
  strcat(wifiDisTopic, specificWifiDisTopic);
  Serial.print("Wifi has disconnected from the server ");
  dtostrf(wifiDisconnected, 1, 0, wifiDisconnectedString);// Convert the value to a char array
  Serial.print(wifiDisconnectedString);
  if(wifiDisconnected == 1)
  {
    Serial.println(" time");
  }
  else
  {
    Serial.println(" times");
  }
}

// Keeping track of how many times mqttConnected has happened.
void printMqttConnected()
{
  const char* specificMqttConTopic = "/mqttCon";
  strcpy(mqttConTopic, topicPrefix);
  strcat(mqttConTopic, specificMqttConTopic);
  Serial.print("MQTT has Conected to the server ");
  dtostrf(mqttConnected, 1, 0, mqttConnectedString);// Convert the value to a char array
  Serial.print(mqttConnectedString);
  if(mqttConnected == 1)
  {
    Serial.println(" time");
  }
  else
  {
    Serial.println(" times");
  }
}

// Keeping track of how many times mqttDisconnected has happened.
void printMqttDisconnected()
{
  const char* specificMqttDisTopic = "/mqttDiscon";
  strcpy(mqttDisTopic, topicPrefix);
  strcat(mqttDisTopic, specificMqttDisTopic);
  Serial.print("MQTT has disconnected from the server ");
  dtostrf(mqttDisconnected, 1, 0, mqttDisconnectedString);// Convert the value to a char array
  Serial.print(mqttDisconnectedString);
  if(mqttDisconnected == 1)
  {
    Serial.println(" time");
  }
  else
  {
    Serial.println(" times");
  }
}

// The setup function, start serial, htu temp/humidity/pressure sensor, and print the device MAC address.
void setup(){
  Serial.begin(115200);
  Wire.begin();
  bme.begin();
  printEspInfo();
}

// The loop function checks WiFi connection, MQTT connection, and calls the temp, humidity, pressure, RSSI, some time realated functions, and a loop counter.
void loop() {
  checkingWifi();
  checkingMqtt();
  printPrefix();
  printLoopCounter();
  double f = printTemp();
  double h = printHum();
  double p = printPsi();
  if (!isnan(f) || !isnan(h) || !isnan(p))
  {
    avgTemp(f);
    avgHum(h);
    avgPsi(p);
    calcMaxTemp(f);
    calcMaxHum(h);
    calcMaxPsi(p);
    calcMaxTempToday(f);
    calcMaxHumToday(h);
    calcMinTemp(f);
    calcMinHum(h); 
    calcMinPsi(p); 
    calcMinTempToday(f);
    calcMinHumToday(h);
  }
  double rssi = printRssi();
  calcAvgRssi(rssi);
  calcMaxRssi(rssi);
  calcMinRssi(rssi);
  printElapsedTime();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  printWifiConnected();
  printWifiDisconnected();
  printMqttConnected();
  printMqttDisconnected();
  publishToMqtt();
  printState();
  delay(delayTime);
}
