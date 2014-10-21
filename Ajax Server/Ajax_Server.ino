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
        01  Brightness Senor
            A8 = 8
            aget
 
        02  LED
            13
            dset
 
        03  Brightness Sensor
            A9 = 9
            aget
 */


String root_table[2][3] = {{"8", "13", "9"},
    {"aget, dset", "aget"}};


uint8_t pinLED;
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
String page;
char c;
char b;
String file_read = "";




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
    
    Serial.begin(9600);
    
    
    Ethernet.begin(mac, ip);
    // starting the Ethernet connection using the MAC & IP addresses that we have set.
    
    server.begin();
    //starting the server.
    
    Serial.print("server IP is ");
    Serial.println(Ethernet.localIP());
    
    
    //load webpage from SD-card
    while (!SD.begin(4)) { //select CS-pin
        Serial.println("initialization failed!");
        delay(100);
        //return;
    }
    Serial.println("SD initialization done.");
    
    //open file
    File myFile = SD.open("arduino2.htm", FILE_READ);
    
    // if the file opened okay, read it:
    if (myFile)
    {
        // read from the file until there's nothing else in it:
        while (myFile.available())
        {
            c = myFile.read();
            page.concat(c);
        }
        //Serial.println(page);
        myFile.close();
        
        Serial.println("File loading completed.");
    }
    else
    {
        // if the file didn't open, print an error:
        Serial.println("Error opening htm-file");
    }
}


void setDigital(int mod_id, int state)
{
    int pin;
    pin = root_table[0][mod_id-1].toInt();
    //Serial.print("Pin to be set: ");
    //Serial.println(pin);
    
    if (state)
    {
        digitalWrite(pin, HIGH);
        /*
        Serial.print("Digital pin ");
        Serial.print(mod_id);
        Serial.println(" set to HIGH");
        */
    }
    else
    {
        digitalWrite(pin, LOW);
        
        /*
        Serial.print("Digital pin ");
        Serial.print(mod_id);
        Serial.println(" set to LOW");
        */
    }
}

