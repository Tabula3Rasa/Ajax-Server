//
// Ajax Server
//
// Description of the project
//
// Author	 	Philipp Haslwanter
// 				Philipp Haslwanter
//
// Date			14/10/14 11:18 am
// Version		<#version#>
//
// Copyright	© Philipp Haslwanter, 2014
// License		<#license#>

#include "Arduino.h"
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

// Include application, user and local libraries


// Define variables and constants
//
// Brief	...
// Details	...
//

/*
 MODULES:
 Module_ID:
    00  'Living room' - Temp Sensor
        A9 = 9
        aget
 
    01  'kitchen' - Temp Sensor
        A10 = 10
        aget
 
    03  empty
        ?
        ?
 
    04  LED
        13
        dset
 
 
 */

//{pins} {type} {factor}
String root_table[3][5] = {{"9", "10", "13", "13", "13"},
    {"aget", "aget", "xxx", "xxx", "dset"}, {"10.24", "7.245", "1", "1"}};


uint8_t pinLED;
uint8_t pinVcc;
byte mac[] = { 0x90, 0xA2, 0xDa, 0x0F, 0x56, 0xE7 };
IPAddress ip(192,168,0,177);
int sendbuff = 1024;

// Setting up a MAC address and IP address for your controller.
// The IP address will be dependent on your local network.

EthernetServer server(80);
// Initializing the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP).

String HTTP_req = "";
String command;
String type = "";
String extension = "";
char c;
char b;
String file_read = "";
char* filename = "index.htm";
File myFile;




//debug variabl







//
// Brief	Setup
// Details	...
//
// Add setup code
void setup()
{
    pinLED = 13;
    pinMode(pinLED  , OUTPUT);
    
    //declare pin 53 as OUTPUT to ensure SPI interface works corretly
    pinMode(53, OUTPUT);
    
    //set pin 37 to HIGH for relais-module
    pinVcc = 37;
    pinMode(pinVcc, OUTPUT);
    digitalWrite(pinVcc, HIGH);
    
    Serial.begin(9600);
    
    
    Ethernet.begin(mac, ip);
    // starting the Ethernet connection using the MAC & IP addresses that we have set.
    
    server.begin();
    //starting the server.
    
    Serial.println();
    Serial.print("server IP is ");
    Serial.println(Ethernet.localIP());
    
    //load webpage from SD-card
    while (!SD.begin(4)) { //select CS-pin
        Serial.println("initialization failed!");
        delay(100);
        //return;
    }
}


void setDigital(int mod_id, int state){
    int pin;
    pin = root_table[0][mod_id].toInt();
    //Serial.print("Pin: ");
    //Serial.println(pin);
    if (state)
    {
        digitalWrite(pin, HIGH);
        //Serial.println("State: ON");
    }
    else
    {
        digitalWrite(pin, LOW);
        //Serial.println("State: OFF");
    }
}

boolean getDigital(int mod_id) {
    int pin;
    pin = root_table[0][mod_id].toInt();
    
    return digitalRead(pin);
}

int getAnalog(int mod_id){
    int pin;
    float fac;
    char temp_char[8];
    pin = root_table[0][mod_id].toInt();
    root_table[2][mod_id].toCharArray(temp_char, root_table[2][mod_id].length() + 1);
    fac = atof(temp_char);
    
    /*
    Serial.print("Analog read factor: ");
    Serial.println(fac);
    Serial.println();
    
    Serial.print("Analog pin number: ");
    Serial.println(pin);
    Serial.print("Analog value: ");
    Serial.println(analogRead(pin));
    Serial.println();
    */
    return analogRead(pin)/fac;
}

void setAnalog(int mod_id, int value){
    int pin;
    pin = root_table[0][mod_id].toInt();
    
    analogWrite(pin, value);
}

// this function will return the number of bytes currently free in RAM
int memoryTest() {
    int byteCounter = 0; // initialize a counter
    byte *byteArray; // create a pointer to a byte array
    // More on pointers here: http://en.wikipedia.org/wiki/Pointer#C_pointers
    
    // use the malloc function to repeatedly attempt allocating a certain number of bytes to memory
    // More on malloc here: http://en.wikipedia.org/wiki/Malloc
    while ( (byteArray = (byte*) malloc (byteCounter * sizeof(byte))) != NULL ) {
        byteCounter++; // if allocation was successful, then up the count for the next try
        free(byteArray); // free memory after allocating it
    }
    
    free(byteArray); // also free memory after the function finishes
    return byteCounter; // send back the highest number of bytes successfully allocated
}


