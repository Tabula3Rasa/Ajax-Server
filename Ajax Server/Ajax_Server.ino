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

    02  LED
        13
        dset
 
    03  empty
        13
        ?
 
    04  empty
        13
        ?
 
 
 */

//{pins} {type} {factor}
String root_table[3][5] = {{"9", "10", "13", "13", "13"},
    {"aget", "aget", "dset", "xxx", "xxx"}, {"10.24", "7.245", "x", "x", "x"}};


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

String HTTP_req;
String protocoll;
String command;
String c_type;
String m_id;
String status;
String extension;
String filename;
char c;
char b;
String file_read;
char* index_filename = "index.htm";
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
        Serial.println("sd card initialization failed!");
        delay(100);
        //return;
    }
}


void setDigital(int mod_id, int status){
    int pin;
    pin = root_table[0][mod_id].toInt();
    //Serial.print("Pin: ");
    //Serial.println(pin);
    if (status)
    {
        digitalWrite(pin, HIGH);
        //Serial.println("Status: ON");
    }
    else
    {
        digitalWrite(pin, LOW);
        //Serial.println("Status: OFF");
    }
}

boolean getDigital(int mod_id) {
    int pin;
    pin = root_table[0][mod_id].toInt();
    
    return digitalRead(pin);
}

float getAnalog(int mod_id){
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
                    Serial.println();
                    */
                    
                    //int i = command.length() + 2;
                    while (HTTP_req.substring(i,i+1) != "." && HTTP_req.substring(i,i+1) != " ") {
                        protocoll += HTTP_req.substring(i,i+1);
                        i++;
                    }
                    /*
                    Serial.print("protocoll: ");
                    Serial.println(protocoll);
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

                        
                        if (protocoll == ""){
                            //Serial.println("HTTP command found!");
                            
                            client.println("Content-Type: text/html");
                            client.println("Connection: keep-alive");
                            
                            // send web page
                            myFile = SD.open(index_filename, FILE_READ);
                            
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
                        else if(protocoll == "ajax") {
                            //Serial.println("AJAX command found!");

                            client.println("Connection: keep-alive");
                            //client.println();
                            
                            //C_TYPE
                            i = HTTP_req.indexOf("C_TYPE") + 7;
                            while (HTTP_req.substring(i,i+1) != "&" && HTTP_req.substring(i,i+1) != " ") {
                                c_type += HTTP_req.substring(i,i+1);
                                i++;
                            }
                            
                            //M_ID
                            i = HTTP_req.indexOf("M_ID") + 5;
                            if (i > 4) {
                                while (HTTP_req.substring(i,i+1) != "&" && HTTP_req.substring(i,i+1) != " ") {
                                    m_id += HTTP_req.substring(i,i+1);
                                    i++;
                                }
                            }
                            
                            //STATUS
                            i = HTTP_req.indexOf("STAT") + 5;
                            //if indexOf finds anything
                            if (i > 4) {
                                while (HTTP_req.substring(i,i+1) != "&" && HTTP_req.substring(i,i+1) != " ") {
                                    status += HTTP_req.substring(i,i+1);
                                    i++;
                                }
                            }
                            
                            /*
                            Serial.print("Command type: ");
                            Serial.println(c_type);
                            Serial.print("Module: ");
                            Serial.println(m_id.toInt());
                            Serial.print("Status: ");
                            Serial.println(status.toInt());
                            Serial.println();
                            */
                            
                            if (c_type=="d_set"){
                                //set a digital pin
                                setDigital(m_id.toInt(), status.toInt());
                                
                                client.println();
                                client.println();
                                
                                /*
                                Serial.println("d_set detected!");
                                Serial.print("Module ID: ");
                                Serial.println(m_id);
                                Serial.print("New Status: ");
                                Serial.println(status);
                                 */
                            }
                            else if(c_type=="d_get"){
                                client.println("Content-Type: text/html");
                                client.println();
                                client.println();
                                
                                //get the status of a digital pin
                                client.println(getDigital(m_id.toInt()));
                                
                                /*
                                Serial.println("d_get detected!");
                                Serial.println(getDigital(m_id));
                                 */
                            }
                            else if(c_type=="a_get"){
                                //get the value of an analog pin
                                float temp_value = getAnalog(m_id.toInt());
                                
                                //client.print("Content-Length: ");
                                //client.println(sizeof(temp_value));
                                client.println("Content-Type: text/html");
                                client.println();
                                client.println();
                                
                                client.println(temp_value);
                                
                                /*
                                Serial.println("a_get detected!");
                                Serial.print("Module_ID: ");
                                Serial.println(m_id);
                                 */
                            }
                            else if(c_type=="a_set"){
                                //set the value of an analog pin
                                /*
                                int value = 0;
                                String str_value;
                                i = 0;
                                
                                while (HTTP_req.substring(HTTP_start+27+i,HTTP_start+27+i+1) != " ") {
                                    str_value += HTTP_req.substring(HTTP_start+27+i,HTTP_start+27+i+1);
                                    i++;
                                }
                                value = str_value.toInt();
                                */
                                
                                setAnalog(m_id.toInt(), status.toInt());
                                
                                //Serial.println("a_set detected!");
                            }
                            else{
                                Serial.println("HTTP-request Error:");
                                Serial.print("PROTOCOLL: ");
                                Serial.println(protocoll);
                                /*
                                Serial.print("C_TYPE: ");
                                Serial.println(c_type);
                                Serial.print("M_ID: ");
                                Serial.println(m_id);
                                Serial.print("STATUS: ");
                                Serial.println(status);
                                Serial.println();
                                
                                
                                Serial.println(HTTP_req);
                                Serial.println();
                                */
                            }
                         
                        }
                        else{
                            //FILENAME
                            i = HTTP_req.indexOf("GET /") + 5;
                            while (HTTP_req.substring(i,i+1) != ".") {
                                filename += HTTP_req.substring(i,i+1);
                                i++;
                            }
                            
                            /*
                            Serial.print("Filename: ");
                            Serial.println(filename);
                            Serial.print("Extension: ");
                            Serial.println(extension);
                            Serial.println();
                            */
                            
                            String header = "";
                            if(extension=="png"){
                                //Serial.println("Extension: png");
                                header = "Connection: keep-alive\nContent-Type: image/png\n";
                                //header = "Connection: keep-alive\nContent-Type: image/png\nContent-Length: 129\n";
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
                            
                            char filename_temp[filename.length()+1];
                            //c_type += HTTP_req.substring(i,i+5);
                            
                            /*
                            Serial.print("Filename before: ");
                            Serial.println(filename);
                            */
                            
                            filename += ".";
                            filename += extension;
                            
                            filename.toCharArray(filename_temp, filename.length()+1);
                            
                            /*
                            Serial.print("Filename: ");
                            Serial.println(filename_temp);
                            Serial.print("Filename length: ");
                            Serial.println(filename.length());
                            Serial.println();
                            
                            if(SD.exists(filename_temp)) {
                                Serial.println("Filename exists!");
                                Serial.println();
                            }
                            else {
                                Serial.println("Filename doesn't exist!");
                                Serial.println();
                            }
                            */
                            
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
                    protocoll = "";
                    command = "";
                    c_type = "";
                    m_id = "";
                    status = "";
                    extension = "";
                    filename = "";
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

