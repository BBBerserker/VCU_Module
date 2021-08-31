#include <Serial_CAN_Module_master.h>

Serial_CAN can;

#define can_tx 2
#define can_rx 3

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    while (!Serial);
    can.begin(can_tx, can_rx, 9600); // tx, rx
    if (can.canRate(CAN_RATE_250)){
        Serial.println("set can rate ok");
    }
    else{
        Serial.println("set can rate fail");
    }
    Gear_Setup();
}
void loop() {
  // put your main code here, to run repeatedly:
  Gear_CAN_loop();
}


/*  Gear_Process_Function
 *  08/27
 *  input : Key-ON Value, BPS value, vehicle_speed
 *  output : CAN Message (Motor Output, Motor Direction)
 *  int s = Shift Lever Position
 */
const int gear_drive = 5;
const int gear_neutral = 4;
const int gear_reverse = 3; 

const int boozer = 11; // boozer PIN

char gear_value; // 기어 값 

int vehicle_speed_forward; // 전진 방향 속도
int vehicle_speed_reverse; // 후진 방향 속도
int sw_BPS; // Brake pedal Value

void Gear_Setup(){
  // gear PIN Setup
  pinMode(gear_reverse, OUTPUT);
  pinMode(gear_neutral, OUTPUT);
  pinMode(gear_drive, OUTPUT);
  pinMode(sw_BPS, OUTPUT); // BPS는 Gear에서 setup하지 않음 현재는 예제로 사용함
  pinMode(boozer, OUTPUT); 

  // gear switch 초기화
  digitalWrite(gear_neutral, LOW); 
  digitalWrite(gear_reverse, LOW);
  digitalWrite(gear_drive, LOW);
  gear_value = 'p';
}

void Gear_CAN_loop(){
   /*  dta[0] = direction ==> 0 : neutral , 1 : forward , 2 : reverse 
       dta[1] = motot enable/disable ==> 0 : disable , 1 : enable  */
   unsigned char dta[2] = {0x00, 0x00};

   // Gear R 포지션 
    if(digitalRead(gear_reverse) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed_forward < 3){
    // 모터 방향 후진 셋팅
    warning_boozer(vehicle_speed_forward);
    Serial.println("Gear R");
    dta[0] = 0x02;
    dta[1] = 0x01;
  }
  //Gear N 포지션
  else if(digitalRead(gear_neutral) == HIGH && digitalRead(sw_BPS) == HIGH){
    Serial.println("Gear N");
    dta[0] = 0x00;
    dta[1] = 0x01;
  } 
  // Gear D 포지션
  else if(digitalRead(gear_drive) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed_reverse < 3){
    // 모터 방향 전진 셋팅
    warning_boozer(vehicle_speed_reverse);
    Serial.println("Gear D");
    dta[0] = 0x01;
    dta[1] = 0x01;
   
  } // Gear P 포지션 
  else if(digitalRead(sw_BPS) == HIGH && vehicle_speed_reverse < 3 && vehicle_speed_forward < 3){
    // 모터 토크 0 셋팅, MCU 모터방향 전환 확인
    warning_boozer(vehicle_speed_forward);
    warning_boozer(vehicle_speed_reverse);
    Serial.println("Gear P");
    dta[0] = 0x00;
    dta[1] = 0x00;
    
  }
  can.send(0x4d1, 0, 0, 2, dta);
  delay(100);
  return;
}

void warning_boozer(int vel){
  if( vel >= 4 ){
    tone(boozer, 659);
  } 
  Serial.println(" Shifting Warning !!");
  return;
}
