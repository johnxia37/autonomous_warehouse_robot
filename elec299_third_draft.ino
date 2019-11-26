#include "QSerial.h"
#include <Servo.h>
#include <String.h>
QSerial myIRserial;

//pin definitions
#define Lencoder 2
#define Rencoder 3
#define Ldirection 4
#define Lspeed 5
#define Rspeed 6
#define Rdirection 7
#define sensor 11
#define Rbumper 12
#define Lbumper 13
#define rightSensor A1
#define middleSensor A2
#define leftSensor A0

//one wheelturn is 20
//one intersection is 4

#define backup 5
#define spinny 18
#define turn 10
#define intersection 4
#define wheelTurn 30
#define Lvroom 170
#define Rvroom 162
#define MAX 225
#define rotate 10
#define Lthresh 910
#define Mthresh 910
#define Rthresh 910

int prev_L = 0;
int prev_R = 0;
int next_L = 0;
int next_R = 0;

Servo grip, tilt, pan;

int mag[3][54] = {{3,0,2,0,0,1,0,3,0,
              0,1,0,4,0,0,3,0,1,0,0,
              0,1,0,1,0,5,0,0,4,0,1,0,1,0,
              0,4,0,2,0,0,1,0,4,0,
              0,5,0,2,0,0,1,0,5,0},
              
              {2,0,3,0,0,2,0,2,0,
              0,2,0,3,0,0,2,0,2,0,0,
              0,6,0,0,6,0,
              0,5,0,1,0,1,0,0,1,0,5,0,
              0,5,0,1,0,1,0,0,1,0,5,0,0,0,0,0},
              
              {1,0,4,0,0,3,0,1,0,
              0,3,0,2,0,0,1,0,3,0,0,
              0,1,0,1,0,5,0,0,4,0,1,0,1,0,
              0,5,0,2,0,0,1,0,5,0,
              0,4,0,2,0,0,1,0,4,0}};

char act[3][54] = {{'f','l','f','p','s','f','r','f','d',
               's','f','r','f','p','s','f','l','f','d','t',
               's','f','l','f','r','f','p','s','f','l','f','r','f','d',
               's','f','l','f','p','s','f','r','f','d',
               's','f','l','f','p','s','f','r','f','d'},
               
               {'f','l','f','p','s','f','r','f','d',
               's','f','r','f','p','s','f','l','f','d','t',
               's','f','p','s','f','d',
               's','f','l','f','r','f','p','q','f','r','f','d',
               's','f','r','f','l','f','p','w','f','l','f','d','x','x'},
               
               {'f','l','f','p','s','f','r','f','d',
               's','f','r','f','p','s','f','l','f','d','t',
               's','f','r','f','l','f','p','s','f','r','f','l','f','d',
               's','f','r','f','p','s','f','l','f','d',
               's','f','r','f','p','s','f','l','f','d'}};

void setup() {
  prev_L = digitalRead(Lencoder);
  prev_R = digitalRead(Rencoder);

  grip.attach(10);
  tilt.attach(9);
  pan.attach(8);
  grip.write(40);
  tilt.write(165);
  pan.write(70);
  
  pinMode(Ldirection,OUTPUT);
  pinMode(Lspeed,OUTPUT);
  pinMode(Rdirection,OUTPUT);
  pinMode(Rspeed,OUTPUT);
  pinMode(Lencoder,INPUT);
  pinMode(Rencoder,INPUT);
  pinMode(Lbumper,INPUT);
  pinMode(Rbumper,INPUT);
  pinMode(sensor, INPUT);
  myIRserial.attach(sensor, -1);
  Serial.begin (9600);
  
}

int val = -1;
 
void loop()
{


int location = getlocation();

/*
while(digitalRead(Lbumper)==1)
{}
val = 2;
*/
  
  for ( int i=0;i<54;i++)
  {
    char alpha = act[char(val)][i];    
      if (alpha=='f')
        forward(mag[char(val)][i]);
      else if(alpha=='l')
        left();
      else if(alpha=='r')
        right();
      else if(alpha=='p')
        pickup();
      else if(alpha=='d')
        dropoff();
      else if(alpha=='s')
        spin();  
      else if(alpha=='q')
      spinnyL();
      else if(alpha=='w')
      spinnyR();
      else if(alpha =='t')
      delay(60000);
   }   
}

void forward(int magnitude)
{
  int count = 0;
  digitalWrite(Ldirection,HIGH);
  digitalWrite(Rdirection,HIGH);
  while(count < magnitude)
  {
    
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    //plus each vroom with 25
    analogWrite(Lspeed,Lvroom+25);
    analogWrite(Rspeed,Rvroom+25);
    if(L > Lthresh && M > Mthresh && R > Rthresh)
    {
      encoder(intersection);
      count=count+1;    
    }
    if(digitalRead(Lbumper) == 0 && digitalRead(Rbumper)==0)
    {
      break;
    }
    else if(L > Lthresh && M < Mthresh && R < Rthresh)
      analogWrite(Lspeed,0);
    else if(L < Lthresh && M < Mthresh && R > Rthresh)
      analogWrite(Rspeed,0);
  } 
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);
}

