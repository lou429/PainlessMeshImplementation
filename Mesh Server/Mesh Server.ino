#include <painlessMesh.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <FS.h>

#define LED (2)

#define   STATION_SSID    "Mesh controller"
#define   STATION_PWD     "xxxxxxx"

#define   MESH_SSID       "Smart Home"
#define   MESH_PASSWORD   "ucpaccess"
#define   MESH_PORT       5555

bool pinState = false;
int lightPin = 12;
int node_count = 0;

ESP8266WebServer server(80);

// Prototypes
void sendMessage(); 
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);
void getHtml();

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

bool calc_delay = false;
bool only_device = false;
bool device_server = false;
SimpleList<uint32_t> nodes;

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

void handleRoot() {
  server.send(200, "text/html", LoadHtmlFromFile());
}

void handleMessage()
{
  /*
  int lastPos = 0;
  int selectedCount = 0; 
  char *selectedNodes = server.arg("selectNode");
  int sendNodes[1];
  for(int x =0; x!= selectedNodes.length(); x++)
  {
    if(selectedNodes[x] == '&')
    {
      selectedCount++;
      sendNodes[x] == selectedNodes.substring(lastPos, x - 1).toInt();
      sendNodes = int[selectedCount]
    }
  }
  */
  String target = server.arg("selectNode");
  Serial.println(target);
  Serial.printf("^Target^\n");
  String msg = server.arg("SINGLEMESSAGE");
  //Serial.printf("Sending message to %s:\n", target);
  if(target.toInt() == mesh.getNodeId())
    triggerLight();
  else
    mesh.sendSingle(target.toInt(), msg);

  server.send(200, "text/html", "<h1>Message sent successfully</h1><p>" + msg + "</p><p>To: " + target + "</p>");
}

void handleBroadcast()
{
  String msg = server.arg("BROADCAST");
  mesh.sendBroadcast(msg);
  server.send(200, "text/html", "<h1>Broadcast sent successfully</h1><p>" + msg + "</p>");
}

void triggerLight()
{
  pinState = !pinState;
  digitalWrite(lightPin, (pinState ? HIGH : LOW));
  Serial.printf("Light state:  %s", (pinState ? "On" : "Off"));
}


void sendMessage() ; // Prototype
Task taskSendMessage( TASK_SECOND * 60, TASK_FOREVER, &sendMessage ); // start with a one second interval

void setup() {
  Serial.begin(9600);

  mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();

  nodes = mesh.getNodeList();
  node_count = nodes.size();

  if(node_count == 0)
  {
    device_server = !device_server;
    only_device = !only_device;
  }
  else (
    //Send broadcast to check for existing server
    
  )

  // Trying the ACCESS POINT
  // =======================

  Serial.print("Configuring access point...");
  pinMode(lightPin, OUTPUT);
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(STATION_SSID, STATION_PWD);

  IPAddress local_ip(192,168,3,1);
  IPAddress gateway(192,168,3,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if(device_server) {
    device_server = !device_server;
    server.onNotFound([](){
      server.send(404, "text/plain", "<h1>404: Not found</h1>");
    });
    //Handle different URL messages
    server.on("/", handleRoot);
    server.on("/single", handleMessage);
    server.on("/broadcast", handleBroadcast);
    server.begin();
    Serial.println("HTTP server started");
  }
}

void loop() {
  if(device_server)
    server.handleClient();
  else if(only_device)
    ESP.restart();
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
  //digitalWrite(LED, !onFlag);
}

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  //Serial.printf("Sending message: %s\n", msg.c_str());
  
  taskSendMessage.setInterval( random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
}


void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  if(only_device)
  {
    if(device_server)
      mesh.sendSingle(nodeId, "I am server");
    only_device = !only_device;
  }
}

void changedConnectionCallback() {
  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  nodes = mesh.getNodeList();
  node_count = nodes.size();
  Serial.printf("Num nodes: %d\n", node_count);
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}