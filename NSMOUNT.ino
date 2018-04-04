/*
 NSMOUNT.ino File Written by Igor Ovchinnikov 25/02/2018
 скетч превращения CNC_v4_Shield в контроллер монтировки под NextStar+ Protocol
*/

boolean Debug = false;  //Режим отладки

#include "CFGMyAstro.h"

double drMaxValue=2.0*PI;            //Максимальное значение угла в радианах
unsigned long ulMaxValue=0xFFFFFFFF; //Максимальное значение unsigned long

unsigned long ulMSPS=0; // Милисекунд на полный оборот ведомого объекта, см. SetSMode()

int imStepsXPS = 0; //Микрошагов в секунду на двигателе X, определяет Force_X();
int imStepsYPS = 0; //Микрошагов в секунду на двигателе Y, определяет Force_Y();
int imStepsZPS = 0; //Микрошагов в секунду на двигателе Z, определяет Force_Z();

int iStDX=1; //Stepper_X_step(iStDX) увеличивает значение X, переопределяется в Setup();
int iStDY=1; //Stepper_Y_step(iStDY) увеличивает значение Y, переопределяется в Setup();

unsigned long ulSPX = 0; //Микрошагов двигателя на полный оборот оси X, определяет Force_X();
unsigned long ulSPY = 0; //Микрошагов двигателя на полный оборот оси Y, определяет Force_Y();
double drXperSTEP=0.0;   //Изменение X на 1 шаг двигателя, в радианах определяет Force_X();
double drYperSTEP=0.0;   //Изменение Y на 1 шаг двигателя, в радианах определяет Force_Y();

double dVRApsX=0.0; //Поворот Земли за время 1 шага Х, см. Force_X();
double dVRApsY=0.0; //Поворот Земли за время 1 шага У, см. Force_Y();

double dKMSPS=1.0+double(lDMSS)/86400000.0; //Поправка на точность Millis()

double drRaIncPS=drMaxValue/86164.091*dKMSPS; //Увеличение прямого восхождения на 1 секунду простоя монтировки
double drTperMS   = 0.0;                      //Изменение звездного времени за 1 милисекунду см. SetSMode()
double dXStepsPMS = 0.0;                      //Микрошагов двигателя X на 1 мс, см. Force_X();

int iLastDX = 0; //Направление последнего перемещения по Х
int iLastDY = 0; //Направление последнего перемещения по Y
int iLastDZ = 0; //Направление последнего перемещения по Z

int iXYRate = 0; //Скорость перемещения по осям X, Y
int iZRate  = 0; //Скорость перемещения по оси Z

int iCtrlEnable=0;  //Доступность элементов управления в системе

unsigned long ulMilisec =0;  // Виртуальное время монтировки
unsigned long ulLoopTimer=0; // Время входа в предыдущий цикл
unsigned long ulShutTimer=0; // Таймер затвора
unsigned long ulPortTimer=0; // Таймер СОМ порта
unsigned long ulCtrlTimer=0; // Таймер элементов управления
//unsigned long ulPXTimer=0;   // Таймер X P-команд
//unsigned long ulPYTimer=0;   // Таймер Y P-команд

int iPictures=0;            //Количество неотснятых кадров
int iExpoz=0;               //Выдержка сек, если 0 - то минимальная выдержка
boolean bShutting=false;    //Затвор не включен, фотографирование не запущено
boolean bStellarium=false;  //Используется прямое подключение к стеллариуму
boolean bPHD2=true;         //bPHD2 подключен 

struct RaRa {double AtX; double AtY; double ToX; double ToY; int FLXY;}; //Координаты наведения в радианах

RaRa AzAlt {0.0, 0.0, 0.0, 0.0, 0}; //Исходные и целевые AzAlt 
RaRa RaDe  {0.0, 0.0, 0.0, 0.0, 0}; //Исходные и целевые RaDe

double drTH   = PI;   //Текущее (исходное) значение часового угла 180 (Север)
double drLST  = PI;   //Местное звездное время 12h 
double drToTH = PI;   //Целевое значение часового угла

