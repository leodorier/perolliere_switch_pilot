#include <SPI.h>
#include <Ethernet.h>

//Zyxel4G public IP adress : 37.166.117.175
//adresse mac du Arduino Shield 2
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x94, 0xAA };

//adresse IP, masque de sous-réseau, addresse IP de la passerelle, adresse IP du serveur DNS (pas tous utilisés)
IPAddress ip(192, 168, 1, 35);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress myDns(8, 8, 8, 8);

EthernetServer server(80);                  //serveur Arduino
EthernetClient clientExt;                     //client externe qui va interagir avec le serveur Arduino

bool buttons[8] = {false};
char *labels[8] = {"C_Propane","C_Station","C_Ohm","C_Bayarde","L_Brevenne","L_Lebon","L_Bethenod","L_Faraday"};

void initPins(){
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(12, INPUT);
  pinMode(5, INPUT);
  pinMode(6, INPUT);
  pinMode(7, INPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(12, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(9, LOW);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Parametrage IP fixe...");
  Ethernet.begin(mac, ip);                  //on paramètre en IP fixe
  // initPins(); // On initialise les LEDs
  Serial.println("Pret !");
  if (Ethernet.linkStatus() == LinkON)        //on vérifie l'état du lien ethernet
    Serial.println("Link status: on");
  else 
    Serial.println("Link status: off");

  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Listen for incoming clients:
  EthernetClient client = server.available();
  size_t i = 0, j = 0;
  if (client) {
    // Read the request data:
    String method = "";
    String request = "";
    while (client.connected()&& client.available()) {
      char c = client.read();
      if (i < 4) method += c;
      if (c == '\n') j++;
      if (j >= 8) request += c;
      i++;
    }
    parseRequest(&request, &method, client);
    sendHTMLPage(client);
    client.stop();
  }
}

void parseRequest(String* request, String* method, EthernetClient client) {
  if (method->indexOf("POST") < 0) return;
  String path = request->substring(request->indexOf("button=") + 7);
  if (path[0] >= '0' && path[0] <= '9') {
    int btnNb = path[0] - '0';
    buttons[btnNb] = !buttons[btnNb];
    digitalWrite(btnNb == 2 ? 12 : btnNb + 2, buttons[btnNb] ? HIGH : LOW);
  }
}

void sendHTMLPage(EthernetClient client) {
  char button[85] = "";
  client.println("HTTP/1.1 200 OK");
  client.println();
  client.println("<html><head><title>Switch pilot</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<style>body{display:flex;justify-content:center;align-items:center;flex-direction:column}form,div{display:flex;gap:6em}div{flex-direction:column;gap:3em}h1{margin:1em;font-size:2em;font-weight:700}.allume{background:#E5BE01}");
  client.println("button{padding:1.5em 2.5em;border:none}</style></head>");
  client.println("<body><h1>Switch pilot</h1><form action=\"/\" method=\"post\">");
  for (int j = 0; j < 2; j++) { 
    client.println("<div><h2>");
    client.println(j == 0 ? "Coulomb" : "Laplace");
    client.println("</h2>");
    for (int i = 0; i < 4; i++) {
      sprintf(button, "<button type=\"submit\" name=\"button\" value=\"%d\" class=\"%s\">%s</button>", i + j * 4, buttons[i+j*4] ? "allume" : "eteint", labels[i+j*4]);
      client.println(button);
    }
    client.println("</div>");
  }
  client.println("</form></body></html>");
}
