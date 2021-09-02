#include <Serial_CAN_Module_master.h>
#include <SoftwareSerial.h>

Serial_CAN can;

#define can_tx 2 // tx of serial can module connect to D2
#define can_rx 3 // rx of serial can module connect to D3

// GEAR
int gear_Pin;
int map_gear;
int gear_value;
char gear;

// RPM
int rpm_Pin;
int map_rpm;
int rpm;

// Accel
int accel_Pin;
int map_accel;
int APS;

int speed;  // 속도(km/h)
int torque; // 토크 값

// GEAR D위치 APS-Speed 토크 값 배열 표
int D_Array[12][10] = {
    {0, 100, 110, 120, 130, 140, 150, 160, 170, 180},
    {10, 13, 12, 11, 10, 8, 7, 6, 5, 3},
    {20, 40, 37, 33, 29, 25, 22, 18, 14, 10},
    {30, 67, 61, 55, 48, 42, 36, 30, 23, 17},
    {40, 94, 85, 76, 68, 59, 50, 41, 33, 24},
    {50, 121, 110, 98, 87, 76, 65, 53, 42, 31},
    {60, 148, 134, 120, 106, 93, 79, 65, 51, 38},
    {70, 174, 158, 142, 126, 109, 93, 77, 61, 44},
    {80, 201, 183, 164, 145, 123, 108, 89, 70, 51},
    {90, 228, 207, 186, 164, 143, 122, 101, 79, 58},
    {100, 255, 231, 208, 184, 160, 136, 113, 89, 65},
    {110, 255, 231, 208, 184, 160, 136, 113, 89, 65}, // D_Array[11][j]=> 마지막 행 계산을 위해 넣은 데이터
};

// GEAR R위치 APS-Speed 토크 값 배열 표
int R_Array[10][6] = {
    {0, 20, 25, 30, 35, 40},
    {20, -28, -21, -14, -7, 0},
    {30, -57, -43, -28, -14, 0},
    {40, -85, -64, -43, -21, 0},
    {50, -113, -85, -57, -28, 0},
    {60, -142, -106, -71, -35, 0},
    {70, -170, -128, -85, -43, 0},
    {80, -198, -149, -99, -50, 0},
    {90, -227, -170, -113, -57, 0},
    {100, -255, -191, -128, -64, 0}, // R_Array[i][5] => 마지막 열 계산을 위해 넣은 데이터
};

// CAN BUS 초기화
void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;
    can.begin(can_tx, can_rx, 9600); // tx, rx

    if (can.canRate(CAN_RATE_250))
    {
        Serial.println("set can rate ok");
    }
    else
    {
        Serial.println("set can rate fail");
    }
}

void loop()
{
    // GEAR, RPM Pin 설정 (현재는 가변저항으로 테스트)
    gear_Pin = analogRead(A0);
    rpm_Pin = analogRead(A1);
    accel_Pin = analogRead(A2);

    SetRange();            // GEAR, RPM, APS 각각의 범위 설정
    gearCheck(gear_value); // gear_value를 체크하여 기어값이 P, D, R 중에 무엇인지 판단

    speed = Rpm_to_Speed(rpm); // 현재 RPM을 선형 속도(km/h)로 변환

    /* 
        추후에 주행중일때 기어를 변경 할 수 없는 로직 추가 필요
    */

    // GEAR 위치를 판단하여 알맞은 토크 값 로직 작동
    switch (gear)
    {
    case 'P':
        Parking_Torque();
        break;
    case 'D':
        Drive_Torque();
        break;
    case 'R':
        Reverse_Torque();
        break;
    default:
        break;
    }

    // 출력
    Serial.print("GEAR : ");
    Serial.print(gear);
    Serial.print("\t");
    Serial.print("RPM : ");
    Serial.print(rpm);
    Serial.print("\t");
    Serial.print("APS : ");
    Serial.print(APS);
    Serial.print("%");
    Serial.print("\t");
    Serial.print("Speed : ");
    Serial.print(speed);
    Serial.print("km/h");
    Serial.print("\t");
    Serial.print("Torque : ");
    Serial.println(torque);

    /*
        추후에 CAN 통신 규격을 정의한 후, 토크값을 CAN data로 변환하여 송신하는 기능 로직 구현 필요
    */

    delay(100);
}

