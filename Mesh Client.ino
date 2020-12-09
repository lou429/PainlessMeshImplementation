#include <Arduino.h>
#include "painlessMesh.h"
#include <ArduinoOTA.h>

#define LED (2)


// //Define PROGMEM to store variable data on flash 
// #define PROGMEM   ICACHE_RODATA_ATTR
// #define ICACHE_RODATA_ATTR  __attribute__((section(".data.text")))

#define MESH_PREFIX "Smart Home"
#define MESH_PASSWORD "ucpaccess"
#define MESH_PORT 5555

Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

int currentNodeId;
bool pinState = false;
int lightPin = 12;

String nodeName = "Test node";

SimpleList<uint32_t> nodeList;
// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

String LoadHtmlFromFile() {
  Serial.printf("Starting LittleFS\n");
  LittleFS.begin();
  Serial.printf("Reading file...\n");

  File file = LittleFS.open("/website/index.html", "r+");

  if(!file) {
    Serial.printf("Failed to open file\n");
    return "";
  }

  Serial.printf("File opened\n");

  String htmlFile = "";

  while(file.available()) {
    htmlFile += (file.read());
  }

  file.close();
  return htmlFile;
}


void sendMessage()
{
  String msg = "Alive : ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 30, TASK_SECOND * 60));
}

void receivedCallback( uint32_t sender, String &message)
{
  Serial.printf("Received: %s from: %u\n", message.c_str(), sender);
  handleMessage(message, sender);
}

void receivedCallBack(String &from, String &msg) {
    Serial.printf("Message received by node name: %s\n%s", msg.c_str(), from.c_str());
    handleMessage(msg, from); 
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("New Node: %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

//Start the OTA (Over The Air) programming service
void HandleOTA()
{
  ArduinoOTA.onStart([]() {
    Serial.println("\nStart");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void connected(const char *payload, size_t length)
{
  Serial.printf("Connected: %s\n", payload);
}

void disconnect(const char *payload, size_t length)
{
  Serial.printf("disconnect %sn\n", payload);
}

void recievedBroadcast(const char *payload, size_t length)
{
  Serial.printf("Broadcast message: %s", payload);
  handleMessage(payload,-1);
}

void handleMessage(String message, String sender)
{
  handleMessage(message, sender.toInt());
}

void handleMessage(String message, int sender)
{
  // if (message.substring(1, message.length() - 1) == String(currentNodeId) || sender == -1)
  // {
    Serial.printf("Correct node Id");
    switch (message.charAt(0))
    {
      case 's': //Change light state
        triggerLight();
        break;
      case 'a':
        if(mesh.sendBroadcast((String(currentNodeId) + " : " + nodeName)))
          Serial.printf("Info broadcast successful");
        else
          Serial.printf("Info broadcast failed");
        break;
      case 'u': //Reply with state
        //mesh.sendSingle(sender, state ? "o" : "f");
        break;
      default:
        Serial.printf("Cannot handle");
        return;
    }
  //}
}

void triggerLight()
{
  pinState = !pinState;
  digitalWrite(lightPin, (pinState ? HIGH : LOW));
  Serial.printf("Light state:  %s", (pinState ? "On" : "Off"));
}

void setup()
{
  Serial.begin(9600);
  digitalWrite(LED, HIGH);
  Serial.setDebugOutput(true);
  //mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes(ERROR | STARTUP); // set before init() so that you can see startup messages
  currentNodeId = mesh.getNodeId();
  nodeList = mesh.getNodeList();
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  //mesh.setName(nodeName);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask(taskSendMessage);
  pinMode(lightPin, OUTPUT);
  taskSendMessage.enable();
  HandleOTA();
}

void loop()
{
  // it will run the user scheduler as well
  mesh.update();
  HandleOTA();
}
