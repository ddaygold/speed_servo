#include "globals.h"

void setup(){
  pterm = load_controller_tuning(PTERM);
  iterm = load_controller_tuning(ITERM);
  dterm = load_controller_tuning(DTERM);
  //We need to update the controler after
  //loading from EEPROM
  controller.SetTunings(pterm,iterm,dterm);
  Serial.begin(9600);
  pinMode(PINA, INPUT);
  pinMode(PINB, INPUT);
  pinMode(ADDRESS_PIN, INPUT);
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH); //so we can jumper over to pin 12
  pinMode(VICTOR_PIN, OUTPUT);
  victor.attach(VICTOR_PIN);
  victor.writeMicroseconds(1510);
  attachInterrupt(0,spin,RISING);
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
    last_tuning_update = millis();
  }
  if(controller.Compute() || controller.GetMode() == MANUAL){
    if(controller.GetMode() != MANUAL){
      int output = (int)controller_output + 1510;
      victor.writeMicroseconds(output);
    } else {
      victor.writeMicroseconds(1510);
    }
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
    if(controller.GetMode() == MANUAL){
      Serial.println("****ESTOP****");
    }
    ticks = 0;
  }
}


void receive(int incoming){
  switch(Wire.read()){
  case SPEED:
    if(controller.GetMode() == MANUAL){
      controller.SetMode(AUTOMATIC);
    }
    return setParam(incoming, &speed);
    break;
  case PTERM:
    setParam(incoming, &pterm);
    save_controller_tuning(PTERM,pterm);
    break;
  case ITERM:
    setParam(incoming, &iterm);
    save_controller_tuning(ITERM,iterm);
    break;
  case DTERM:
    setParam(incoming, &dterm);
    save_controller_tuning(DTERM,dterm);
    break;
  case ESTOP:
    controller.SetMode(MANUAL);
    speed = 0;
    controller_output = 0;
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
double load_controller_tuning(int tuning){
  double out;
  byte * reassembled = (byte *)&out;
  for(int i = 0; i < sizeof(double); ++i){
    *reassembled = EEPROM.read(sizeof(double)*tuning + i);
    reassembled++;
  }
  return out;
}
void save_controller_tuning(int tuning, double value){
  byte * disassembled = (byte *)&value;
  for(int i = 0; i < sizeof(double); ++i){
    EEPROM.write(sizeof(double)*tuning + i, *disassembled);
    disassembled++;
  }
}