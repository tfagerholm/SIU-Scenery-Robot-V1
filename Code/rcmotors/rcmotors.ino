#include <SabertoothSimplified.h>
#include <SoftwareSerial.h>

#include <SPI.h>
#include <symax_protocol.h>

const int deadspace = 0;
const int STpin = 1;
const int ESTOP_PIN = 2;
const int dti = 5;
const int AMult = 1;
const int LBrake = 3;
const int RBrake = 4;

int Accel = 0;
int SpeedSwitch = 0;
int Throttle = 0;
int LRStick = 0;
int FBStick = 0;
int LMotorPWM = 0;
int RMotorPWM = 0;
int LMotorOUT = 0;
int RMotorOUT = 0;
int LTargetPWM = 0;
int RTargetPWM = 0;
int Diff = 0;
int Roll = 0;

nrf24l01p wireless; 
symaxProtocol protocol;

unsigned long time = 0;
SoftwareSerial SWSerial(NOT_A_PIN, 11); // RX on no pin (unused), TX on pin 11 (to S1).

SabertoothSimplified ST(SWSerial);


void setup() {

  SWSerial.begin(9600);
  Serial.begin(115200);
  // SS pin must be set as output to set SPI to master !
  pinMode(SS, OUTPUT);
  pinMode(RBrake, OUTPUT);
  pinMode(LBrake, OUTPUT);
  pinMode(ESTOP_PIN, INPUT);
  // Set CE pin to 10 and CS pin to 9
  wireless.setPins(53,49);
    
  // Set power (PWRLOW,PWRMEDIUM,PWRHIGH,PWRMAX)
  wireless.setPwr(PWRLOW);
  
  protocol.init(&wireless);
  
  time = micros();
  
}

rx_values_t rxValues;

unsigned long newTime;

void loop() 
{
  time = micros();
  uint8_t value = protocol.run(&rxValues); 
  newTime = micros();
  SpeedSwitch = rxValues.highspeed;
  if ( value != BOUND_NEW_VALUES && value != BOUND_NO_VALUES )
  {
    ST.motor(1,0);
    ST.motor(2,0);
    LMotorPWM = 0;
    RMotorPWM = 0;
    digitalWrite(LBrake, HIGH);
    digitalWrite(RBrake, HIGH);
    delay(dti);
    return;
  }
  else
  {
      if (value == BOUND_NO_VALUES)
      {
        delay(dti);
        return;
      }
      else if ( value == BOUND_NEW_VALUES )
      {
      Roll = rxValues.roll;
      if ( SpeedSwitch == 1 )
      {
         Roll = 0;     
      }
      // Add Code for Roll to move Linear Actuators after we have the motor controller for them.
      // OR CONSIDER VIDEO DOWN AND PICTURE UP
      
      Throttle = rxValues.throttle;
      Accel = (( Throttle / 32)+AMult);
      
      LRStick = rxValues.yaw;
      FBStick = rxValues.pitch;

      LTargetPWM = FBStick - LRStick;
      RTargetPWM = FBStick + LRStick;
      
      Diff = LTargetPWM - LMotorPWM;
      if ( Diff > 0 )
      {
        if ( Diff > Accel )
        {
          LMotorPWM = LMotorPWM + Accel;
        }
        else
        {
          LMotorPWM = LTargetPWM;
        }
      }
      else if ( Diff < 0)
      {
        if ( Diff < ( -Accel ) )
        {
          LMotorPWM = LMotorPWM - Accel;
        }
        else
        {
          LMotorPWM = LTargetPWM;
        }
      }

      Diff = RTargetPWM - RMotorPWM;
      if ( Diff > 0 )
      {
        if ( Diff > Accel )
        {
          RMotorPWM = RMotorPWM + Accel;
        }
        else
        {
          RMotorPWM = RTargetPWM;
        }
      }
      else if ( Diff < 0)
      {
        if ( Diff < ( -Accel ) )
        {
          RMotorPWM = RMotorPWM - Accel;
        }
        else
        {
          RMotorPWM = RTargetPWM;
        }
      }
      LMotorOUT = LMotorPWM;              // I Fucked Up
      RMotorOUT = RMotorPWM;              // I just don't want to go back and replace these
      LMotorOUT = MaxTurn(LMotorOUT);
      RMotorOUT = MaxTurn(RMotorOUT);
      

      Serial.print( LMotorOUT );
      Serial.print( "\t" );
      Serial.println( RMotorOUT );
      if ( LMotorOUT != 0 )
      {
        digitalWrite(LBrake, LOW);
      }     
      if ( RMotorOUT != 0 )
      {
        digitalWrite(RBrake, LOW);
      }
            
      ST.motor(1, LMotorOUT);
      ST.motor(2, RMotorOUT);
      if ( LMotorOUT == 0 )
      {
        digitalWrite(LBrake, HIGH);
      }     
      if ( RMotorOUT == 0 )
      {
        digitalWrite(RBrake, HIGH);
      }
      delay(dti);  
    }  
  }
 
}

int MaxTurn ( int PWM )
{
  if ( PWM > 127 )
  {
    PWM = 127;
  }
  else if ( PWM < -127 )
  {
    PWM = -127;
  }
  return PWM;
}
