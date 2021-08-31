// Module_name: Key_ON
// 12 : Start_Button (Arduino Switch)
// keyON CAN_ID = 0x1A
// Written by Jung Ju Im, 07/30/21

void setup() {
  Key_Setup();
}

void loop() {
  Key_CAN_loop();
}

// 필요 전역 변수들
const int sw_acc = 4;
const int sw_ig1 = 7;
const int sw_start = 8;
// off = o, ACC = a, IG1 = i, start = s
static char key_status = 'o';

/* Required Condition */
// Key_ON, Relay_Main, VCU_Ready, VCU_ShiftPOS, VCU_BPS1
const int key_on = 1;
const int relay_main = 1;
const int vcu_ready = 1;
static int vcu_shiftpos = 2; // CAN 통신으로 bit값을 받아 int 값으로 치환


// o = key off , a = key acc, i = key ig1, s = key start 
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
