void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

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

void Gear_CAN_loop(int s){

   // Gear R 포지션 
  if(digitalRead(gear_reverse) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed_forward < 3) 
  {
    warning_boozer(vehicle_speed_forward);
    // canMsg.data[0] = 0x00; // bit 16 gear 1 0 0 (R)
    // canMsg.data[1] = 0x00; // bit 17 gear
    // canMsg.data[2] = 0x01; // bit 18 gear
    // canMsg.data[3] = 0x00; // bit 32 Motor Dircetion ( 2 = reverse )
    // canMsg.data[4] = 0x01; // bit 33 Motor Dircetion 
    // canMsg.data[5] = 0x01; // bit 34 Motor Enable ( 1 = motor enable )
    // canMsg.data[6] = 0x00; // bit 35 Motor Enable
    Serial.println("Gear R");
  }
  //Gear N 포지션
  else if(digitalRead(gear_neutral) == HIGH && digitalRead(sw_BPS) == HIGH)
  {
    // canMsg.data[0] = 0x01; // bit 16 gear = 0 1 1 (N)
    // canMsg.data[1] = 0x01; // bit 17 gear
    // canMsg.data[2] = 0x00; // bit 18 gear
    // canMsg.data[3] = 0x00; // bit 32 Motor Dircetion ( 0 = init )
    // canMsg.data[4] = 0x00; // bit 33 Motor Dircetion 
    // canMsg.data[5] = 0x00; // bit 34 Motor Enable ( 0 = motor disable )
    // canMsg.data[6] = 0x00; // bit 35 Motor Enable
        Serial.println("Gear N");

  } 
  // Gear D 포지션
  else if(digitalRead(gear_drive) == HIGH && digitalRead(sw_BPS) == HIGH && vehicle_speed_reverse < 3)
  {
    // 모터 방향 전진 셋팅
    warning_boozer(vehicle_speed_reverse);
    // canMsg.data[0] = 0x00; // bit 16 gear 0 1 0 (D)
    // canMsg.data[1] = 0x01; // bit 17 gear
    // canMsg.data[2] = 0x00; // bit 18 gear
    // canMsg.data[3] = 0x01; // bit 32 Motor Dircetion ( 1 = forward )
    // canMsg.data[4] = 0x00; // bit 33 Motor Dircetion 
    // canMsg.data[5] = 0x01; // bit 34 Motor Enable ( 1 = motor enable )
    // canMsg.data[6] = 0x00; // bit 35 Motor Enable
    Serial.println("Gear D");

  } // Gear P 포지션 
  else if(digitalRead(sw_BPS) == HIGH && vehicle_speed_reverse < 3 && vehicle_speed_forward < 3)
  {
    // 모터 토크 0 셋팅, MCU 모터방향 전환 확인
    warning_boozer(vehicle_speed_forward);
    warning_boozer(vehicle_speed_reverse);
    // canMsg.data[0] = 0x01; // bit 16 gear = 1 0 1 (P)
    // canMsg.data[1] = 0x00; // bit 17 gear
    // canMsg.data[2] = 0x01; // bit 18 gear
    // canMsg.data[3] = 0x00; // bit 32 Motor Dircetion ( 0 = init )
    // canMsg.data[4] = 0x00; // bit 33 Motor Dircetion 
    // canMsg.data[5] = 0x00; // bit 34 Motor Enable ( 0 = motor disable )
    // canMsg.data[6] = 0x00; // bit 35 Motor Enable
    Serial.println("Gear P");

  }
}

void warning_boozer(int vel){
  if( vel >= 4 ){
    tone(boozer, 659);
  } 
  Serial.println(" Shifting Warning !!");
  return;
}