void left()
{
  digitalWrite(Rdirection,HIGH);
  analogWrite(Rspeed,Rvroom);
  digitalWrite(Ldirection,LOW);
  analogWrite(Lspeed,Lvroom/2);
  encoder(turn);
  while(true)
  {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    if(L < Lthresh && M > Mthresh && R < Rthresh)
    break;
  }
  analogWrite(Rspeed,0); 
  analogWrite(Lspeed,0);
}

void right()
{
  digitalWrite(Ldirection,HIGH);
  digitalWrite(Rdirection,LOW);
  analogWrite(Rspeed,Rvroom/2);
  analogWrite(Lspeed,Lvroom);
  encoder(turn);
  while(true)
  {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    if(L < Lthresh && M > Mthresh && R < Rthresh)
    break;
  }
  analogWrite(Lspeed,0); 
  analogWrite(Rspeed,0);
}

void spin()
{
  digitalWrite(Ldirection,HIGH);
  digitalWrite(Rdirection,LOW);
  analogWrite(Lspeed,Lvroom-30);
  analogWrite(Rspeed,Rvroom-22);
  encoder(spinny);
  while(true)
  {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    if(L < Lthresh && M < Mthresh && R > Rthresh)
    break;
  }
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0); 
  
}

void encoder(int turns)
{
  int wheelCountL = 0;
  int wheelCountR = 0;
  prev_L = digitalRead(Lencoder);
  prev_R = digitalRead(Rencoder);  
  while (wheelCountL < turns && wheelCountR < turns)
  {
    next_L = digitalRead(Lencoder);
    next_R = digitalRead(Rencoder);
    if (prev_L != next_L)
      wheelCountL++;
    if (prev_R != next_R)
      wheelCountR++;
    prev_L=next_L;
    prev_R=next_R;
  }
}


void pickup()
{
  digitalWrite(Ldirection,LOW);
  digitalWrite(Rdirection,LOW);
  analogWrite(Lspeed,Lvroom-70);
  analogWrite(Rspeed,Rvroom-62);
  encoder(backup);
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);
  delay(800);
  tilt.write(78);
  pan.write(70);
  delay(500);
  grip.write(120);
  delay(500);
  tilt.write(220);
  digitalWrite(Ldirection,LOW);
  digitalWrite(Rdirection,LOW);  
  analogWrite(Lspeed,Lvroom);
  analogWrite(Rspeed,Rvroom);
  encoder(10);
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);  
}

void dropoff()
{
  tilt.write(220);
  delay(300);
  tilt.write(60);
  delay(50);
  grip.write(40);    
  delay(500);
  tilt.write(145);
  digitalWrite(Ldirection,LOW);
  digitalWrite(Rdirection,LOW);  
  analogWrite(Lspeed,Lvroom);
  analogWrite(Rspeed,Rvroom);
  encoder(6);
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);    
}

void spinnyL()
{
  digitalWrite(Ldirection,HIGH);
  digitalWrite(Rdirection,LOW);
  analogWrite(Lspeed,Lvroom);
  analogWrite(Rspeed,Rvroom);
  encoder(5);
  while(true)
  {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    if(L < Lthresh && M < Mthresh && R > Rthresh)
    break;
  }
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);
}

void spinnyR()
{
  digitalWrite(Ldirection,HIGH);
  digitalWrite(Rdirection,LOW);
  analogWrite(Lspeed,Lvroom);
  analogWrite(Rspeed,Rvroom);
  encoder(30);
  while(true)
  {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor);
    if(L < Lthresh && M < Mthresh && R > Rthresh)
    break;
  }
  analogWrite(Lspeed,0);
  analogWrite(Rspeed,0);
}

void IRcalibrate()
{
    while(digitalRead(Lbumper)==1)
    {
    int L = analogRead(leftSensor);
    int M = analogRead(middleSensor);
    int R = analogRead(rightSensor); 
    Serial.print("left ");
    Serial.print(L);
    Serial.print("middle ");
    Serial.print(M);
    Serial.print("right ");
    Serial.println(R);
    delay(200);
    }  
}

void IR()
  {
    while(true)
    {
    int val = myIRserial.receive(200);
    Serial.println(char(val));
    if (char(val)==1||char(val)==0||char(val)==2)
    break;
    }  
    val = char(val); 
  }

int getlocation()
{

  while(1)
  {
    int IRinput = myIRserial.receive(200);
    if (IRinput == 48)
    {
      val = 0;
      return val;
    }
    if (IRinput == 49)
    {
      val = 1;
      return val;
    }
    if (IRinput == 50)
    {
      val = 2;
      return val;
    }
}
}
  