// GEAR, RPM, APS 각각의 범위 설정
void SetRange()
{
    // GEAR 범위 설정
    map_gear = map(gear_Pin, 0, 1023, 0, 30);
    gear_value = constrain(map_gear, 0, 30);

    // RPM 범위 설정
    map_rpm = map(rpm_Pin, 0, 1023, 0, 7000);
    rpm = constrain(map_rpm, 0, 7000);

    // (임시방편 : RPM 가변저항 범위가 약간 튀어서 100 아래로는 0으로 설정)
    if (rpm < 100)
    {
        rpm = 0;
    }

    // APS 범위 설정 (임시방편 : 가변저항 범위가 약간 튀어서 105로 설정)
    map_accel = map(accel_Pin, 0, 1023, 0, 105);
    APS = constrain(map_accel, 0, 105);

    // (임시방편 : APS 가변저항 범위가 약간 튀어서 100 이상으로는 100으로 설정)
    if (APS >= 100)
    {
        APS = 100;
    }
}

// 기어 값 체크해서 P, D, R 확인
void gearCheck(int value)
{
    // Parking
    if (value <= 10)
    {
        gear = 'P';
    }
    // Drive
    else if (value <= 20)
    {
        gear = 'D';
    }
    // Reverse
    else if (value <= 30)
    {
        gear = 'R';
    }
}

// RPM 값 속도(km/h)로 변환 (정확한 변환 공식 필요)
int Rpm_to_Speed(int rpm)
{
    int spd = rpm / 35;
    return spd;
}

// GEAR가 P위치에 있을때 액셀 토크값을 0으로 설정
void Parking_Torque()
{
    torque = 0;
}

// GEAR가 D위치에 있을때 APS(%)와 속도(km/h)를 이용하여 액셀 토크값을 구하는 함수
void Drive_Torque()
{
    // D_Array 배열 표 참조
    for (int i = 1; i < 12; i++)
    {
        for (int j = 1; j < 10; j++)
        {
            // APS가 10보다 작을 경우 모두 0 처리
            if (APS < 10)
            {
                torque = 0;
            }
            // APS가 10 이상일 경우 (10~20, 20~30, ... 90~100 값을 비교)
            else if (APS >= D_Array[i][0] && APS < D_Array[i + 1][0])
            {
                // 속도가 100km/h 미만일 경우 APS에 따라 고정 값 출력
                if (speed < 100)
                {
                    torque = D_Array[i][1];
                }
                // 속도가 180km/h 이상일 경우 APS에 따라 고정 값 출력
                else if (speed >= 180)
                {
                    torque = D_Array[i][9];
                }
                // 속도가 100~180km/h 사이인 경우 (100~110, 110~120 ... 170~180 값을 비교)
                else if (speed >= D_Array[0][j] && speed < D_Array[0][j + 1])
                {
                    torque = D_Array[i][j];
                }
            }
        }
    }
}

// GEAR가 R위치에 있을때 APS(%)와 속도(km/h)를 이용하여 액셀 토크값을 구하는 함수
void Reverse_Torque()
{
    // R_Array 배열 표 참조
    for (int i = 1; i < 10; i++)
    {
        for (int j = 1; j < 6; j++)
        {
            // APS가 0일 경우
            if (APS == 0)
            {
                // 속도(km/h) 값에 따라 액셀 토크 설정 (배열 데이터 크기를 간략화 하기 위해 이 부분만 하드코딩)
                if (speed < 10)
                {
                    torque = 0;
                }
                else if (speed < 15)
                {
                    torque = 10;
                }
                else if (speed < 20)
                {
                    torque = 20;
                }
                else if (speed < 25)
                {
                    torque = 30;
                }
                else if (speed < 30)
                {
                    torque = 40;
                }
                else if (speed < 35)
                {
                    torque = 50;
                }
                else if (speed >= 35)
                {
                    torque = 60;
                }
            }
            // APS가 0보다 크고, 20보다 작을 경우 모두 0 처리
            else if (APS < 20)
            {
                torque = 0;
            }
            // APS가 20이상일 경우 (20~30, 30~40, ... 90~100 값을 비교)
            else if (APS >= R_Array[i][0] && APS < R_Array[i + 1][0])
            {
                // 속도가 25km/h 미만일 경우 APS에 따라 고정 값 출력
                if (speed < 25)
                {
                    torque = R_Array[i][1];
                }
                // 속도가 40km/h 이상일 경우 모두 0 처리
                else if (speed >= 40)
                {
                    torque = 0;
                }
                // 속도가 25~40km/h 사이인 경우 (25~30, 30~35, 35~40 값을 비교)
                else if (speed >= R_Array[0][j] && speed < R_Array[0][j + 1])
                {
                    torque = R_Array[i][j];
                }
            }
        }
    }
}