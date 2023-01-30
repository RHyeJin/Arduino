//Slave
#include <IRremote.h>
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#define IRsenser A0                                      //Remote control input A0

Servo curtain;                                           //Curtain motor
Servo window;                                            //Window  motor
SoftwareSerial slave(6,7);                               //Arduino connect Rx:6, Tx:7

unsigned long pre_time = 0;                              //Use millis()
unsigned long cur_time = 0;                              // "
LiquidCrystal_I2C LCD(0x3f, 16,2);
volatile byte state = 1;                                 //Use interrupt to change machine status
volatile byte Cstate = 1;                                //Change the curtain status
volatile byte Wstate = 1;                                //Change the window status
volatile byte LCDstate = 1;                              //Change the LCD status(on,off)
IRrecv irrecv(IRsenser);                                 //Use IRsensor to use remotecontrol
decode_results results;                                  //Save remotecontrol address

void setup() {
  LCD.init();
  LCD.backlight();
  Serial.begin(9600);                                    //Check with computer
  slave.begin(9600);
  curtain.attach(8);
  window.attach(4);
  pre_time = millis();                                   //Save millis() 
  irrecv.enableIRIn();
  pinMode(2, INPUT);                                       
  attachInterrupt(0, isr, FALLING);                      //To use interrupt
  pinMode(10,OUTPUT);                                    //Check interrupt with LED(10)
  digitalWrite(10,1);
  window.write(0);
  curtain.write(179);
}

void loop() {
  cur_time = millis();
  int index = 0, i=0;
  char *ptr = NULL;
  if(cur_time - pre_time >= 1000)                         //Loop every second
  { 
    char str[50]; 
    char *sArr[5]; 
    if(state == 1)                                        //Machine status true
    {         
      slave.begin(9600);                                    //When interrupt on machine state change and communication begin                               
      LCD.clear();
      LCD.setDelay(1000, 100);
      while(slave.available())
      { 
        str[index++]= slave.read();                       //Recieve master's data
      }
      if(str != NULL)
      {
        //Serial.println();
        //Serial.print("str : ");
        //Serial.println(str);
        str[index] = '\0';
        ptr = strtok(str,"#");                             //Divide the received string data by "#"
        while(ptr != NULL)
        {
          sArr[i++] = ptr;
          ptr = strtok(NULL, "#"); 
        }  
        int a= atoi(sArr[0]);                              //Save the divided first data to int a -> light
        int b= atoi(sArr[1]);                              //Save the divided second data to int b -> humidity
        int c= atoi(sArr[2]);                              //Save the divided third data to int c -> temperature
        int d= atoi(sArr[3]);                              //Save the divided forth data to int d -> distance
        // Serial.print("light : ");
        // Serial.print(a);
        // Serial.print("        humid : ");
        // Serial.print(b);
        // Serial.print("        temp : ");
        // Serial.print(c);
        // Serial.println();
        Serial.print(d);
        Serial.println();
        LCD.setCursor(0,0);
        LCD.print(" Welcome to");
        LCD.setCursor(0,1);
        LCD.print("    Smart Home!");
        if(d <= 10 && d != 0){                              //When the distance is less than 10cm
          LCD.clear();
          LCD.setCursor(0,0);
          LCD.print("   Warning!!");                       //Put on LCD
        }
        if(irrecv.decode(&results))
       {
          switch(results.value){

            case 0xFF30CF:                                  //Remote control channel 1 address, HEX
            if(Cstate == 1){                                //If curtain status changed the curtain is open or close
              curtain.write(179);
              delay(100);
            }
            else{
              curtain.write(60);
              delay(100);
            }
            Cstate = !Cstate;                               //Status change
            break;

            case 0xFF18E7:                                  //Remote control channel 2 address, HEX
            if(Wstate == 1){                                //If window status changed the window is open or close
              window.write(120);
              delay(50);
              }else{
              window.write(0);
              delay(50);
              }
            Wstate = !Wstate;                               //Status change
            break;

            case 0xFF7A85:                                  //Remote control channel 3 address, HEX
            slave.write(20);                                //Slave send '20' to master when push on channel 3
            break;

            case 0xFF42BD:                                  //Remote control channel 7 address, HEX
            LCD.clear();                                    //Clear previous lcdmonitor
            LCD.setCursor(0,0);
            LCD.print("Tempe =  ");
            LCD.print(c);
            LCD.print(" ");
            LCD.print((char)223);
            LCD.print("C");
            LCD.setCursor(0,1);
            LCD.print("Humid =  ");
            LCD.print(b);
            LCD.print(" %");                                 //Put on temparature and humidity in monitor
            delay(3000);                                     //For 3 seconds
            break;
            
            case 0xFF4AB5:                                   //Remote control channel 8 address, HEX
            LCD.clear();                                     //Clear previous lcdmonitor
            LCD.setCursor(0,0);
            LCD.print("LUX = ");
            LCD.print(a);                                    //Put on illuminance in monitor
            if(a>110){
              LCD.setCursor(0,1);
              LCD.print("Curtain Open!");
            }                                                //If illuminance is more than 110 put on "Curtain open" in monitor
            delay(3000);                                     //For 3 seconds
            break;
            
            case 0xFF52AD:                                   //Remote control channel 9 address, HEX
            LCDstate = !LCDstate;                            //If window LCDstate changed the LCDstate is on or off
            if(LCDstate == 1){ 
              LCD.backlight();
              LCD.display();
            }else{
              LCD.noBacklight();
              LCD.noDisplay();
            }
            break;
            
            default:
            break;
          }
          Serial.print("retomecontrol : ");
          Serial.println(results.value, HEX);
          irrecv.resume();
        }
      }
    }
    pre_time = cur_time;
  }    
}

void isr(){                                                  //Interrupt fuction
  interrupts();
  if(cur_time - pre_time >=500){                             //To prevent chattering
    digitalWrite(10,!state);                                 //Check the interrupt occur with led
    Serial.println("interrupt");                             //Put on computer monitor
    state = !state;                                          //Change the state
    if(state == 0){                                          //If interrupt on and machine state=0
      slave.end();                                           //Comunication off
      LCD.clear();                                           //Checkout status
      LCD.setCursor(0, 0);
      LCD.print("not connected");                             //Checkout status
    }
    window.write(0);
    curtain.write(179);
    pre_time = cur_time; 
  }
}