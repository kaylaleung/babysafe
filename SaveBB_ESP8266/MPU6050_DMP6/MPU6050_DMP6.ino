// Baby Monitor using ESP8266 WeMos D1 Mini with MPU6050 acceleromter
// to detect position of the baby facing up or facing down
// Change log:
//    10/5/2016 - Initial version. Integrate with a Wifi Client code to post it to Cloud
//                when baby position reached a threshold value
//
// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class using DMP (MotionApps v2.0)
/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2012 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include <ESP8266WiFi.h>

// -> WiFiManager
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
// WiFiManager

#include "MPU6050_6Axis_MotionApps20.h"
//#include "MPU6050.h" // not necessary if using MotionApps include file

// -> SHT-30
#include <WEMOS_SHT3X.h>
SHT3X sht30(0x45);
// SHT-30

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// Kayla ->
const char* ssid     = "battlestar";
const char* password = "Galactica173223875078004";

const char* host = "savebbservice02060953.azurewebsites.net";
// We now create a URI for the request
String url = "/tables/alertitem";

bool isPrompt = false;
bool WifiSetup = false;
bool Cloud = true;

int count = 0;

bool turnonBeginCount = true;
int beginCount = 0;
int skipFirstXCount = 500;

// Every 100 very stable. = every 10 secs.
// Every 500 very stable. = every 23 secs. Crashed a few times but it AUTO recovered!
int SAMPLING_COUNT = 200;

// 1 is up, -1 is down, 0 is indeterminate - only use by the very first time!
int lastPosition = 0;  

bool LED_ON = false;

// Kayla <-



// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for SparkFun breakout and InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 mpu;
//MPU6050 mpu(0x69); // <-- use for AD0 high

/* =========================================================================
   NOTE: In addition to connection 3.3v, GND, SDA, and SCL, this sketch
   depends on the MPU-6050's INT pin being connected to the Arduino's
   external interrupt #0 pin. On the Arduino Uno and Mega 2560, this is
   digital I/O pin 2.
 * ========================================================================= */

/* =========================================================================
   NOTE: Arduino v1.0.1 with the Leonardo board generates a compile error
   when using Serial.write(buf, len). The Teapot output uses this method.
   The solution requires a modification to the Arduino USBAPI.h file, which
   is fortunately simple, but annoying. This will be fixed in the next IDE
   release. For more info, see these links:

   http://arduino.cc/forum/index.php/topic,109987.0.html
   http://code.google.com/p/arduino/issues/detail?id=958
 * ========================================================================= */



// uncomment "OUTPUT_READABLE_QUATERNION" if you want to see the actual
// quaternion components in a [w, x, y, z] format (not best for parsing
// on a remote host such as Processing or something though)
//#define OUTPUT_READABLE_QUATERNION
// PAY ATTENTION to z  1 -> 0.5 (90 degree) -> 0

// uncomment "OUTPUT_READABLE_EULER" if you want to see Euler angles
// (in degrees) calculated from the quaternions coming from the FIFO.
// Note that Euler angles suffer from gimbal lock (for more info, see
// http://en.wikipedia.org/wiki/Gimbal_lock)
// Kayla ->
#define OUTPUT_READABLE_EULER
// Kayla <- 
// PAY ATTENTION to last variable  0 -> 80  ->  179  SEEMS Reliable

// uncomment "OUTPUT_READABLE_YAWPITCHROLL" if you want to see the yaw/
// pitch/roll angles (in degrees) calculated from the quaternions coming
// from the FIFO. Note this also requires gravity vector calculations.
// Also note that yaw/pitch/roll angles suffer from gimbal lock (for
// more info, see: http://en.wikipedia.org/wiki/Gimbal_lock)
//#define OUTPUT_READABLE_YAWPITCHROLL
// PAY ATTENTION to YAW & ROW.  YAW     ROW 0 -> 80  -> 0

// uncomment "OUTPUT_READABLE_REALACCEL" if you want to see acceleration
// components with gravity removed. This acceleration reference frame is
// not compensated for orientation, so +X is always +X according to the
// sensor, just without the effects of gravity. If you want acceleration
// compensated for orientation, us OUTPUT_READABLE_WORLDACCEL instead.
//#define OUTPUT_READABLE_REALACCEL

// uncomment "OUTPUT_READABLE_WORLDACCEL" if you want to see acceleration
// components with gravity removed and adjusted for the world frame of
// reference (yaw is relative to initial orientation, since no magnetometer
// is present in this case). Could be quite handy in some cases.
//#define OUTPUT_READABLE_WORLDACCEL

// uncomment "OUTPUT_TEAPOT" if you want output that matches the
// format used for the InvenSense teapot demo
//#define OUTPUT_TEAPOT



#define INTERRUPT_PIN D6  // use pin 2 on Arduino Uno & most boards
#define LED_PIN D6 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = { '$', 0x02, 0,0, 0,0, 0,0, 0,0, 0x00, 0x00, '\r', '\n' };

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}



// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize serial communication
    // (115200 chosen because it is required for Teapot Demo output, but it's
    // really up to you depending on your project)
    Serial.begin(115200);
    while (!Serial); // wait for Leonardo enumeration, others continue immediately

// Kayla ->
    if(WifiSetup)
    {
      // We start by connecting to a WiFi network
      Serial.println();
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(ssid);
      
      WiFi.begin(ssid, password);
      
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    
      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
    else
    {
      Serial.println();
      Serial.println();
      Serial.print("Auto Connecting ");
      
      WiFiManager wifiManager;
      //reset saved settings
      //wifiManager.resetSettings();

      wifiManager.autoConnect();

      Serial.println("");
      Serial.println("WiFi connected");  
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());      
    }
// Kayla <-

 
    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3v or Ardunio
    // Pro Mini running at 3.3v, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.

    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

// Kayla -> 
    // wait for ready
    if(isPrompt)
    {
      Serial.println(F("\nSend any character to begin DMP programming and demo: "));
      while (Serial.available() && Serial.read()); // empty buffer
      while (!Serial.available());                 // wait for data
      while (Serial.available() && Serial.read()); // empty buffer again    
    }
    else
      delay(5000);
// Kayla <-
 
    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(220);
    mpu.setYGyroOffset(76);
    mpu.setZGyroOffset(-85);
    mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

    // make sure it worked (returns 0 if so)
    if (devStatus == 0) {
        // turn on the DMP, now that it's ready
        Serial.println(F("Enabling DMP..."));
        mpu.setDMPEnabled(true);

        // enable Arduino interrupt detection
        Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
        attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
        mpuIntStatus = mpu.getIntStatus();

        // set our DMP Ready flag so the main loop() function knows it's okay to use it
        Serial.println(F("DMP ready! Waiting for first interrupt..."));
        dmpReady = true;

        // get expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
    } else {
        // ERROR!
        // 1 = initial memory load failed
        // 2 = DMP configuration updates failed
        // (if it's going to break, usually the code will be 1)
        Serial.print(F("DMP Initialization failed (code "));
        Serial.print(devStatus);
        Serial.println(F(")"));
    }

    // configure LED for output
    //pinMode(LED_PIN, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    LED_ON = false;
    digitalWrite(LED_BUILTIN, HIGH); // OFF

}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
        // other program behavior stuff here
        // .
        // .
        // .
        // if you are really paranoid you can frequently test in between other
        // stuff to see if mpuInterrupt is true, and if so, "break;" from the
        // while() loop to immediately process the MPU data
        // .
        // .
        // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
        // reset so we can continue cleanly
        mpu.resetFIFO();
        //Serial.println(F("FIFO overflow!"));

    // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {

        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

        // track FIFO count here in case there is > 1 packet available
        // (this lets us immediately read more without waiting for an interrupt)
        fifoCount -= packetSize;

// Kayla -> 
        // Skip the 1st 500 data which are unstable
        if(turnonBeginCount && beginCount < skipFirstXCount)
        {
          beginCount++;
          Serial.print("beginCount: ");
          Serial.println(beginCount);
          return;
        }
        else
        {
          if(turnonBeginCount)
          {
            LED_ON = true;
            digitalWrite(LED_BUILTIN, LOW); // ON
          }
          turnonBeginCount = false;
        }
// Kayla <-           

        #ifdef OUTPUT_READABLE_QUATERNION
            // display quaternion values in easy matrix form: w x y z
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            Serial.print("quat\t");
            Serial.println(q.w);
            Serial.print("\t");
            Serial.print(q.x);
            Serial.print("\t");
            Serial.print(q.y);
            Serial.print("\t");
            Serial.println(q.z);
        #endif

        #ifdef OUTPUT_READABLE_EULER            
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetEuler(euler, &q);