double drFi=Latitude/360.0*drMaxValue; //Широта в радианах

//long lControl=0; //Для AscControl()

// int iSteps = 0;           // Подсчет количества шагов

  int iTMode= 0; // Тип монтировки (задаем в сетапе)
//int iTMode= 1; // Альт-азимутальная монтировка
//int iTMode= 2; // Экваториальная монтировка Noth
//int iTMode= 3; // Экваториальная монтировка South
  int iSMode=-1; // Скорость ведения (задаем в сетапе)
//int iSMode= 0; // Остановлена
//int iSMode= 1; // Ведение со Звездной скоростью
//int iSMode= 2; // Ведение с Солнечной скоростью
//int iSMode= 3; // Ведение с Лунной скоростью

boolean bAlignment = false;  //Выравнивание не выполнено
boolean bForceX    = false;  //Ускоренный режим Х меняем в сетапе см. SetSMode()
boolean bForceY    = false;  //Ускоренный режим Y меняем в сетапе см. SetSMode()
boolean bForceZ    = false;  //Ускоренный режим Z

byte P[7], H[8], W[8];

#include "MTime.h"
#include "Astro.h"
#include "NSMOUNT.h"

void action(String STRA)
{
  int i;
  char cAction;
  cAction=STRA.charAt(0);
  switch (cAction)
  {
    case '?': {Serial.write(0); Serial.write(35); break;}//{Serial.print("1#"); break;}
    case 'p': {}
    case 'P': {Serial.write(35); break;}
    case 'a': {if(bAlignment) Serial.write(1); else Serial.write(0); Serial.write(35); break;}
    case 'A': {if(STRA.charAt(1)==0) bAlignment=false; if(STRA.charAt(1)==1) bAlignment=true; Serial.write(35); break;}
    case 'F': {GetSubStr(); iPictures=STR1.toInt(); iExpoz=STR2.toInt(); Serial.write(35); break;}
    case 'H': {SetTime();    Serial.write(35); break;} //Установка времени
    case 'h': {SendTime();   Serial.write(35); break;} //Запрос времени
    case 'W': {SetLatLon();  Serial.write(35); break;} //Установка координат
    case 'w': {SendLatLon(); Serial.write(35); break;} //Запрос координат
    case 'V': {Serial.write(1); Serial.write(0); Serial.write(35); break;} //Версия протокола
    case 'v': {Serial.write(4); Serial.write(4); Serial.write(35); break;} //Версия программы (?)
    case 'K': {Serial.print(STRA.charAt(1)); Serial.write(35); break;}
    case 'J': {if(bAlignment) Serial.write(1); else Serial.write(0); Serial.write(35); break;} //только для аскома, if(bAlignment) должно быть
    case 'L': {if (((iTMode==0||iTMode==1)&&AzAlt.FLXY==0)||((iTMode==2||iTMode==3)&&(RaDe.FLXY==0))) Serial.print("0#");
               else Serial.print("1#");  break;}
    case 'm': {Serial.write(7); Serial.write(35); break;} // 6 - Edvanced GT, 7 - SLT
    case 'M': {if(iTMode==0||iTMode==1) {AzAlt.ToX=AzAlt.AtX; AzAlt.ToY=AzAlt.AtY; AzAlt.FLXY=0;}
               if(iTMode==2||iTMode==3) {RaDe.ToX=RaDe.AtX; RaDe.ToY=RaDe.AtY; RaDe.FLXY=0;}
               Serial.write(35); break;}           
    case 'd': {Serial.write(iSMode); Serial.write(35); break;} //Незадекларированная в протоколе команда запроса скорости ведения
    case 'D': {if(STRA.charAt(1)>=0&&STRA.charAt(1)<=3)        //Незадекларированная в протоколе команда установки скорости ведения
               {iSMode=SetSMode(STRA.charAt(1)); if(iSMode>0) {ulMilisec=millis(); ulLoopTimer= millis();}}
               Serial.write(35); break;}                 
    case 'n': {if(digitalRead(ENABLE_XYZ_PIN)==HIGH) Serial.write(0); if(digitalRead(ENABLE_XYZ_PIN)==LOW) Serial.write(1); Serial.write(35); break;} //Motors disabled/enabled  
    case 'N': {if(STRA.charAt(1)==0) digitalWrite(ENABLE_XYZ_PIN, HIGH); if(STRA.charAt(1)==1) digitalWrite(ENABLE_XYZ_PIN, LOW); Serial.write(35); break;} //Motors desable/enable
    case 'G': {if(Debug) {Debug=false; Serial.print("UnDebug#");} else {Debug=true; Serial.print("Debug#");} break;}
    case 't': {Serial.write(iTMode); Serial.write(35); break;}
    case 'T': {if(STRA.charAt(1)==0) if((iTMode==2)||(iTMode==3)) iSMode=SetSMode(0);
               if(STRA.charAt(1)>=1&&STRA.charAt(1)<=3) {iTMode=STRA.charAt(1); iSMode=SetSMode(1);}
               if(iSMode>0) {ulMilisec=millis(); ulLoopTimer= millis();}
               Serial.write(35); break;}
    case 'E': {Serial.print(HexToStr(RaToUL(RaDe.AtX,ulMaxValue)>>16,4)+','+HexToStr(RaToUL(RaDe.AtY,ulMaxValue)>>16,4)+'#');
                break;}           
    case 'e': {Serial.print(HexToStr(RaToUL(RaDe.AtX,ulMaxValue),8)); Serial.print(","); // 
               Serial.print(HexToStr(RaToUL(RaDe.AtY,ulMaxValue),8)); Serial.write(35); // 
               break;}
    case 'x': {Serial.print(HexToStr(RaToUL(RaDe.AtX,ulMaxValue),8)); Serial.write(35); break;} // Отдельно координату X передаем
               
    case 'y': {Serial.print(HexToStr(RaToUL(RaDe.AtY,ulMaxValue),8)); Serial.write(35); break;} // Отдельно координату Y передаем
                             
    case 'Z': {Serial.print(HexToStr(RaToUL(AzAlt.AtX,ulMaxValue)>>16,4)); Serial.print(","); // 
               Serial.print(HexToStr(RaToUL(AzAlt.AtY,ulMaxValue)>>16,4)); Serial.write(35); // 
               break;}     
    case 'z': {Serial.print(HexToStr(RaToUL(AzAlt.AtX,ulMaxValue),8)); Serial.print(","); // 
               Serial.print(HexToStr(RaToUL(AzAlt.AtY,ulMaxValue),8)); Serial.write(35); //   
               break;}
    case 'B': {GetSubStr (); if((STR1.length()==4)&&(STR2.length()==4)) STR=STR1+"0000,"+STR2+"0000";}         
    case 'b': {GetSubStr ();
               if(STR1.length()==8&&STR2.length()==8)
               {
                if (iTMode==0||iTMode==1) //Азимутальный режим
                 {
                  AzAlt.ToX=ULToRa(StrToHEX(STR1),ulMaxValue);
                  AzAlt.ToY=ULToRa(StrToHEX(STR2),ulMaxValue);
                  if (bAlignment) {ulLoopTimer= millis(); ToAZaH(true);}
                  else {AzAlt.AtX=AzAlt.ToX; AzAlt.AtY=AzAlt.ToY; bAlignment=true;}
                 }
                Serial.write(35);}
                break;}
    case 'R': {GetSubStr (); if((STR1.length()==4)&&(STR2.length()==4)) STR=STR1+"0000,"+STR2+"0000";}
    case 'r': {GetSubStr ();
               if(STR1.length()==8&&STR2.length()==8)
               {
                if ((iTMode==2)||(iTMode==3)) //Экваториальный режим
                 {
                   RaDe.ToX=ULToRa(StrToHEX(STR1),ulMaxValue);
                   RaDe.ToY=ULToRa(StrToHEX(STR2),ulMaxValue);
                   ulMilisec =millis();
                   if (bAlignment) ToRaDe(true); else {RaDe.AtX=RaDe.ToX; RaDe.AtY=RaDe.ToY; bAlignment=true; iSMode=SetSMode(1);}
                 }
                if ((iTMode==0)||(iTMode==1)) //Азимутальный режим
                 {
                   RaDe.AtX=ULToRa(StrToHEX(STR1),ulMaxValue); RaDe.ToX=RaDe.AtX;
                   RaDe.AtY=ULToRa(StrToHEX(STR2),ulMaxValue); RaDe.ToY=RaDe.AtY;
                   ulLoopTimer= millis();
                   AzAltFromRaDe(2);
                   if (bAlignment) ToAZaH (true); else {AzAlt.AtX=AzAlt.ToX; AzAlt.AtY=AzAlt.ToY; bAlignment=true; iSMode=SetSMode(1);}
                 }
                }
               bStellarium=true; //Если поступила команда 'r', считаем, что подключен стеллариум
               bPHD2=false;
               Serial.write(35); break;}
    case 'S': {GetSubStr (); if((STR1.length()==4)&&(STR2.length()==4)) STR=STR1+"0000,"+STR2+"0000";}
    case 's': {GetSubStr ();
               if((STR1.length()==8)&&(STR2.length()==8))
               {
                if (iTMode==0||iTMode==1) //Азимутальный режим
                {
                 AzAlt.AtX=ULToRa(StrToHEX(STR1),ulMaxValue); AzAlt.ToX=AzAlt.AtX;
                 AzAlt.AtY=ULToRa(StrToHEX(STR2),ulMaxValue); AzAlt.ToY=AzAlt.AtY;
                 ulLoopTimer=millis();
                 RaDeFromAzAlt();
                 ulMilisec=millis();
                }
                if (iTMode==2||iTMode==3) //Экваториальный режим
                {
                 RaDe.AtX=ULToRa(StrToHEX(STR1),ulMaxValue); RaDe.ToX=RaDe.AtX;
                 RaDe.AtY=ULToRa(StrToHEX(STR2),ulMaxValue); RaDe.ToY=RaDe.AtY;
                 ulMilisec=millis();
                 AzAltFromRaDe(3);
//                 ulLoopTimer=millis();
                }
                bAlignment=true;
                Serial.write(35);
               }
               break;}               
  };
}

