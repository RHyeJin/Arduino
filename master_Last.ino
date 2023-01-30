//Master

#include <SoftwareSerial.h>                                   // RX TX Serial Communicate
#include <DHT.h>                                              // Humidity
#define DHTPIN 2                                              // Temperature Humidity
#define DHTTYPE DHT11                                         // Using DHT11 

SoftwareSerial P_Serial(6,7);                                 // Using 6pin and 7pin
DHT dht(DHTPIN, DHTTYPE);                                     // Set dht

int lux = A1;   // Analog illuminance
int lu;         // Integer illuminance
unsigned long pre_time = 0;                                   // Pre time
unsigned long cur_time = 0;                                   // Current time
int trig = 3;                                                 // Output of ultrasonic wave
int echo = 4;                                                 // Input of ultrasonic wave                                         // State of LED 
int buzz = 8;                                                 // Buzzer Pin
                                                              
int Lstate = 0;   //The state of street light

void setup() 
{
  Serial.begin(9600);                                         // Check with computer
  P_Serial.begin(9600);                                       // Baudrate is 9600

  pre_time = millis();                                        // Save setup time at pre_time
 
  dht.begin();  
  pinMode(trig, OUTPUT);  
  pinMode(echo, INPUT);
  pinMode(buzz,OUTPUT);

  setColor(0,0,0);                                            // Blue, Green, Red
}

void loop() 
{ 
  digitalWrite(trig, 1);
  delayMicroseconds(10);  
  digitalWrite(trig, 0);
  float dis = ((float)(pulseIn(echo, 1) * 340)/10000)/2;      // Save data of Time of ultrasonic wave reflected
                                                              //from the object and returned 
  int d = dis;                                                // Distance of detected object
  cur_time = millis();            // Save loop time at cur_time

  int h = dht.readHumidity();     // Humidity 
  int t = dht.readTemperature();  // Temperature
  lu = analogRead(lux);           // illuminance
 
  if(dis<400 && dis>2){  // Activate at this range
    if(dis<11){         
      tone(8,2093,100);
      delay(100);
  }
  else{                  // No sound
  noTone(8); 
  }
 } 

  if(cur_time - pre_time >= 1000)
  {
  pre_time = cur_time;

  String sensing1 = (String)lu;                               // light
  String sensing2 = (String)h;                                // humidity
  String sensing3 = (String)t;                                // temperature
  String sensing4 = (String)d;                                // distance
  // It's charater type When we do Serial communication
 
  String str = (String)sensing1 + "#" + (String)sensing2 
                + "#" +(String)sensing3+ "#" +(String)sensing4;
  
  // Send Data to slave By uing "#" 
  // "#" is standard for split data    

  P_Serial.println(str);                                      // Send Data to slave
  int a = P_Serial.read();                                    //slave send to me data a 
 
  Serial.print(" \n");
  Serial.print("slave send : ");
  Serial.println(a); 
  Serial.print("light: ");
  Serial.println(lu);
  Serial.print("temperature: ");
  Serial.println(t);
  Serial.print("humidity: ");
  Serial.println(h);
  Serial.print("distance: ");
  Serial.println(dis);                                    
 
    if(a == 20){                                // If slave send data 20(remote control data)    
      Lstate = Lstate + 1;
      if(Lstate == 1 ){                          // Push button 3 one time    
        setColor(255,1,130);                    // Pink color
      }
      else if(Lstate == 2 ){                    // Push button 3 two times
        setColor(1,130,180);                    // White color
      }
      else if(Lstate == 3 ){                    // Push button 3 three times
        setColor(180,110,1);                    // Blue color
      }
      else{                                     // Push button 4 times
        setColor(0,0,0);
        Lstate =0;
      } 
    }  
  }
}
void setColor(int blue, int green, int red){   //Set LED color 
   analogWrite(13, red);
   analogWrite(12, green);
   analogWrite(11, blue); 
 }
