#include <Serial_CAN_Module_master.h>
#include <SoftwareSerial.h>

Serial_CAN can;

#define can_tx 2
#define can_rx 3

/* 
 * vehicle_speed_forward
 * vehicle_speed_reverse
 * sw_BPS
 * key_status
 * gear_value
 * relay_main
 * key_on
 * (태완)
 * 후진기어, 브레이크 페달
 * (지섭)
 * gear_value
 * map_gear
 * map_rpm
 * rpm
 * map_accel
 * APS
 * speed
 * torque
 */

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    while (!Serial)
      ;
    can.begin(can_tx, can_rx, 9600); // tx, rx
    
    if (can.canRate(CAN_RATE_250)){
        Serial.println("set can rate ok");
    }
    else{
        Serial.println("set can rate fail");
    }
    Key_Setup();
    Gear_Setup();
}
void loop() {
  // put your main code here, to run repeatedly:
  Key_CAN_loop();
  Gear_CAN_loop();
}


/*  Gear_Process_Function
 *  08/27
 *  input : Key-ON Value, BPS value, vehicle_speed
 *  output : CAN Message (Motor Output, Motor Direction)
 *  int s = Shift Lever Position
 */
/* Required Condition */
// Key_ON, Relay_Main, VCU_Ready, VCU_ShiftPOS, VCU_BPS1
const int key_on = 1;
const int relay_main = 1;
const int vcu_ready = 1;
static int vcu_shiftpos = 2; // CAN 통신으로 bit값을 받아 int 값으로 치환

 
// 필요 전역 변수들 ( 임의로 설정 )
const int sw_acc = 7;
const int sw_ig1 = 6;
const int sw_start = 5;
// off = o, ACC = a, IG1 = i, start = s
static char key_status = 'o';
 
const int gear_drive = 6;
const int gear_neutral = 7;
const int gear_reverse = 8; 

const int boozer = 11; // boozer PIN

char gear_value; // 기어 값 

//int vehicle_speed_forward; // 전진 방향 속도
//int vehicle_speed_reverse; // 후진 방향 속도
int vehicle_speed; // 속도
int sw_BPS; // Brake pedal Value

void Key_Setup(){
  pinMode(sw_acc, OUTPUT);
  pinMode(sw_ig1, OUTPUT);
  pinMode(sw_start, OUTPUT);

  digitalWrite(sw_acc, LOW); 
  digitalWrite(sw_ig1, LOW);
  digitalWrite(sw_start, LOW);
  
  key_status = 'o';
  return;
}

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

void Key_CAN_loop(){
   // Key ACC 상태
    if(key_status == 'o' && digitalRead(sw_acc) == HIGH && relay_main == 1 && key_on == 1 ){
      key_off_to_acc();
      
      // Key IG1 ==> 파워 Enable BMS와 협조 MCU에 SOC확인후 모터 제어
    } else if( key_status == 'a' && digitalRead(sw_ig1) == HIGH && relay_main == 1 && key_on == 1 ){
      key_acc_to_ig1();
      
     // Key Position Start => 드라이빙 모드
    } else if( key_status == 'i' && digitalRead(sw_start) == HIGH && relay_main == 1 && key_on == 1
                && vcu_shiftpos == 2 && vcu_shiftpos == 4 ){
      key_ig1_to_start();

      // Key Position OFF ==> 전원 OFF
    } else if( key_status == 'o' ) {
      key_status = 'o';
      Serial.println("key Position = OFF");
    }
    return;   
}

void Gear_CAN_loop(){
   /*  dta[0] = direction ==> 0 : neutral , 1 : forward , 2 : reverse 
       dta[1] = motot enable/disable ==> 0 : disable , 1 : enable  */
   unsigned long id = 0x4d;
   int dlc = 8;
   unsigned char dta[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
   if( key_status == 's'){
     // Gear R 포지션 
      if(digitalRead(gear_reverse) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed < 3){
      // 모터 방향 후진 셋팅
      warning_boozer(vehicle_speed);
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
    else if(digitalRead(gear_drive) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed < 3){
      // 모터 방향 전진 셋팅
      warning_boozer(vehicle_speed);
      Serial.println("Gear D");
      dta[0] = 0x01;
      dta[1] = 0x01;
     
    } // Gear P 포지션 
    else if(digitalRead(sw_BPS) == HIGH && vehicle_speed < 3){
      // 모터 토크 0 셋팅, MCU 모터방향 전환 확인
      warning_boozer(vehicle_speed);
      Serial.println("Gear P");
      dta[0] = 0x00;
      dta[1] = 0x00;
    }
  }
  can.send(id, 0, 0, dlc, dta);
//  for(int i=0; i<8; i++)
//        {
//            Serial.print("0x");
//            Serial.print(dta[i], HEX);
//            Serial.print('\t');
//        }

  delay(100);
  return;
}

//void can_loop(){
//  if(can.recv(&id, dta))
//    {
//        Serial.print("GET DATA FROM ID: ");
//        Serial.println(id);
//        for(int i=0; i<8; i++)
//        {
//            Serial.print("0x");
//            Serial.print(dta[i], HEX);
//            Serial.print('\t');
//        }
//        Serial.println();
//    }
//}

void key_off_to_acc(){
  key_status = 'a';
  Serial.println("key Position = ACC");
  return;
}
void key_acc_to_ig1(){
   key_status = 'i';
   Serial.println("key Position = IG1");
   return;
}
void key_ig1_to_start(){
  key_status = 's';
  Serial.println("key Position = Start");
  return;
}

void warning_boozer(int vel){
  if( vel >= 4 ){
    tone(boozer, 659);
  } 
  Serial.println(" Shifting Warning !!");
  return;
}