void setup()
{
  pinMode(MOUNT_TYPE_PIN, INPUT_PULLUP); // Сенсор типа монтировки
  pinMode(ENABLE_XYZ_PIN,  OUTPUT);      // ENABLE XYZ PIN
  digitalWrite(ENABLE_XYZ_PIN, LOW);     // LOW
  pinMode(DX_STEP_PIN,  OUTPUT);         // DX STEP PIN
  digitalWrite(DX_STEP_PIN, LOW);        // LOW
  pinMode(DX_DIR_PIN,  OUTPUT);          // DX DIR PIN
  digitalWrite(DX_DIR_PIN, LOW);         // LOW
  pinMode(DX_FORCE_PIN,  OUTPUT);        // DX FORCE PIN
  digitalWrite(DX_FORCE_PIN, HIGH);      // HIGH
  pinMode(DX_SW_PIN, INPUT_PULLUP);      // HIGH
  pinMode(DY_STEP_PIN,  OUTPUT);         // DY STEP PIN
  digitalWrite(DY_STEP_PIN, LOW);        // LOW
  pinMode(DY_DIR_PIN,  OUTPUT);          // DY DIR PIN
  digitalWrite(DY_DIR_PIN, LOW);         // LOW
  pinMode(DY_FORCE_PIN,  OUTPUT);        // DY FORCE PIN
  digitalWrite(DY_FORCE_PIN, HIGH);      // HIGH
  pinMode(DY_SW_PIN, INPUT_PULLUP);      // HIGH
  pinMode(DZ_STEP_PIN,  OUTPUT);         // DZ STEP PIN
  digitalWrite(DZ_STEP_PIN, LOW);        // LOW
  pinMode(DZ_DIR_PIN,  OUTPUT);          // DZ DIR PIN
  digitalWrite(DZ_DIR_PIN, LOW);         // LOW
  pinMode(DZ_FORCE_PIN,  OUTPUT);        // DZ FORCE PIN
  digitalWrite(DZ_FORCE_PIN, HIGH);      // HIGH
  pinMode(SHUTTER_PIN,  OUTPUT);         // SHUTTER PIN
  digitalWrite(SHUTTER_PIN, LOW);        // LOW
  pinMode(13,  OUTPUT);                  // Индикатор работы контроллера
  digitalWrite(13, LOW);                 // Выключен

  pinMode(CS_SENCE, INPUT_PULLUP);
  pinMode(CX_SENCE, INPUT_PULLUP);
  pinMode(CY_SENCE, INPUT_PULLUP);
  pinMode(CZ_SENCE, INPUT_PULLUP);
  
  Serial.begin(9600);
  Serial.setTimeout(2); //Максимальная задержка при чтении байтов из порта до 31/10/17 было 10 мс
  Serial.flush();
  
  iCtrlEnable=InitControl();
  ulCtrlStat=AskControl(); 
     
  if(analogRead(MOUNT_TYPE_PIN) <250) {iTMode=2; iSMode=SetSMode(0);} // Экваториальная монтировка (Noth) остановлена
  if(analogRead(MOUNT_TYPE_PIN)>=250) {iTMode=1; iSMode=SetSMode(0);} // Альт-азимутальная монтировка остановлена
  if(analogRead(MOUNT_TYPE_PIN)>=750) {iTMode=2; iSMode=SetSMode(1);} // Экваториальная монтировка (Noth), cкорость ведения (Sideral(1))
    
  Force_Z(!bForceZ);    // Пересчет параметров шагов двигателя Z; X,Y - задаются в SetSMode();
  ulMilisec = millis(); // Виртуальное время монтировки приравниваем к фактическому
  ulLoopTimer=millis(); // Время выполнения цикла
  ulPortTimer=millis(); // Таймер СОМ порта
  ulCtrlTimer=millis(); // Таймер элементов управления
 }

