#include "MeOrion.h"
#include <Wire.h>
#include <SoftwareSerial.h>

#define MIN_RADIUS_VALUE 3

#define  FIND_THE_BALL          0
#define  WALKED_TURN_TO_BALL    1
#define  WALKED_AROUND_BALL     2
#define  PICKED_UP_BALL         3
#define  PICKED_UP_FINISH       4
#define  PICK_UP_TIME_EXPIRES   5

char table[32] = {0};
static volatile int count = 0;
volatile int pick_state = FIND_THE_BALL;

long nb_time_0;
long nb_time_1;
long nb_time_2;
long nb_time_3;
volatile unsigned int Radius_debounced_count = 0;
volatile unsigned int Radius_match_count = 0;
volatile unsigned Radius_debounced_value = 0;

SoftwareSerial softuart(13,12);
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);
MePort port(PORT_4);
Servo myservo1;  // create servo object to control a servo 
int servo1pin =  port.pin1();//attaches the servo on PORT_3 SLOT1 to the servo object

volatile int Angle = 0,Radius = 0;
volatile int Pre_Angle = 0,Pre_Radius = 0;

void setup() {
   Serial.begin(57600);
   softuart.begin(57600);
   pick_state = FIND_THE_BALL;
   myservo1.attach(servo1pin);  // attaches the servo on servopin1
   myservo_down();
   nb_time_0 = nb_time_1 = nb_time_2 = nb_time_3 = millis();
}

void myservo_up()
{
  myservo1.write(15); 
}

void myservo_down()
{
  myservo1.write(150); 
}

void parseCmd(char * cmd)
{
  if(cmd[0]=='T')
  { // gcode
    parseTcode(cmd);  
  }
}
void parseTcode(char * cmd)
{
  char * tmp;
  char * str;
  str = strtok_r(cmd, " ", &tmp);
  if(str[0] != 'T')
  {
    return;  
  }
  while(str!=NULL)
  {
    str = strtok_r(0," ", &tmp);
    if(str[0]=='R')
    {
      Radius = atoi(str+1);
      if(Radius >60)
      {
        Radius = 50;
      }
    }
    if(str[0]=='A')
    {
      Angle = atoi(str+1);

      #if 0
      if(Angle >160)
      {
        Angle = 160;
      }
      if(Angle < -160)
      {
        Angle = -160;
      }
      #endif
    }
  }
  Pre_Radius = Radius;
  Pre_Angle = Angle;
  Serial.print("Radius ");Serial.println(Radius);
  Serial.print("Angle ");Serial.println(Angle);
}

bool check_ball_debounced(void)
{
  bool ball_is_true = false;
  if(Radius < MIN_RADIUS_VALUE)
  {
     return false;
  }
  if(millis() - nb_time_1 >= 200)
  {
    nb_time_1 = millis();
    if(Radius_debounced_count == 0)
    {
      Radius_debounced_value = Radius;
    }
    if(abs(Radius - Radius_debounced_value) <= 2)
    {
      Radius_match_count++;
    }
    Radius_debounced_count ++;
  }

  if(Radius_debounced_count == 5)
  {
    Radius_debounced_count = 0;
    if(Radius_match_count > 2)
    {
      ball_is_true = true;
    }
    else
    {
      ball_is_true = false;
    }
    Radius_match_count = 0;
  }
  return ball_is_true;
}

void car_turn(int car_speed)
{
  motor1.run(car_speed); /* value: between -255 and 255. */
  motor2.run(car_speed); /* value: between -255 and 255. */
}   

void car_run(int car_speed)
{
  motor1.run(-car_speed); /* value: between -255 and 255. */
  motor2.run(car_speed); /* value: between -255 and 255. */
}

void car_stop()
{
  motor1.stop();
  motor2.stop(); 
}

void picking_state_machine(int state)
{
  Serial.print("picking_state_machine, state:");
  Serial.print(state,DEC);
  Serial.print("---Angle:");
  Serial.print(Angle,DEC);
  Serial.print("---Radius:");
  Serial.println(Radius,DEC);
  car_stop();
  switch(state)
  {
    case FIND_THE_BALL:
    {
      if(Radius <= MIN_RADIUS_VALUE)
      {
      }
      else
      {
        if(check_ball_debounced() == true)
        {
          pick_state = WALKED_TURN_TO_BALL;
        }
      }
      break; 
    }
    case WALKED_TURN_TO_BALL:
    {
      if(Radius == 0)
      {
        pick_state = FIND_THE_BALL;
      }
      if((Radius > 38) && (abs(Angle) <40))
      {
        pick_state = PICKED_UP_BALL;
      }
      else if(Angle > 30)
      {
        car_turn(-60);
        delay(40);
        car_stop();
      }
      else if(Angle < -30)
      {
        car_turn(60);
        delay(40);
        car_stop();
      }
      else
      {
        pick_state = WALKED_AROUND_BALL;  
      }
      break; 
    }
    case WALKED_AROUND_BALL:
    {
      if(abs(Angle) > 30)
      {
        car_stop();
        pick_state = WALKED_TURN_TO_BALL;
      }
      else if(Radius > 38)
      {
        car_stop();
        pick_state = PICKED_UP_BALL;
      }
      else
      {
        car_run(100);
      }
      break;
    }
    case PICKED_UP_BALL:
    {
      car_run(200);
      delay(500);
      car_stop();
      pick_state = PICKED_UP_FINISH;
      myservo_up();
      delay(1200);
      break;
    }
    case PICKED_UP_FINISH:
    {
      myservo_down();
      delay(3000);
      pick_state = FIND_THE_BALL;
      break;
    }
    case PICK_UP_TIME_EXPIRES:
      pick_state = FIND_THE_BALL;
      break;
    default:
      pick_state = FIND_THE_BALL;
      break;
  }   
}

void loop()
{    
  char readdata = 0;
  int i = 0;

  if(millis() - nb_time_2 >= 100)
  {
    nb_time_2 = millis();
    picking_state_machine(pick_state);
  }

  //这个循环用于超时检测，找到球之后，如果走到球面前的动作超过10s,则重新开始捡球
  if((pick_state > FIND_THE_BALL) && (pick_state < PICKED_UP_BALL ))
  {
    if(millis() - nb_time_0 >= 10000)
    {
      nb_time_0 = millis();
      myservo_down();
      pick_state = PICK_UP_TIME_EXPIRES;
    }
  }

  if (softuart.available())
  {
    while((readdata = softuart.read()) != (int)-1)
    {
      if(readdata == '#')
      {
        //Serial.print("  count "); Serial.print(count);
        //Serial.print("\r\n");
        parseCmd(table);
        memset(table,0,32);
        count = 0;
      }
      else
      {
        table[count++] = readdata;
        //Serial.print(readdata);
        //Serial.print(" ");
      }
    }
  }
}
