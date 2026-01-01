#include <SPI.h>
#include <WiFi.h>
#include "Thermostat.h"

#define REQ_TYPE_BUF_LEN 4
#define REQ_URI_BUF_LEN 40
#define MAX_ULONG (0UL - 1UL)

WiFiServer server(80);
Thermostat thermostat(1 ,22 ,23 ,15 ,3000);

String ssid = "";     //  your network SSID (name)
String chosenSSID = "";
String pass = "";  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

String extractValueFromOneLevelJSON(String& json, String&& key) {
  int index = json.indexOf(key);
  if(index == -1) {
    return "\0";
  }
  int valueStartIndex = json.indexOf(":", index);
  int valueEndIndex = json.indexOf(",", valueStartIndex);
  return json.substring(valueStartIndex+1, valueEndIndex);
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  int numSSID = WiFi.scanNetworks();
  Serial.print("discovered: ");
  Serial.println(numSSID);
  int chosenSSIDi = -1;
  for (int thisNet = 0; thisNet < numSSID; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    String ssid = WiFi.SSID(thisNet);
    Serial.print(ssid);
    if(ssid.indexOf(chosenSSID) != -1 && chosenSSIDi == -1) {
      chosenSSIDi = thisNet;
    }
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }

  if(chosenSSIDi != -1) {
    ssid = WiFi.SSID(chosenSSIDi); 
    Serial.println("Found network containing Chosen SSID");
  }

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
  printCurrentNet();
  printWifiData();
  server.begin();
}

void loop() {
  thermostat.updateThermostat();
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    String message = "";
    char c;
    for(uint16_t i = 0; client.connected() && client.available() && i < 0xffff; ++i) {
      c = client.read();
      if(c != '\0') {
        message+=c;
      }
    }
    int reqTypeEndInd = message.indexOf(" ");
    if(reqTypeEndInd == -1) {
      Serial.println("400 - 1");
      sendClientError(client, 400);
      return;
    }
    int reqUriEndInd = message.indexOf(" ", reqTypeEndInd+1);
    if(reqTypeEndInd == -1) {
      Serial.println("400 - 2");
      sendClientError(client, 400);
      return;
    }
    int reqHeadersEndInd = message.indexOf("\n\n", reqUriEndInd);
    if(reqHeadersEndInd == -1) { //could not find end of headers with normal line endings, assume the packet uses crlf line endings
      reqHeadersEndInd = message.indexOf("\n\r\n", reqUriEndInd);
    }
    if(reqTypeEndInd == -1) { //could not find end of headers at all
      Serial.println("400 - 3");
      sendClientError(client, 400);
      return;
    }
    String reqType = message.substring(0, reqTypeEndInd);
    String reqUri = message.substring(reqTypeEndInd+1, reqUriEndInd);
    Serial.println(reqType);
    Serial.println(reqUri);
    Serial.println(message);
    if(reqUri == "/") {
      if(reqType == "GET") {
        String message = thermostat.serialize();
        sendClientPage(client, 200, message);
      } else if(reqType == "PATCH") {
        int reqContentStart = reqHeadersEndInd+2;
        if(message[reqContentStart] == '\n') {
          ++reqContentStart ;
        }
        if(
          extractValueFromOneLevelJSON(message, "temperature") != "\0" ||
          extractValueFromOneLevelJSON(message, "heating") != "\0" ||
          extractValueFromOneLevelJSON(message, "cooling") != "\0" ||
          extractValueFromOneLevelJSON(message, "fanning") != "\0"
        ) {
          sendClientError(client, 409);          
        }
        String eSetpoint = extractValueFromOneLevelJSON(message, "setpoint");
        String eUseHeater = extractValueFromOneLevelJSON(message, "useHeater");
        String eUseCooler = extractValueFromOneLevelJSON(message, "useCooler");
        String eUseFan = extractValueFromOneLevelJSON(message, "useFan");
        String eHCL = extractValueFromOneLevelJSON(message, "hardwareCooldownDuration");

        if(eSetpoint != "\0") {
          thermostat.setpoint = eSetpoint.toFloat();
        }
        if(eUseHeater != "\0") {
          thermostat.useHeater = eUseHeater.toInt();
        }
        if(eUseCooler != "\0") {
          thermostat.useCooler = eUseCooler.toInt();
        }
        if(eUseFan != "\0") {
          thermostat.fanAlwaysOn = eUseFan.toInt();
        }
        if(eHCL != "\0") {
          thermostat.hardwareCooldownLength = eHCL.toInt();
        }
        sendClientError(client, 200);
      } else {
        sendClientError(client, 405);
      }
    } else {
      sendClientError(client, 404);
      return;
    }
  }
}

void sendClientPage(Client& client, int num, String& message) {
  client.println(String("HTTP/1.1 ") + num + String(" ") + getStatusTextForCode(num));
  client.println("Content-Type: application/json");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
  client.println();
  client.print(message);
  client.stop();
  Serial.println("client disconnected");
}

void sendClientError(Client& client, int num) {
  String message = "";
  sendClientPage(client, num, message);  
}

String getStatusTextForCode(int num) {
  switch(num) {
    case 200:
      return "OK";
    case 400:
      return "Bad Request";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    case 409:
      return "Conflict";
    default:
      Serial.println("DEFAULTED FOR STATUS TEXT");
      return "No Status";
  }
}

void printWifiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case 5:
      Serial.println("WEP");
      break;
    case 2:
      Serial.println("WPA");
      break;
    case 4:
    case 3:
      Serial.println("WPA2");
      break;
    case 7:
      Serial.println("None");
      break;
    case 8:
      Serial.println("Auto");
      break;
    default:
      Serial.println(thisType);
  }
}
