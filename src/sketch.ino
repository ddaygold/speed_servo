#include "globals.h"

void setup(){
  //Load saved tunings from EEPROM, initalize the controller
  pterm = load_controller_tuning(PTERM);
  iterm = load_controller_tuning(ITERM);
  dterm = load_controller_tuning(DTERM);
  controller.SetTunings(pterm,iterm,dterm);
  controller.SetMode(AUTOMATIC);
  controller.SetOutputLimits(-1*PWM_WIDTH,PWM_WIDTH);
  controller.SetSampleTime(100);

  Serial.begin(9600);
  //A and B channels of the Quad encoder
  pinMode(PINA, INPUT);
  pinMode(PINB, INPUT);
  attachInterrupt(0,spin,RISING);

  pinMode(VICTOR_PIN, OUTPUT);
  victor.attach(VICTOR_PIN);
  victor.writeMicroseconds(PWM_CENTER); 

  /*
     The Arduino has a ground pin, then pin 13, then pin 12
     We set 13 as the input, and set 12 high so we can jumper to 
     either GND or 12 (HIGH) to set the i2c address 
   */
  pinMode(ADDRESS_PIN, INPUT);
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH); 
  int address = (digitalRead(ADDRESS_PIN) == HIGH)?2:1;
  Serial.print("Address: ");
  Serial.println(address);
  Wire.begin(address);
  Wire.onReceive(receive);//This requires a protocol to understand which parameter is being changed
  Wire.onRequest(writeTicks);//This is always the speed
}

void loop(){
  /*
     We only set the external tuning variables in the i2c
     receive routine. We load the controller with new values periodically
     this assumes that you are setting the values infrequently, so the lag period 
     doesn't have a large impact.
   */
  if(millis() - last_tuning_update > 1000){
    controller.SetTunings(pterm,iterm,dterm);
    last_tuning_update = millis();
  }
  if(controller.Compute() || controller.GetMode() == MANUAL){
    if(controller.GetMode() != MANUAL){
      int output = (int)controller_output + PWM_CENTER;
      victor.writeMicroseconds(output);
    } else {
      //The controller is only in MANUAL when it has been estopped
      //Therefore, write the Victor's zero value
      victor.writeMicroseconds(PWM_CENTER);
    }
    printDebug();
    last_ticks = ticks;
    ticks = 0;
  }
}


void receive(int incoming){
  switch((enum command)Wire.read()){
  case SPEED:
    if(controller.GetMode() == MANUAL){
      controller.SetMode(AUTOMATIC);
    }
    setParam(incoming, &speed);
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

void writeTicks(){
  Wire.write((char *) &last_ticks, sizeof(int));
}

void printDebug(){
    //All serial printing is for debugging only.
    //Control information happens over i2c
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
  byte buff[sizeof(double)];
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
