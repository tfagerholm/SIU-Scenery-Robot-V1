#include <SabertoothSimplified.h>

SabertoothSimplified ST;

void setup() {

  Serial1.begin(9600);
  SabertoothTXPinSerial.begin(9600); // This is the baud rate you chose with the DIP switches.

  ST.drive(0); // The Sabertooth won't act on mixed mode until
  ST.turn(0);  // it has received power levels for BOTH throttle and turning, since it
               // mixes the two together to get diff-drive power levels for both motors.
               // So, we set both to zero initially.
}


void loop() {

#define INPUT_SIZE 30

// Get next command from Serial (add 1 for final 0)
char input[INPUT_SIZE + 1];
byte size = Serial1.readBytes(input, INPUT_SIZE);
// Add the final 0 to end the C string
input[size] = 0;

// Read each command pair 
char* command = strtok(input, "&");
while (command != 0)
{
    // Split the command in two values
    char* separator = strchr(command, ':');
    if (separator != 0)
    {        
        // Actually split the string in 2: replace ':' with 0
        *separator = 0;
        int xval = atoi(command);
        ++separator;
        int yval = atoi(separator);

 
 xval = map(xval, -32768, 32768, -50, 50);     // scale it to use it with the servo (value between 0 and 180)
 yval = map(yval, -32768, 32768, -50, 50);
 
 ST.motor(2,xval);
 ST.motor(1,yval);
 delay(10);

    }
    // Find the next command in input string
    command = strtok(0, "&");
}
  


}

