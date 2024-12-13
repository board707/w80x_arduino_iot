#include <Arduino.h>
#include <W600WiFi.h>
#include <WiFiServer.h>

WiFiServer	server(80);
String HTTP_req;          // stores the HTTP request
boolean LED_status = 0;   // state of LED, off by default

void ProcessCheckbox(WiFiClient cl);

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting AP mode ...");
	WiFi.softAP("W801", "1234567890");
	delay(5000);
	Serial.println(String("STANUM: ") + String(WiFi.softAPgetStationNum()));
    Serial.println(String("AP IP: ") + String(WiFi.softAPIP()));
    Serial.println(String("AP MAC: ") + String(WiFi.softAPmacAddress()));
    Serial.println(String("AP SSID: ") + String(WiFi.softAPSSID()));
    Serial.println(String("AP PSK: ") + String(WiFi.softAPPSK()));
	server.begin();
	delay(1000);
	pinMode(LED_BUILTIN_2, OUTPUT);
	if(server.status()==1) Serial.println("Server status - listen...");
	
}
void loop()
{
	WiFiClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Arduino LED Control</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>LED</h1>");
                    client.println("<p>Click to switch LED on and off.</p>");
                    client.println("<form method=\"get\">");
                    ProcessCheckbox(client); // LED control
                    client.println("</form>");
                    client.println("</body>");
                    client.println("</html>");
                    Serial.print(HTTP_req); // Show request
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)

}

// switch LED and send back HTML for LED checkbox
void ProcessCheckbox(WiFiClient cl)
{
    if (HTTP_req.indexOf("LED2=2") > -1) {  // see if checkbox was clicked
        // the checkbox was clicked, toggle the LED
        if (LED_status) {
            LED_status = 0;
        }
        else {
            LED_status = 1;
        }
    }
    
    if (LED_status) {    // switch LED on
        digitalWrite(LED_BUILTIN_2, HIGH);
        // checkbox is checked
        cl.println("<input type=\"checkbox\" name=\"LED2\" value=\"2\" \
        onclick=\"submit();\" checked>LED2");
    }
    else {              // switch LED off
        digitalWrite(LED_BUILTIN_2, LOW);
        // checkbox is unchecked
        cl.println("<input type=\"checkbox\" name=\"LED2\" value=\"2\" \
        onclick=\"submit();\">LED2");
    }
}