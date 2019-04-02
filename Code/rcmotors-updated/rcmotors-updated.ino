#include <Sabertooth.h>

#include <SPI.h>
#include <symax_protocol.h>

const int deadspace = 0;
const int STpin = 1;
const int ESTOP_PIN = 2;
const int dti = 10;
const int Brake = 4;

int Accel = 0;
int AMult = 1;
int SpeedSwitch = 0;
int Throttle = 0;
int ThrottleDiv = 4;
int ThrottleConst = 20;
int LRStick = 0;
int FBStick = 0;
int LMotorPWM = 0;
int RMotorPWM = 0;
int LMotorOUT = 0;
int RMotorOUT = 0;
int LTargetPWM = 0;
int RTargetPWM = 0;
int Diff = 0;
int LASignalDown = 0;
int LASignalUp = 0;
int LASpeed = 127;
int LAPWM = 0;

nrf24l01p wireless; 
symaxProtocol protocol;

unsigned long time = 0;
unsigned long brakeDelay = 500;
unsigned long brakeDelayMark = 0;
unsigned long brakeDelayTime = 0;

Sabertooth ST(128, Serial1);
Sabertooth LA(128, Serial3);


void setup() {

  Serial1.begin(9600);
  Serial3.begin(9600);
  Serial.begin(115200);
  // SS pin must be set as output to set SPI to master !
  pinMode(SS, OUTPUT);
  pinMode(Brake, OUTPUT);
  pinMode(ESTOP_PIN, INPUT);
  // Set CE pin to 10 and CS pin to 9
  wireless.setPins(53,49);
    
  // Set power (PWRLOW,PWRMEDIUM,PWRHIGH,PWRMAX)
  wireless.setPwr(PWRMAX);
  
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
  
  if ( value != BOUND_NEW_VALUES && value != BOUND_NO_VALUES )
  {
    ST.motor(1,0);
    ST.motor(2,0);
    LMotorPWM = 0;
    RMotorPWM = 0;
    digitalWrite(Brake, LOW);
    delay(dti);
    return;
  }
  else
  {
/*      if (value == BOUND_NO_VALUES)
      {
        delay(dti);
        return;
      }
      else*/ if ( value == BOUND_NEW_VALUES )
      {
      LASignalDown = rxValues.video;
      LASignalUp = rxValues.picture;
      if ( LASignalUp == 1 && LASignalDown == 1 )
      {
        LAPWM = 0;
      }
      else if ( LASignalDown == 1 && LASignalUp == 0 )
      {
        LAPWM = -LASpeed;
      }      
      else if ( LASignalDown == 0 && LASignalUp == 1 )
      {
        LAPWM = LASpeed;
      }
      else if ( LASignalDown == 0 && LASignalUp == 0 )
      {
        LAPWM = 0;
      }
      LA.motor(1,LAPWM);
      LA.motor(2,LAPWM);
      
      Accel = ( rxValues.highspeed * AMult ) + AMult;
      Throttle = ( rxValues.throttle / ThrottleDiv ) + ThrottleConst;
      
      LRStick = rxValues.yaw;
      FBStick = rxValues.pitch;

      LTargetPWM = FBStick - LRStick;
      RTargetPWM = FBStick + LRStick;
      }
      
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
      if ( LMotorPWM > 0 )
      {
        LMotorOUT = map( LMotorPWM, 1, 127, 1, Throttle );
      }
      else if ( LMotorPWM < 0 )
      {
        LMotorOUT = map( LMotorPWM, -1, -127, -1, -Throttle );
      }
      else
      {
        LMotorOUT = 0;
      }
      if ( RMotorPWM > 0 )
      {
        RMotorOUT = map( RMotorPWM, 1, 127, 1, Throttle );
      }
      else if ( RMotorPWM < 0 )
      {
        RMotorOUT = map( RMotorPWM, -1, -127, -1, -Throttle );
      }
      else
      {
        RMotorOUT = 0;
      }
            
      if ( RMotorOUT != 0 && LMotorOUT != 0 )
      {
        digitalWrite(Brake, HIGH);
      }
            
      ST.motor(1, LMotorOUT);
      ST.motor(2, RMotorOUT);
      if ( RMotorOUT == 0 && LMotorOUT == 0 )
      {
        if ( brakeDelayMark == 0 )
        {
          brakeDelayMark = millis();
        }
        brakeDelayTime = millis() - brakeDelayMark;
        if ( brakeDelayTime >= brakeDelay )
        {
        digitalWrite(Brake, LOW);
        brakeDelayMark = 0;  
        }
      }
         
      delay(dti);  
    
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