// Kayla ->             
            //Serial.print("euler\t");
            //Serial.print(euler[0] * 180/M_PI);
            //Serial.print("\t");
            //Serial.print(euler[1] * 180/M_PI);
            //Serial.print("\t");
            //Serial.println(abs( euler[2] * 180/M_PI ));

            if(Cloud)
            {
              // Use WiFiClient class to create TCP connections
              WiFiClient client;
              const int httpPort = 80;
              if (!client.connect(host, httpPort)) {
                Serial.println("http connection failed");
                return;
              }

              count++;
              int v = (int) abs( euler[2] * 180/M_PI );
              String PostData;
              //Serial.print(count);
              //Serial.print(": ");
              //Serial.println(v);
              //Sample every SAMPLING_COUNT times print send request once
              if(count >= SAMPLING_COUNT)
              {
                count = 0;
                bool sendToCloud = false;

                // Toggle the LED
                if(LED_ON)
                {
                  LED_ON = false;
                  digitalWrite(LED_BUILTIN, HIGH);
                }
                else
                {
                  LED_ON = true;
                  digitalWrite(LED_BUILTIN, LOW);                
                }            

                long randNumberHumidity;
                randNumberHumidity = random(55, 60);
                long randNumberTemp;
                randNumberTemp = random(70, 75);
                long randNumberHeartRate;
                randNumberHeartRate = random(80, 90);

                String env = ", 'Humidity':";
                //env += randNumberHumidity;
                //env += ", 'Temp':";
                //env += randNumberTemp;
                //env += ", 'HeartRate':";
                //env += randNumberHeartRate;

                // -> SHT-30
                sht30.get();
                env += sht30.humidity;
                env += ", 'Temp':";
                env += sht30.fTemp;
                env += ", 'HeartRate':";
                env += randNumberHeartRate;
                // SHT-30

                // if facedown
                if(v >= 140)
                {
                  // -1 means facedown, if facedown before don't send again
                  if(lastPosition != -1)
                    sendToCloud = true;
                    
                  lastPosition = -1;
                  //PostData = "{'AlertType':'SMS','PhoneNum':'17329936799','AlertValue':'-1', 'Deleted':false}";
                  PostData = "{'AlertType':'SMS','PhoneNum':'17329936799','AlertValue':'-1', 'Deleted':false";
                  PostData += env;
                  PostData += "}";
                }                  
                else
                {
                  // 1 means faceup, if faceup before don't send again
                  if(lastPosition != 1)
                    sendToCloud = true;
                    
                  lastPosition = 1;
                  //PostData = "{'AlertType':'','PhoneNum':'','AlertValue':'0', 'Deleted':false}";
                  PostData = "{'AlertType':'','PhoneNum':'17329936799','AlertValue':'0', 'Deleted':false";
                  PostData += env;
                  PostData += "}";
                }

                if(sendToCloud)
                {
                  Serial.print("Requesting URL: ");
                  Serial.print(host);
                  Serial.println(url);
                  Serial.println(PostData);
    
                  client.println("POST " + url + " HTTP/1.1");
                  client.print("Host: ");
                  client.println(host);
                  client.println("Content-Type: application/json");
                  client.println("ZUMO-API-VERSION: 2.0.0");
                  client.print("Content-Length: ");
                  client.println(PostData.length());
                  client.println();
                  client.println(PostData);
  
                  Serial.println("SENT to Cloud!");
                }
                else
                  Serial.println("No change in position, SKIP sending");
                                
              }

              //unsigned long timeout = millis();
              //while (client.available() == 0) {
              //  if (millis() - timeout > 5000) {
              //    Serial.println(">>> Client Timeout !");
              //    client.stop();
              //    return;
              //  }              
              //}

              // Read all the lines of the reply from server and print them to Serial
              //while(client.available()){
              //  String line = client.readStringUntil('\r');
              //  Serial.print(line);
              //}
              
              //Serial.println();
              //Serial.println("closing connection");            
            }
// Kayla <- 
            
        #endif

        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            Serial.print("ypr\t");
            Serial.print(ypr[0] * 180/M_PI);
            Serial.print("\t");
            Serial.print(ypr[1] * 180/M_PI);
            Serial.print("\t");
            Serial.println(ypr[2] * 180/M_PI);
        #endif

        #ifdef OUTPUT_READABLE_REALACCEL
            // display real acceleration, adjusted to remove gravity
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            Serial.print("areal\t");
            Serial.print(aaReal.x);
            Serial.print("\t");
            Serial.print(aaReal.y);
            Serial.print("\t");
            Serial.println(aaReal.z);
        #endif

        #ifdef OUTPUT_READABLE_WORLDACCEL
            // display initial world-frame acceleration, adjusted to remove gravity
            // and rotated based on known orientation from quaternion
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetAccel(&aa, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
            mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
            Serial.print("aworld\t");
            Serial.print(aaWorld.x);
            Serial.print("\t");
            Serial.print(aaWorld.y);
            Serial.print("\t");
            Serial.println(aaWorld.z);
        #endif
    
        #ifdef OUTPUT_TEAPOT
            // display quaternion values in InvenSense Teapot demo format:
            teapotPacket[2] = fifoBuffer[0];
            teapotPacket[3] = fifoBuffer[1];
            teapotPacket[4] = fifoBuffer[4];
            teapotPacket[5] = fifoBuffer[5];
            teapotPacket[6] = fifoBuffer[8];
            teapotPacket[7] = fifoBuffer[9];
            teapotPacket[8] = fifoBuffer[12];
            teapotPacket[9] = fifoBuffer[13];
            Serial.write(teapotPacket, 14);
            teapotPacket[11]++; // packetCount, loops at 0xFF on purpose
        #endif

        // blink LED to indicate activity
        //blinkState = !blinkState;
        //digitalWrite(LED_PIN, blinkState);
    }
   
}


