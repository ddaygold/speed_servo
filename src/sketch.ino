#include "globals.h"

void setup(){
  pinMode(PINA, INPUT);
  pinMode(PINB, INPUT);
  pinMode(ADDRESS_PIN, INPUT);
  pinMode(VICTOR_PIN, OUTPUT);
  victor.writeMicroseconds(1510);
  attachInterrupt(0,spin,CHANGE);
  Wire.begin(0b0001000+(digitalRead(ADDRESS_PIN) == HIGH)?1:0);
  Wire.onReceive(receive);
  Wire.onRequest(writeVoltage);
}

void loop(){
  delay(100);
  processPID();
}

void processPID(){
  /*
    If the extremes are 870 microsecond and 2140 microseconds, with
    a center at 1510 microseconds, we have a width of 635 microseconds
  */
  int difference = speed - ticks;
  int new_out = difference*pterm;
  constrain(new_out, -635, 635);
  new_out += 1510;
  victor.writeMicroseconds(new_out);
  current_speed = new_out;
  ticks = 0; 
}

void receive(int incoming){
  switch(Wire.read()){
  case SPEED:
    return setParam(incoming, &speed);
    break;
  case PTERM:
    return setParam(incoming, &pterm);
    break;
  case ITERM:
    return setParam(incoming, &iterm);
    break;
  case DTERM:
    return setParam(incoming, &dterm);
    break;
  }
}

void writeVoltage(){
  Wire.write(analogRead(VOLTAGE_PIN));
}
void setParam(int incoming, volatile int * dest){
  if(incoming != 3){
    return;
  }
  int intermediate = Wire.read() + (Wire.read() << 8);
  *dest = intermediate;
}

void spin(){
  if(digitalRead(PINA) == HIGH && digitalRead(PINB) == LOW){
    ticks++;
  } else {
    ticks--;
  }
}