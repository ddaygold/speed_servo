#include "globals.h"
#include <PID_v1.h>

void setup(){
  Serial.begin(9600);
  pinMode(PINA, INPUT);
  pinMode(PINB, INPUT);
  pinMode(ADDRESS_PIN, INPUT);
  pinMode(VICTOR_PIN, OUTPUT);
  victor.writeMicroseconds(1510);
  attachInterrupt(0,spin,CHANGE);
  int address = (digitalRead(ADDRESS_PIN) == HIGH)?2:1;
  Serial.print("Address: ");
  Serial.println(address);
  Wire.begin(address);
  Wire.onReceive(receive);
  Wire.onRequest(writeVoltage);
}

void loop(){
  delay(100);
  processPID();
  char output_buffer[100];
  snprintf(output_buffer, 100,
	   "Recv vals [p,i,d x1000]: {speed: %d, p: %d, i: %d, d: %d}",
	   speed, pterm*1000, iterm*1000, dterm*1000);
  Serial.println(output_buffer);
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
  if(incoming != sizeof(int) +1){
    return;
  }
  *dest = Wire.read() + (Wire.read() << 8);
}
void setParam(int incoming, volatile double * dest){
  if(incoming != sizeof(double)+1){
    return;
  }
  byte buff[4];
  for(int i = 0; i < sizeof(double); ++i){
    buff[i] = Wire.read();
  }
  *dest = *((double *)buff);
}

void spin(){
  if(digitalRead(PINA) == HIGH && digitalRead(PINB) == LOW){
    ticks++;
  } else {
    ticks--;
  }
}