//
// Brief	Loop
//
// Add loop code
void loop()
{
    //Checking for a request
    EthernetClient client = server.available();
    // listening for incoming clients
    if (client)
    {
        
        boolean currentLineIsBlank = false;
        // an http request ends with a blank line
        while (client.connected())
        {
            if (client.available())
            {
                
                c = client.read();
                //Serial.print(c);
                HTTP_req += c;
                
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if(c == '\n' && currentLineIsBlank)
                {
                    int i = 0;
                    while (HTTP_req.substring(i, i+1) != " ") {
                        command += HTTP_req.substring(i, i+1);
                        i++;
                    }
                    
                    i++;
                    i++;
                    /*
                    Serial.print("Command: ");
                    Serial.println(command);
                    */
                    /*
                    Serial.println("START -- Full command --- START");
                    Serial.print(HTTP_req);
                    Serial.println("");
                    */
                    
                    //int i = command.length() + 2;
                    while (HTTP_req.substring(i,i+1) != "." && HTTP_req.substring(i,i+1) != " ") {
                        type += HTTP_req.substring(i,i+1);
                        i++;
                    }
                    /*
                    Serial.print("Type: ");
                    Serial.println(type);
                    */
                    
                    if(HTTP_req.substring(i,i+1)=="."){
                        extension = HTTP_req.substring(i+1,i+4);
                        /*
                        Serial.print("Extension: ");
                        Serial.println(extension);
                        */
                    }
                    
                    if(command == "GET") {
                        //Serial.println("GET found!");
                        // start sending a standard http response header
                        client.println("HTTP/1.1 200 OK");

                        
                        if (type == ""){
                            //Serial.println("HTTP command found!");
                            
                            client.println("Content-Type: text/html");
                            client.println("Connection: keep-alive");
                            
                            // send web page
                            myFile = SD.open(filename, FILE_READ);
                            
                            // if the file opened okay, read it:
                            if (myFile){
                                client.print("Content-Length: ");
                                client.println(myFile.size());
                                client.println();
                                client.println();
                                // read from the file until there's nothing else in it:
                                while (myFile.available()){
                                    client.print(char(myFile.read()));
                                }
                                myFile.close();
                            }
                            else{
                                myFile.close();
                            }
                            
                            client.println();
                        }
                        else if(type == "ajax") {
                            //Serial.println("AJAX command found!");

                            client.println("Connection: keep-alive");
                            //client.println();
                            
                            int HTTP_start = HTTP_req.indexOf("ajax");
                            String HTTP_type = HTTP_req.substring(HTTP_start + 9, HTTP_start + 14);
                            int module_id = HTTP_req.substring(HTTP_start + 20, HTTP_start + 22).toInt();
                            
                            /*
                            Serial.print("HTTP_type: ");
                            Serial.println(HTTP_type);
                            Serial.print("Module: ");
                            Serial.println(module_id);
                            */
                            
                            if (HTTP_type=="d_set"){
                                //set a digital pin
                                int state = HTTP_req.substring(HTTP_start + 28, HTTP_start + 29).toInt();
                                
                                //Serial.print("New State: ");
                                //Serial.println(state);
                                
                                setDigital(module_id, state);
                                
                                //Serial.println("d_set detected!");
                            }
                            else if(HTTP_type=="d_get"){
                                client.println("Content-Type: text/html");
                                client.println();
                                client.println();
                                
                                //get the state of a digital pin
                                client.println(getDigital(module_id));
                                
                                Serial.println("d_get detected!");
                            }
                            else if(HTTP_type=="a_get"){
                                //get the value of an analog pin
                                float temp_value = getAnalog(module_id);
                                
                                //client.print("Content-Length: ");
                                //client.println(sizeof(temp_value));
                                client.println("Content-Type: text/html");
                                client.println();
                                client.println();
                                
                                client.println(temp_value);
                                
                                //Serial.println("a_get detected!");
                            }
                            else if(HTTP_type=="a_set"){
                                //set the value of an analog pin
                                int value = 0;
                                String str_value;
                                i = 0;
                                
                                while (HTTP_req.substring(HTTP_start+27+i,HTTP_start+27+i+1) != " ") {
                                    str_value += HTTP_req.substring(HTTP_start+27+i,HTTP_start+27+i+1);
                                    i++;
                                }
                                value = str_value.toInt();
                                
                                setAnalog(module_id, value);
                                
                                //Serial.println("a_set detected!");
                            }
                            else{
                                Serial.print("HTTP-request Error: type = ");
                                Serial.println(HTTP_type);
                            }
                         
                        }
                        else{
                            String header = "";
                            if(extension=="png"){
                                header = "Connection: keep-alive\nContent-Type: image/png\n";
                            }
                            else if(extension=="jpg"){
                                header = "Connection: keep-alive\nContent-Type: image/jpg\n";
                            }
                            else if (extension == "css"){
                                header = "Connection: keep-alive\nContent-Type: text/css\n";
                            }
                            else{
                                header = "Connection: keep-alive\n";
                                
                                Serial.println("ERROR: extension not known!");
                            }
                            
                            char filename_temp[type.length()+1];
                            type += HTTP_req.substring(i,i+5);
                            type.toCharArray(filename_temp, type.length());
                            
                            // send file
                            myFile = SD.open(filename_temp, FILE_READ);
                            
                            // if the file opened okay, read it:
                            if (myFile){
                                client.println(header);
                                // read from the file until there's nothing else in it:
                                while (myFile.available()){
                                    client.print(char(myFile.read()));
                                }
                                myFile.close();
                                //Serial.println("File opened!");
                            }
                            else{
                                myFile.close();
                                Serial.println("ERROR: File not opened!");
                            }
                            client.println();
                        }
                        
                        
                    }
                    else {
                        Serial.println("No command found!");
                        Serial.print("Command: ");
                        Serial.println(HTTP_req.substring(0, 4));
                        
                    }
                    
                    
                    // display received HTTP request on serial port
                    /*
                    Serial.println();
                    Serial.println("This is the request:");
                    Serial.print(HTTP_req);
                    Serial.println();
                    */
                    HTTP_req = "";            // finished with request, empty string
                    type = "";
                    command = "";
                    extension = "";
                    
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
            }
        }
        delay(1);
        // give the web browser time to receive the data
        // if a lot of requests are sent, the Arduino will freeze
        client.stop();
        //closing the connection:
    }
}

