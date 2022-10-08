#include <Arduino.h>
#line 1 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
/*
  Copyright statement: This article is the original article of the CSDN blogger "Naisu_kun",
  which follows the CC 4.0 BY-SA copyright agreement.
  For reprinting, please attach the original source link and this statement.
  Original link：https://blog.csdn.net/Naisu_kun/article/details/88572129
  - Explanation of the code and other parameters: see the link above.
  - Note: Original language: Chinese (You can use the Google translator to read English, or another language)

  Copyright 2021 Dirk Luberth Dijkman Bangert 30 1619GJ Andijk The Netherlands
  https://github.com/ldijkman/WT32-ETH01-LAN-8720-RJ45-
  Copyright 2022 Bogdan Maksimovic, Niksic, Montenegro
  https://github.com/montemadjo/maximvs-plc
  GNU General Public License,
  which basically means that you may freely copy, change, and distribute it,
  but you may not impose any restrictions on further distribution,
  and you must make the source code available.
  All above must be included in any redistribution
*/

// Zoran dodao komentar

// Bogdan dodao komentar

// Zoranova grana

#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>

#define ETH_ADDR 1
#define ETH_POWER_PIN 16
#define ETH_POWER_PIN_ALTERNATIVE 16
#define ETH_MDC_PIN 23
#define ETH_MDIO_PIN 18
#define ETH_TYPE ETH_PHY_LAN8720
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT

IPAddress local_ip(192, 168, 1, 112);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);
IPAddress dns2 = (uint32_t)0x00000000;

const byte NUMBER_OF_NAMES = 8;
char *names[NUMBER_OF_NAMES] = {"shonje", "bezveznjakovici", "digitroni", "glupsoni", "tupsoni", "praistorija", "neznalice", "nishta roba"};

// TODO: BM - explore OOP possibilites in arduino.

volatile uint32_t pulseTime1, pulseTime2;
volatile uint32_t card1, card2;

// Define Maximus board terminal inputs
#define WG1_D0 35 // J25
#define WG1_D1 14 // J24
#define WG1_LED 12 // J26
#define WG2_D0 36 // J19
#define WG2_D1 39 // J18
#define WG2_LED 15 // J20
#define OUT1 2 //J31
#define OUT2 4 //J30

// Define interrupt service routines
#line 122 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void handleRoot();
#line 127 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void handleAjax();
#line 138 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void WiFiEvent(system_event_id_t event);
#line 177 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void setup();
#line 219 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void loop();
#line 63 "/home/zoran/A/evirad sistemi/WT32/maximvs-plc/Boards/simple/simple.ino"
void IRAM_ATTR isr_WG1_D0() {
  if(digitalRead(WG1_D0) || digitalRead(WG1_D0)) return; // filter koji eliminiše kratke smetnje
  card1 <<=1;
  pulseTime1=millis();
}

void IRAM_ATTR isr_WG1_D1() {
  if(digitalRead(WG1_D1) || digitalRead(WG1_D1)) return;
  card1 <<= 1; ++card1;
  pulseTime1=millis();
}

void IRAM_ATTR isr_WG2_D0() {
  if(digitalRead(WG2_D0) || digitalRead(WG2_D0)) return;
  card2 <<=1;
  pulseTime2=millis();
}
void IRAM_ATTR isr_WG2_D1() {
  if(digitalRead(WG2_D1) || digitalRead(WG2_D1)) return;
  card2 <<=1; ++card2;
  pulseTime2=millis();
}


