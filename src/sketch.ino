#include "globals.h"

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
  controller.SetMode(AUTOMATIC);
  controller.SetOutputLimits(-635,635);
}

void loop(){
  /*
    If the extremes are 870 microsecond and 2140 microseconds, with
    a center at 1510 microseconds, we have a width of 635 microseconds
  */
  if(millis() - last_tuning_update > 1000){
    controller.SetTunings(pterm,iterm,dterm);
    Serial.print("Speed: ");
    Serial.print(speed);
    Serial.print(" ticks: ");
    Serial.print(ticks);
    Serial.print(" p: ");
    Serial.print(pterm);
    Serial.print(" i: ");
    Serial.print(iterm);
    Serial.print(" d: ");
    Serial.print(dterm);
    Serial.print(" OUTPUT: ");
    Serial.println(controller_output);
    last_tuning_update = millis();
  }
  if(controller.Compute()){
    victor.writeMicroseconds((int)controller_output + 1510);
    ticks = 0;
  }
  delay(10);
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