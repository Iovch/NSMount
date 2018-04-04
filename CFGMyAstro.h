/*
 NSMOUNTC.h File Written by Igor Ovchinnikov 20/01/2018
*/

#define MOUNT_TYPE_PIN A3 //Переключатель типа монтировки
#define ENABLE_XYZ_PIN 8  //Enable X,Y,Z pin
#define DX_STEP_PIN  5    //Контакт ардуино идущий на STEP драйвера X
#define DX_DIR_PIN   2    //Контакт ардуино идущий на DIR  драйвера X
#define DX_SW_PIN   A1    //Контакт переключателя направления вращения оси X
#define DX_FORCE_PIN 9    //Разгонный пин драйвера X
#define DY_STEP_PIN  6    //Контакт ардуино идущий на STEP драйвера Y
#define DY_DIR_PIN   3    //Контакт ардуино идущий на DIR  драйвера Y
#define DY_SW_PIN   A2    //Контакт переключателя направления вращения оси Y
#define DY_FORCE_PIN 10   //Разгонный пин драйвера Y
#define DZ_STEP_PIN  7    //Контакт ардуино идущий на STEP драйвера Z
#define DZ_DIR_PIN   4    //Контакт ардуино идущий на DIR  драйвера Z
#define DZ_FORCE_PIN 11   //Разгонный пин драйвера Z
#define SHUTTER_PIN  12   //Пин управления затвором фотоаппарата

#define CX_SENCE A6  //Сенсор оси X элемента управления или X+ для GUIDEPORT
#define CY_SENCE A7  //Сенсор оси Y элемента управления или Y+ для GUIDEPORT
#define CZ_SENCE A4  //Сенсор оси Z элемента управления (для джойстика кнопка) или X- для GUIDEPORT
#define CS_SENCE A5  //Сенсор чувствительности (скорости) элементов управления или Y- для GUIDEPORT

double Latitude = 56.7985; // Широта местности в градусах по умолчанию
double Longitude=-60.5923; // Долгота местности в градусах по умолчанию
int    iZH=5;              // Часовой пояс по умолчанию

int iStepsDX  =   48;    //Полных шагов на 1 оборот двигателя X
int iStepsXPS =  250;    //Полных шагов в секунду на двигателе X
int iXStepX   =   16;    //Кратность шага драйвера X
int iBLX      =    0;    //Люфт редуктора Х в микрошагах
double dRDX   = 1780.69; //Передаточное число редуктора X

int iStepsDY  =   96;    //Полных шагов на 1 оборот двигателя Y
int iStepsYPS =  350;    //Полных шагов в секунду на двигателе Y
int iYStepX   =    4;    //Кратность шага драйвера Y
int iBLY      =    0;    //Люфт редуктора Y в микрошагах
double dRDY   = 3168.00; //Передаточное число редуктора Y

int iStepsZPS = 300; //Полных шагов в секунду на двигателе Z
int iZStepX   =  32; //Кратность шага драйвера Z
int iStDZ     =  -1; //Исходное направление шага двигателя Z

long lShInt=1000; //Межкадровый интервал мс
long lMinSh=200;  //Минимальная выдержка мс

int iCtrlDelay=200; //Задержка обработки команд элементов управления
int iGuidDelay=0;   //Задержка выполнения команд гидирующего порта, мс

long lDMSS = -52000; //Поправка к Millis() за средние солнечные сутки (86400000ms)