void loop()
{
 long   lRLDTime=0;     //Реальное время исполнения цикла
 long   lVDMTime=0;     //Виртуальное время исполнения цикла
 long   lStepsNeed=0;   //Необходимое количество шагов 
 double dRaIncNeed=0;   //Необходимое увеличение ПВ
 int Bytes=0, iDCTRL=0;

 SetStDX(); //Установка направления вращения оси Х (задержка 0,1 мс)
 SetStDY(); //Установка направления вращения оси У (задержка 0,1 мс)

 Bytes=GetString();        // Чтение порта
 if(Bytes>0) action(STR);  // Обработка команд порта
 p();                      // Обработка команд Pass-through порта
 ulCtrlStat=AskControl();  // Запрос и обработка состояния элементов управления
    
 lVDMTime  = millis()-ulMilisec;  //Виртуальное время исполнения предыдущего цикла
 lRLDTime  = millis()-ulLoopTimer; //Реальное время исполнения предыдущего цикла
 ulLoopTimer= millis();
 
 Force_X(false); Force_Y(false);

 drLST=RLST(); //AskClock(); Корректировка часов реального времени внутри LST()

 if(Debug)
  {
//    Serial.print(" drTH=");   Serial.print (drTH/drMaxValue*360.0,   4);    Serial.print (" ");
//    Serial.print(" drToTH="); Serial.print (drToTH/drMaxValue*360.0, 4);    Serial.print (" ");
//    Serial.print(" drLST=");  Serial.print (drLST/drMaxValue*360.0,  4);    Serial.print (" "); 
//    Serial.print(" Az=");     Serial.print (AzAlt.AtX/drMaxValue*360.0, 4); Serial.print (" ");
//    Serial.print(" ToAz=");   Serial.print (AzAlt.ToX/drMaxValue*360.0, 4); Serial.print (" ");
//    Serial.print(" Alt=");    Serial.print (AzAlt.AtY/drMaxValue*360.0, 4); Serial.print (" ");
//    Serial.print(" ToAlt=");  Serial.print (AzAlt.ToY/drMaxValue*360.0, 4); Serial.println("");
      Serial.print(" iCtrlEnable "); Serial.print(iCtrlEnable); //Serial.println("");
      Serial.print(" CX_SENCE ");  Serial.print(analogRead(CX_SENCE));  //Serial.println("");
      Serial.print(" CY_SENCE ");  Serial.print(analogRead(CY_SENCE));  //Serial.println(""); 
      Serial.print(" CZ_SENCE ");  Serial.print(analogRead(CZ_SENCE));  //Serial.println(""); 
      Serial.print(" CS_SENCE ");  Serial.print(analogRead(CS_SENCE));  Serial.println(""); 
    delay(200);
  }

 if((iTMode==2||iTMode==3)&&iSMode!=0) //Экваториальный режим и ведение включено
  {
   if(abs(RaDe.AtX-RaDe.ToX)>=drXperSTEP||abs(RaDe.AtY-RaDe.ToY)>=drYperSTEP) //Обработка команд GOTO
     {
      ToRaDe(true);
      ulMilisec=millis();
     }
   else //Экваториальный режим и ведение выключено
    {
     Force_X(false); //Микрошаговый режим для пересчета dXStepsPMS
     lStepsNeed=lVDMTime*dXStepsPMS;
     if(lStepsNeed>=1)
     {
      Stepper_X_step(lStepsNeed*(-iStDX));             // Шагаем в сторону уменьшения ПВ!
      ulMilisec+=round(double(lStepsNeed)/dXStepsPMS); // Изменение виртуального времени монтировки за выполненные шаги
     }
//    if(analogRead(MOUNT_TYPE_PIN)>=750) iSMode=SetSMode(1); // Принудительное включение трекинга (Sideral(1))
    }
  }

 if (iTMode==1&&iSMode==0) //Азимутальная монтировка остановлена
   {
    if(abs(AzAlt.AtX-AzAlt.ToX)>=drXperSTEP||abs(AzAlt.AtY-AzAlt.ToY)>=drYperSTEP) //Обработка команд GOTO
     {
      ulLoopTimer= millis();
      ToAZaH(true);
     }
     else AzAlt.FLXY=0;
     RaDeFromAzAlt();
   }

 if (bAlignment) //Работа нижеприведенных вещей не имеет смысла без предварительной привязки к координатам:
  {
    
   if (iTMode==1&&iSMode!=0) //Азимутальная монтировка ведение включено:
    {
     drTH += (drTperMS*lRLDTime);   //Изменение часового угла за время предыдущего цикла
     AzAltFromRaDe(2);
     if((abs(AzAlt.ToX-AzAlt.AtX)>=drXperSTEP)||(abs(AzAlt.ToY-AzAlt.AtY)>=drYperSTEP)) {ToAZaH(false);}
    }
 
  if((iTMode==2||iTMode==3)&&iSMode==0) //Экваториальный режим ведение выключено
   {
    if(abs(RaDe.AtX-RaDe.ToX)>=drXperSTEP||abs(RaDe.AtY-RaDe.ToY)>=drYperSTEP) //Обработка команд GOTO
     {
      ToRaDe(true);
      ulMilisec=millis();
     }
    else
     {  
      dRaIncNeed=lVDMTime*drRaIncPS/1000.0;
      if(dRaIncNeed>=(drMaxValue/360/60/60))
      {
       RaDe.AtX+=dRaIncNeed;
       RaDe.ToX+=dRaIncNeed;
       ulMilisec+=round(dRaIncNeed/drRaIncPS/1000.0);   //millis();
      }
     }
   }
  }
 Shutting();  // Фотографирование
 if(bStellarium) if((millis()-ulPortTimer)>=1000) {action("e"); ulPortTimer=millis();} // Прямое подключение к стеллариуму
 if(bPHD2) if((millis()-ulPortTimer)>=1000) {Serial.write(35); ulPortTimer=millis();} // Прямое подключение к PHD2
 }