int getAnalog(int mod_id)
{
    int pin;
    pin = root_table[0][mod_id-1].toInt();
    
    /*
    Serial.print("Analog pin number: ");
    Serial.println(pin);
    Serial.print("Analog value: ");
    Serial.println(analogRead(pin));
    Serial.println("");
    */
    return analogRead(pin);
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
                    //Ajax request received
                    String command = HTTP_req.substring(0, 4);
                    String type = "";
                    command.trim();
                    
                    Serial.print("Command: ");
                    Serial.println(command);
                    
                    int i = command.length() + 2;
                    while (HTTP_req.substring(i,i+1) != "." && HTTP_req.substring(i,i+1) != "/") {
                        type += HTTP_req.substring(i,i+1);
                        i++;
                    }
                    
                    Serial.print("Type: ");
                    Serial.println(type);
                    
                    if(command == "GET") {
                        //Serial.println("GET found!");
                        
                        // start sending a standard http response header
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        //client.println("Connection: close");
                        client.println();
                        
                        if (type == " HTTP"){
                            Serial.println("HTTP command found!");
                            
                            /*
                            client.print("Content-Lenght: ");
                            client.println(page.length());
                            client.println("Connection: keep-alive");
                             */
                            //client.println();
                            
                            // send web page
                            //Serial.println("webserver return:");
                            //Serial.print("Page size: ");
                            //Serial.println(page.length());
                            long f;
                            for (f=0; f < page.length(); f++) {
                                client.print(page.substring(f, f+1));
                            }
                            
                            /*
                            for (f = 1; page.length() > sendbuff * f; f++) {
                                client.print(page.substring((f-1)*sendbuff,f*sendbuff));

                                Serial.print("Sent from ");
                                Serial.print((f-1)*sendbuff);
                                Serial.print(" to ");
                                Serial.println(f*sendbuff-1);
                                //Serial.println(f);
                            }
                            client.print(page.substring((f-1) * sendbuff));
                            //Serial.print(page.substring((f-1) * sendbuff));
                            
                            Serial.print("Sent from ");
                            Serial.println((f-1)*sendbuff);
                            */
                             
                            client.println();
                            
                            //Serial.print(page);
                            
                            
                        }
                        else if(type == "ajax") {
                            //Serial.println("AJAX command found!");

                            //client.print("Content-Lenght: ");
                            //client.println(page.length());
                            client.println("Connection: keep-alive");
                            client.println();
                            
                            int HTTP_start = HTTP_req.indexOf("ajax");
                            String HTTP_type = HTTP_req.substring(HTTP_start + 9, HTTP_start + 14);
                            int module_id = HTTP_req.substring(HTTP_start + 20, HTTP_start + 22).toInt();
                            
                            
                            Serial.print("HTTP_type: ");
                            Serial.println(HTTP_type);
                            Serial.print("Module: ");
                            Serial.println(module_id);
                            
                            
                            if (HTTP_type=="d_set"){
                                //set a digital pin
                                int state = HTTP_req.substring(HTTP_start + 28, HTTP_start + 29).toInt();
                                
                                Serial.print("New State: ");
                                Serial.println(state);
                                
                                setDigital(module_id, state);
                                
                                //Serial.println("d_set detected!");
                            }
                            else if(HTTP_type=="d_get"){
                                //get the state of a digital pin
                                
                                Serial.println("d_get detected!");
                            }
                            else if(HTTP_type=="a_get"){
                                //get the value of an analog pin
                                
                                client.print(getAnalog(module_id)/10.24);
                                client.println(";77");
                                
                                //Serial.println("a_get detected!");
                            }
                            else if(HTTP_type=="a_set"){
                                //set the value of an analog pin
                                
                                Serial.println("a_set detected!");
                            }
                            else{
                                Serial.print("HTTP-request Error: type = ");
                                Serial.println(HTTP_type);
                            }

                            
                            
                        }
                        else{
                            Serial.println("LINK command found!");
                            Serial.println(type);
                        }
                        
                    }
                    else if(command=="POST"){
                        //Serial.println("POST found!");
                        
                        if(type == "upload"){
                            //Serial.println("UPLOAD command found!");
                            
                            Serial.println("Start reading:");
                            
                            /*
                             File dataFile = SD.open("test.txt", FILE_WRITE);
                             
                             // if the file is available, write to it:
                             if (dataFile) {
                             for (int i = 0; i<195; i++) {
                             b = client.read;
                             dataFile.print(b);
                             }
                             dataFile.close();
                             
                             }
                             */
                            
                            int index = HTTP_req.indexOf("Content-Length") + 16;
                            int add = 0;
                            long startindex = 0;
                            long endindex = 0;
                            String filename = "";
                            String boundary = "";
                            boolean empty_line = false;
                            
                            while (HTTP_req.substring(index + add, index + add + 1) != String('\n')) {
                                add++;
                            }
                            
                            long content_length = HTTP_req.substring(index, index + add).toInt();
                            
                            for (int i = 0; i < content_length; i++) {
                                b = client.read();
                                file_read += b;
                            }
                            
                            Serial.println("Request:");
                            Serial.println(HTTP_req);
                            Serial.println("File:");
                            Serial.println(file_read);
                            
                            index = file_read.indexOf("filename") + 10;
                            
                            add  = 1;
                            int i = 0;
                            
                            String s = file_read.substring(index, index + add);
                            
                            while (s != "\"" && i < 10){
                                add++;
                                Serial.println(s);
                                s = file_read.substring(index + add - 1, index + add);
                            }
                            add--;
                            
                            filename = file_read.substring(index, index + add);
                            Serial.print("Filename:");
                            Serial.println(filename);
                                
                            
                            /*
                            Serial.println("--------------------------------");
                            Serial.print(file_read);
                            Serial.println("--------------------------------");
                            */
                            
                            long index1 = 0;
                            long index2 = 0;
                            
                            index1 = index2 = file_read.indexOf("boundary=");
                            
                            Serial.print("Index1 = Index2: ");
                            Serial.println(index1);
                            Serial.println(index2);
                            
                            s = file_read.substring(index2, index2 + 1);
                            
                            while (s != String('\r')) {
                                index2++;
                                s = file_read.substring(index2, index2 + 1);
                            }
                            
                            boundary = file_read.substring(index1, index2);
                            
                            Serial.print("Boundary-content search start: ");
                            Serial.println(index2 + 3);
                            Serial.println(boundary);
                            
                            index2 = file_read.indexOf(boundary);
                            
                            Serial.print("index1: ");
                            Serial.println(index1);
                            Serial.print("index2: ");
                            Serial.println(index2);
                            
                            
                            while (index2 <= index1) {
                                index2 = file_read.indexOf(boundary, index2 + 1);
                            }
                            
                            Serial.print("Boundary-content search end: ");
                            Serial.println(index2 - 3);
                            
                            //find start of file:
                            s = file_read.substring(startindex, startindex + 1);
                            
                            while (s != String('\n') || !empty_line) {
                                if (empty_line && s == String('\r')) {
                                    s = file_read.substring(startindex, startindex + 1);
                                    startindex += 2;
                                    break;
                                }
                                else if(s == String('\n')) {
                                    empty_line = true;
                                }
                                else {
                                    
                                    empty_line = false;
                                }
                                 
                                startindex++;
                                s = file_read.substring(startindex, startindex + 1);
                            }
                            
                            //find end of file:
                            endindex = startindex + file_read.substring(startindex).indexOf("------WebKitFormBoundary") - 1;
                            
                            Serial.print("Startindex: ");
                            Serial.println(startindex);
                            Serial.print("Endindex: ");
                            Serial.println(endindex);
                            Serial.println();
                            
                            /*
                            Serial.println("File:");
                            Serial.println(file_read.substring(startindex, endindex));
                            Serial.println();
                            */
                            
                            //now save the file to the sd-card!!!!
                            
                            char cfilename[filename.length()+1];
                            filename.toCharArray(cfilename, sizeof(cfilename));
                            
                            if (SD.exists(cfilename)) {
                                Serial.println("File already exists!");
                                SD.remove(cfilename);
                            }
                            
                            File myFile = SD.open(cfilename, FILE_WRITE);
                            
                            if (myFile) {
                                Serial.println("Write to file");
                                myFile.print(file_read.substring(startindex, endindex + 1));
                                Serial.println("Close file");
                                myFile.close();
                            }
                            
                            //page = file_read;

                            
                            /*
                             while(b != '\n'){
                             b = client.read();
                             }
                             */
                            
                            Serial.println("done reading...");
                            Serial.println("");

                        }
                        else{
                            Serial.println("No command found!");
                            Serial.println(type);
                        }
                        
                    }
                    else {
                        
 
                        Serial.println("No command found!");
                        Serial.print("Command: ");
                        Serial.println(HTTP_req.substring(0, 4));
                        
                    }
                    
                    
                    //--------------------------------------------------------------------------------------------------------------------------------------
                    if(HTTP_req.indexOf("ajax") > -1)
                    {
//                        // send a standard http response header
//                        client.println("HTTP/1.1 200 OK");
//                        client.println("Content-Type: text/html");
//                        //client.print("Content-Lenght: ");
//                        //client.println(page.length());
//                        client.println("Connection: keep-alive");
//                        client.println();
//                        
//                        //Serial.println(HTTP_req);
//                        
//                        int HTTP_start = HTTP_req.indexOf("ajax");
//                        String HTTP_type = HTTP_req.substring(HTTP_start + 9, HTTP_start + 14);
//                        int module_id = HTTP_req.substring(HTTP_start + 20, HTTP_start + 22).toInt();
//                        
//                        Serial.println("ajax command detecte!");
//                        Serial.print("Module: ");
//                        Serial.println(module_id);
//                        
//                        
//                         if (HTTP_type=="d_set"){
//                             //set a digital pin
//                             int state = HTTP_req.substring(HTTP_start + 28, HTTP_start + 29).toInt();
//                             
//                             Serial.print("New State: ");
//                             Serial.println(state);
//                             
//                             setDigital(module_id, state);
//                             
//                             //Serial.println("d_set detected!");
//                         }
//                         else if(HTTP_type=="d_get"){
//                             //get the state of a digital pin
//                             
//                             Serial.println("d_get detected!");
//                         }
//                         else if(HTTP_type=="a_get"){
//                         //get the value of an analog pin
//                         
//                             client.print(getAnalog(module_id));
//                             client.println(";77");
//                             
//                             //Serial.println("a_get detected!");
//                         }
//                         else if(HTTP_type=="a_set"){
//                             //set the value of an analog pin
//                             
//                             Serial.println("a_set detected!");
//                         }
//                         else{
//                             Serial.print("HTTP-request Error: type = ");
//                             Serial.println(HTTP_type);
//                         }
//                        
//                        Serial.print("HTTP request type = ");
//                        Serial.println(HTTP_type);
//                        Serial.println("");
                        
                    }
                    else if(HTTP_req.indexOf("upload") > -1)
                    {
//                        Serial.println("Start reading:");
//                        
//                        /*
//                        File dataFile = SD.open("test.txt", FILE_WRITE);
//                        
//                        // if the file is available, write to it:
//                        if (dataFile) {
//                            for (int i = 0; i<195; i++) {
//                                b = client.read;
//                                dataFile.print(b);
//                            }
//                            dataFile.close();
//                            
//                        }
//                        */
//                        
//                        int index = HTTP_req.indexOf("Content-Length") + 16;
//                        int add = 0;
//                        String cr = "";
//                        cr = String('\n');
//                        
//                        while (HTTP_req.substring(index + add, index + add + 1) != String('\n')) {
//                            add++;
//                        }
//                        
//                        long content_length = HTTP_req.substring(index, index + add).toInt();
//                        
//                        for (int i = 0; i < content_length; i++) {
//                            b = client.read();
//                            file_read += b;
//                        }
//                        
//                        Serial.println("");
//                        Serial.print(file_read);
//                        Serial.println("");
//                        
//                        /*
//                        boolean empty_line = false;
//                        
//                        long startindex = 0;
//                        long endindex = 0;
//                        
//                        while (file_read.substring(startindex, startindex + 1) != String('\n') && empty_line) {
//                            if (file_read.substring(startindex, startindex +1) == " ") {
//                                empty_line = true;
//                            }
//                            else {
//                            
//                                empty_line = false;
//                            }
//                            
//                            startindex++;
//                        }
//                        
//                        startindex += 2;
//                        
//                        
//                        
//                        Serial.print("Startindex = ");
//                        Serial.println(startindex);
//                        Serial.print("Endindex = ");
//                        Serial.println(endindex);
//                         */
//                        
//                        /*
//                        while(b != '\n'){
//                            b = client.read();
//                        }
//                        */
//                        
//                        Serial.println("done reading...");
//                        Serial.println("");
                        
                    }
//                    else // HTTP request for web page
//                    {
//                        // send a standard http response header
//                        client.println("HTTP/1.1 200 OK");
//                        client.println("Content-Type: text/html");
//                        client.print("Content-Lenght: ");
//                        client.println(page.length());
//                        client.println("Connection: keep-alive");
//                        client.println();
//                        
//                        // send web page
//                        Serial.println();
//
//                        long f;
//                        for (f = 1; page.length() > sendbuff * f; f++) {
//                            client.print(page.substring((f-1)*sendbuff,f*sendbuff));
//                            
//                            /*
//                            Serial.print("Sent from ");
//                            Serial.print((f-1)*sendbuff);
//                            Serial.print(" to ");
//                            Serial.println(f*sendbuff-1);
//                            */
//                        }
//                        client.print(page.substring((f-1)*sendbuff));
//                        
//                        /*
//                        Serial.print("Sent from ");
//                        Serial.print((f-1)*sendbuff);
//                        Serial.print(" to ");
//                        Serial.println(page.length());
//                        Serial.println();
//                         */
//                    }
                    
                    //--------------------------------------------------------------------------------------------------------------------------------------
                    
                    
                    // display received HTTP request on serial port
                    
                    Serial.println();
                    Serial.println("This is the request:");
                    Serial.print(HTTP_req);
                    Serial.println();
                    
                    HTTP_req = "";            // finished with request, empty string
                    
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
    
    /*
    digitalWrite(myLED, HIGH);
    Serial.println("LED ON");
    delay(500);
    digitalWrite(myLED, LOW);
    Serial.println("LED OFF");
    delay(500);
     */
}
 