// Web page
String myhtmlPage =
    String("") + "\r\n" +
    "<html>" + "\r\n" +
    "<head>" + "\r\n" +
    "    <title>Maximus WebServer Test</title>" + "\r\n" +
    "    <script>" + "\r\n" +
    "        function getData() {" + "\r\n" +
    "            var xmlhttp;" + "\r\n" +
    "            if (window.XMLHttpRequest) {" + "\r\n" +
    "                xmlhttp = new XMLHttpRequest();" + "\r\n" +
    "            }" + "\r\n" +
    "            else {" + "\r\n" +
    "                xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");" + "\r\n" +
    "            }" + "\r\n" +
    "            xmlhttp.onreadystatechange = function() {" + "\r\n" +
    "                if (this.readyState == 4 && this.status == 200) {" + "\r\n" +
    "                    document.getElementById(\"txtRandomData\").innerHTML = this.responseText;" + "\r\n" +
    "                }" + "\r\n" +
    "            };" + "\r\n" +
    "            xmlhttp.open(\"GET\", \"getRandomData\", true); " + "\r\n" +
    "            xmlhttp.send();" + "\r\n" +
    "        }" + "\r\n" +
    "    </script>" + "\r\n" +
    "</head>" + "\r\n" +
    "<body>" + "\r\n" +
    "    <div>Maximus PLC je najjaci PLC!</div>" + "\r\n" +
    "    <div>Svi ostali PLC-ovi su za njega</div>" + "\r\n" +
    "    <div><p id=\"txtRandomData\"><b> </b></p></div>" + "\r\n" +
    "    <input type=\"button\" value=\"random\" onclick=\"getData()\">" + "\r\n" +
    "</body>" + "\r\n" +
    "</html>";

WebServer server(80);

void handleRoot() // Callback
{
    server.send(200, "text/html", myhtmlPage); //!!! Note that returning to the web page requires "text / html" !!!
}

void handleAjax() // Callback
{
    String message = "<b>- ";
    message += String(names[random(0, NUMBER_OF_NAMES)]); // Get random number  // could do millis(); also
    message += " ^ " + String(card1);
    message += "</b>";
    server.send(200, "text/plain", message); // Send message back to page
}

static bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event) // strange WiFiEvent? we are wired?
{
    switch (event)
    {
    case SYSTEM_EVENT_ETH_START:
        Serial.println("ETH Started");
        // set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case SYSTEM_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex())
        {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        eth_connected = true;
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        eth_connected = false;
        break;
    case SYSTEM_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        eth_connected = false;
        break;
    default:
        break;
    }
}

void setup()
{
    pinMode(ETH_POWER_PIN_ALTERNATIVE, OUTPUT);
    digitalWrite(ETH_POWER_PIN_ALTERNATIVE, HIGH);

    Serial.begin(115200);
    delay(2500); // little delay otherwise next printline is not done, because serial begin is not completed or something alike
    Serial.println("hello");

    WiFi.onEvent(WiFiEvent); // WiFi ????

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_MDC_PIN, ETH_MDIO_PIN, ETH_TYPE, ETH_CLK_MODE); // Enable ETH

    // ETH.config(local_ip, gateway, subnet, dns1, dns2); // Static IP, leave without this line to get IP via DHCP

    while (!((uint32_t)ETH.localIP()))
    {
    }; // Waiting for IP (leave this line group to get IP via DHCP)

    server.on("/", handleRoot);                        // Register link and callback function
    server.on("/getRandomData", HTTP_GET, handleAjax); // Request and callback function of the get method sent by ajax in the registration web page

    server.begin(); // Start server
    Serial.println("Web server started");

    pinMode(WG1_D0, INPUT);
    pinMode(WG1_D1, INPUT);
    pinMode(WG1_LED, OUTPUT); digitalWrite(WG1_LED, HIGH); // nula je aktivna
    pinMode(WG2_D0, INPUT);
    pinMode(WG2_D1, INPUT);
    pinMode(WG2_LED, OUTPUT); digitalWrite(WG2_LED, HIGH);
    
    pinMode(OUT1, OUTPUT); digitalWrite(OUT1, LOW); // jedinica je aktivna
    pinMode(OUT2, OUTPUT); digitalWrite(OUT2, LOW);
    
    // Attach interrupt for all 4 inputs on the board.
    attachInterrupt(WG1_D0, isr_WG1_D0, FALLING);
    attachInterrupt(WG1_D1, isr_WG1_D1, FALLING);
    attachInterrupt(WG2_D0, isr_WG2_D0, FALLING);
    attachInterrupt(WG2_D1, isr_WG2_D1, FALLING);
}

void loop()
{
    server.handleClient(); // Handling web requests from clients
    if(card1 && pulseTime1 && millis() > pulseTime1 + 10 ) {
      Serial.print("CARD1="); Serial.print(card1, HEX); Serial.println(""); card1=0;
    }
    if(card2 && pulseTime2 && millis() > pulseTime2 + 10 ) {
      Serial.print("CARD2="); Serial.print(card2, HEX); Serial.println(""); card2=0;
    }
}

