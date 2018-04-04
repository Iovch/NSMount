/*
 NSMOUNT.h Written by Igor Ovchinnikov 25/02/2018
*/

String STR= "", STR1="", STR2="";

int  iMJX, iMJY, iMJZ;      //Значения сенсоров Х,У,Z в исходном состоянии
unsigned long ulCtrlStat=0; //Текущее и предыдущее положение элементов управления
int iLastCtrlKey=0;         //Последняя использованная команда элемента управления

byte PJ[4][8]; //Массив состояний элементов управления


unsigned long RaToUL (double Ra, unsigned long MaxValue) //Перевод величин в радианах в unsigned long [0,MaxValue]
 {
  double   D0=Ra, D2=MaxValue;
  unsigned long RaToUL;
  while(D0<0.0) D0+=drMaxValue;
  RaToUL=D0/drMaxValue*D2;
  return RaToUL;
 }

double ULToRa (unsigned long Value, unsigned long MaxValue) //Перевод unsigned long [0,MaxValue] в радианы 
 {
   long double D1=Value>>8;
   long double D2=MaxValue>>8;
   return D1/D2*drMaxValue;
 }

void SetLatLon(void)
{
  Latitude =double(W[0])+double(W[1])/60.0+double(W[2])/3600.0;
  if(W[3]==1) Latitude=-Latitude;
  Longitude=double(W[4])+double(W[5])/60.0+double(W[6])/3600.0;
  if(W[7]==0) Longitude=-Longitude;
}

void SendLatLon(void)
{
  W[0]=int(abs(Latitude));
  W[1]=int((abs(Latitude)-W[0])*60);
  W[2]=int((abs(Latitude)-W[0]-W[1]*60)*3600);
  if(Latitude<0) W[3]=1; else W[3]=0;
  W[4]=int(abs(Longitude));
  W[5]=int((abs(Longitude)-W[4])*60);
  W[6]=int((abs(Longitude)-W[4]-W[5]*60)*3600);
  if(Longitude<0) W[7]=0; else W[7]=1;
  Serial.write(W,8);
}

boolean Force_X(boolean bForce)
{
 int iXSX=0; 
 if(!bForceX && bForce) //Включаем полношаговый режим
  {
   digitalWrite(DX_FORCE_PIN, LOW);
   iXSX = 1;                       //Кратность шага драйвера X
   imStepsXPS = iStepsXPS*iXSX;    //Микрошагов в секунду на двигателе X
   bForceX=true;
  }
 if(bForceX && !bForce) //Включаем микрошаговый режим
  {
   digitalWrite(DX_FORCE_PIN, HIGH);
   iXSX = iXStepX;                  //Кратность шага драйвера X
   imStepsXPS = 500;                //Микрошагов в секунду на двигателе X
   bForceX=false;
  }
 if(iXSX!=0) //Если что-либо изменилось
  {  
   ulSPX = dRDX*iStepsDX*iXSX;               //Шагов двигателя X на полный оборот оси прямого восхождения
   drXperSTEP=drMaxValue/iXSX/iStepsDX/dRDX; //Изменение X за 1 шаг двигателя
   dXStepsPMS=double(ulSPX)/double(ulMSPS);  //Микрошагов двигателя X на 1 мс
   if(iTMode==2||iTMode==3) dVRApsX=drMaxValue/86164091.0*dKMSPS*double(imStepsXPS)/1000.0; //Изменение Ra за время шага
   else dVRApsX=0.0;
  }
 return bForceX;
}

boolean Force_Y(boolean bForce)
{
  int iYSX=0;
  if(!bForceY && bForce) //Включаем полношаговый режим
   {
    digitalWrite(DY_FORCE_PIN, LOW);
    iYSX = 1;                       //Кратность шага драйвера Y
    imStepsYPS = iStepsYPS*iYSX;    //Шагов в секунду на двигателе Y
    bForceY=true;
   }
  if(bForceY && !bForce) //Включаем микрошаговый режим
   {
    digitalWrite(DY_FORCE_PIN, HIGH);
    iYSX = iYStepX;                  //Кратность шага драйвера Y
    imStepsYPS = 500;                //Микрошагов в секунду на двигателе Y
    bForceY=false;
   }
  if(iYSX!=0)
   { 
    ulSPY = iStepsDY*dRDY*iYSX;               //Микрошагов двигателя Y на полный оборот оси склонений
    drYperSTEP=drMaxValue/iYSX/iStepsDY/dRDY; //Изменение Y за 1 шаг двигателя
    if(iTMode==2||iTMode==3) dVRApsY=drMaxValue/86164091.0*dKMSPS*double(imStepsYPS)/1000.0; //Изменение Ra за время шага
    else dVRApsY=0.0;
   }
 return bForceY;
}

boolean Force_Z(boolean bForce)
{
  if(!bForceZ && bForce) //Включаем полношаговый режим
   {
    digitalWrite(DZ_FORCE_PIN, LOW);
    imStepsZPS = iStepsZPS; //Шагов в секунду на двигателе Z
    bForceZ=true;
   }
  if(bForceZ && !bForce) //Включаем микрошаговый режим
   {
    digitalWrite(DZ_FORCE_PIN, HIGH);
    imStepsZPS = 500; //Микрошагов в секунду на двигателе Z
    bForceZ=false;
   }
  return bForceZ; 
 }

long Stepper_step(long ipSteps, unsigned uStepPin, unsigned uDirPin, unsigned uStepsPS)
{
 long iSteps=ipSteps, lRetVal=0;
 if((uStepPin>53)||(uDirPin>53)) return lRetVal;
 if(iSteps > 0) digitalWrite(uDirPin,  LOW);
 if(iSteps < 0) digitalWrite(uDirPin,  HIGH);
 iSteps=abs(iSteps);
 while (iSteps>0)
 {
  digitalWrite(uStepPin,  HIGH);
  digitalWrite(13, HIGH);
  delay(1000/uStepsPS);
  //delayMicroseconds(1000*(1000%uStepsPS));
  digitalWrite(uStepPin,  LOW);
  digitalWrite(13, LOW);
  iSteps--;
  if (ipSteps>0) lRetVal++; else lRetVal--;
 }
 return lRetVal;
}

void Stepper_X_step(int ipSteps)
{
  Stepper_step(ipSteps, DX_STEP_PIN, DX_DIR_PIN, imStepsXPS);
}

void Stepper_Y_step(int ipSteps)
{
  Stepper_step(ipSteps, DY_STEP_PIN, DY_DIR_PIN, imStepsYPS);
}

void Stepper_Z_step(int ipSteps)
{
  Stepper_step(ipSteps, DZ_STEP_PIN, DZ_DIR_PIN, imStepsZPS);
}

int GetString (void)
{
  int GetString=0;
  STR="";
  char c;
  while (Serial.available()>0) //если есть что читать;
  {
   c = Serial.read(); //читаем символ
   STR += c;
   GetString+=1;
   if(STR.length()==1)
   {
    if(c=='P') {GetString+=Serial.readBytes(P,7); break;}
    if(c=='H') {GetString+=Serial.readBytes(H,8); break;}
    if(c=='W') {GetString+=Serial.readBytes(W,8); break;}
   }
   delay(1);
  }
  return GetString;
}

int GetSubStr ()
{
  int i,j;
  STR1="", STR2="";
  i=STR.indexOf(',');
  j=STR.length();
  if (i>0) {STR1=STR.substring(1,i); STR2=STR.substring(i+1,j);}
  if (i<0 && STR.length()>1) STR1=STR.substring(1);
  return i;
}

unsigned long StrToHEX (String STR)
{
  int  i;
  char c;
  unsigned long ulVal=0;
  for (i=0; i<STR.length(); i++)
  {
   ulVal=ulVal*16;
   c=STR.charAt(i);
   switch (c) 
    {
      case 'f': ;
      case 'F': ulVal++;
      case 'e': ;
      case 'E': ulVal++;
      case 'd': ;
      case 'D': ulVal++;
      case 'c': ;
      case 'C': ulVal++;
      case 'b': ;
      case 'B': ulVal++;
      case 'a': ;
      case 'A': ulVal++;
      case '9': ulVal++;
      case '8': ulVal++;
      case '7': ulVal++;
      case '6': ulVal++;
      case '5': ulVal++;
      case '4': ulVal++;
      case '3': ulVal++;
      case '2': ulVal++;
      case '1': ulVal++;
    };
  };
 return ulVal;
}

String HexToStr (unsigned long ulpHex, int iDigits)
{
 String HexStr="";
 unsigned long ulHex=ulpHex;
 int iDigit=iDigits;
 do {
 if((ulHex&0xF)==0xF) HexStr="F"+HexStr; else 
 if((ulHex&0xE)==0xE) HexStr="E"+HexStr; else
 if((ulHex&0xD)==0xD) HexStr="D"+HexStr; else
 if((ulHex&0xC)==0xC) HexStr="C"+HexStr; else
 if((ulHex&0xB)==0xB) HexStr="B"+HexStr; else
 if((ulHex&0xA)==0xA) HexStr="A"+HexStr; else
 if((ulHex&0x9)==0x9) HexStr="9"+HexStr; else
 if((ulHex&0x8)==0x8) HexStr="8"+HexStr; else
 if((ulHex&0x7)==0x7) HexStr="7"+HexStr; else
 if((ulHex&0x6)==0x6) HexStr="6"+HexStr; else
 if((ulHex&0x5)==0x5) HexStr="5"+HexStr; else
 if((ulHex&0x4)==0x4) HexStr="4"+HexStr; else
 if((ulHex&0x3)==0x3) HexStr="3"+HexStr; else
 if((ulHex&0x2)==0x2) HexStr="2"+HexStr; else
 if((ulHex&0x1)==0x1) HexStr="1"+HexStr; else
 HexStr="0"+HexStr;
 ulHex=(ulHex>>4);
 iDigit--;
 } while (iDigit>0);
 return HexStr;
}

int p(void)
{
 int p=0;
 unsigned uXTimer,uYTimer;
 unsigned long ulStepsNeed;
 double dN=0.0, drDSideral1000;  
 if (P[0]==1) //Проверка наличия моторов
   {
    if((P[1]==16)||(P[2]==17)) Serial.print("#"); //Типа версии моторов
   }
 if (P[0]==2) //Монтировка, фиксированные скорости
   {
    switch (P[3]) 
     {
      case 1: {dN=1.0;  break;} case 2: {dN=2.0;   break;} case 3: {dN=4.0;   break;}
      case 4: {dN=8.0;  break;} case 5: {dN=16.0;  break;} case 6: {dN=32.0;  break;}
      case 7: {dN=64.0; break;} case 8: {dN=100.0; break;} case 9: {dN=200.0; break;}
      default: dN=0.0;
     };
//    if(dN>0) iXYRate=P[3]; //Синхронизация скорости с контроллером управления 
    if(P[1]==16) //Ось Х
    {
     if(((P[2]==36)||(P[2]==37))&&(dN>0))
     {
      if(dN>=iXStepX) {if(!bForceX) bForceX=Force_X(true);  ulStepsNeed=dN/iXStepX;} //Переход в полный шаг
      if(dN <iXStepX) {if( bForceX) bForceX=Force_X(false); ulStepsNeed=dN;}         //Переход в микрошаг
      if(P[2]==36) {Stepper_X_step( iStDX*ulStepsNeed); p= 1; ulLoopTimer=millis(); ulMilisec=millis();} //По Х вправо
      if(P[2]==37) {Stepper_X_step(-iStDX*ulStepsNeed); p=-1; ulLoopTimer=millis(); ulMilisec=millis();} //По Х влево
     }
     if(P[2]==39) ;                                                                          //Стоп Х
     if(P[2]==254&&P[6]==2) Serial.print("#"); //Ось Х управляется
    }
    if(P[1]==17) //Ось Y
    {
     if(((P[2]==36)||(P[2]==37))&&(dN>0))
     {
      if(dN>=iYStepX) {if(!bForceY) bForceY=Force_Y(true);  ulStepsNeed=dN/iYStepX;} //Переход в полный шаг
      if(dN< iYStepX) {if( bForceY) bForceY=Force_Y(false); ulStepsNeed=dN;}         //Переход в микрошаг
      if(P[2]==36) {Stepper_Y_step( iStDY*ulStepsNeed); p= 2;}   //По Y вверх
      if(P[2]==37) {Stepper_Y_step(-iStDY*ulStepsNeed); p=-2;}   //По Y вниз
     } 
     if(P[2]==39) ; //Стоп У
     if(P[2]==254&&P[6]==2) Serial.print("#"); //Ось У управляется
    }
    if(P[1]==10||P[1]==11) {Serial.print("#"); bPHD2=true; bStellarium=false;} //считаем, что bPHD2 подключен
   }
  if (P[0]==3) //Монтировка, Push (толчки)
   {
    if(P[1]==16) //Ось Х
    {
     if(P[2]==38&&P[3]==13)  {Force_X(false); Stepper_X_step( iStDX*P[4]); p= 1;} //По Х вправо P[4]*10 ms
     if(P[2]==38&&P[3]==243) {Force_X(false); Stepper_X_step(-iStDX*P[4]); p=-1;} //По Х влево  P[4]*10 ms
    }
    if(P[1]==17) //Ось Y
    {
     if(P[2]==38&&P[3]==13)  {Force_Y(false); Stepper_Y_step( iStDY*P[4]); p= 2;} //По Y вверх P[4]*10 ms
     if(P[2]==38&&P[3]==243) {Force_Y(false); Stepper_Y_step(-iStDY*P[4]); p=-2;} //По Y вниз  P[4]*10 ms
    }
   }
  if (P[0]==4) {P[2]=39; P[3]=0;} //Остановка монтировки
  if (P[0]==5) //Фокусер
  {
    switch (P[3]) 
     {
      case 1: {dN=1.0;   break;} case 2: {dN=2.0;   break;} case 3: {dN=4.0;   break;}
      case 4: {dN=8.0;   break;} case 5: {dN=16.0;  break;} case 6: {dN=32.0;  break;}
      case 7: {dN=64.0; break;}  case 8: {dN=100.0; break;} case 9: {dN=200.0; break;}
      default: dN=0.0;
     };
    if((dN <iZStepX)&&( bForceZ))  Force_Z(false);
    if((dN>=iZStepX)&&(!bForceZ)) {Force_Z(true);  dN/=iZStepX;} 
    if(P[2]==36) {Stepper_Z_step( iStDZ*dN); p= 3;} //Фокусер+
    if(P[2]==37) {Stepper_Z_step(-iStDZ*dN); p=-3;} //Фокусер- 
    if(P[2]==39) ; //Стоп фокусер
  }
 return p; 
}

RaRa AtXYtoXY (RaRa XY, boolean Force, unsigned long ulMS)
{
  int iDIRX=0, iDIRY=0; //Знак изменения значения координат X,Y
  double drDX, drDY;    //Разница координат Х,У
  boolean ForceX=Force, ForceY=Force; //Запоминаем рекомендуемую скорость
  boolean bBLX=false, bBLY=false;
  unsigned long ulStartMS=millis();

  XY.FLXY=0;
  
  Force_X(Force); Force_Y(Force);
   
  if (XY.ToX > XY.AtX) {drDX = (XY.ToX-XY.AtX); iDIRX=  1;}
  if (XY.ToX < XY.AtX) {drDX = (XY.AtX-XY.ToX); iDIRX= -1;}
  if (drDX > PI)       {drDX = 2*PI-drDX;  iDIRX = -(iDIRX);}

  if (XY.ToY > XY.AtY) {drDY = (XY.ToY-XY.AtY); iDIRY=  1;}
  if (XY.ToY < XY.AtY) {drDY = (XY.AtY-XY.ToY); iDIRY= -1;}
  if (drDY > PI)       {drDY = 2*PI-drDY;  iDIRY = -(iDIRY);}

  if (drDX > PI) {XY.FLXY=-1; return XY;} //Ошибка в расчете шагов по первой координате
  if (drDY > PI) {XY.FLXY=-2; return XY;} //Ошибка в расчете шагов по второй координате

  if (drDX < drXperSTEP) ForceX=Force_X(false);
  if (drDY < drYperSTEP) ForceY=Force_Y(false);

  while ((((drDX >= drXperSTEP) && iStDX!=0)||((drDY >= drYperSTEP) && iStDY!= 0))&&((millis()-ulStartMS)<ulMS))
   {
    if (drDX >= drXperSTEP)
     {
      XY.FLXY=1; //Далее компенсация люфта Х:
      if ((!bBLX) && (iDIRX >0) && (iLastDX==-1)) {Force_X(false); Stepper_X_step( iStDX*iBLX); Force_X(ForceX); iLastDX= 1; bBLX=true;}
      if ((!bBLX) && (iDIRX <0) && (iLastDX== 1)) {Force_X(false); Stepper_X_step(-iStDX*iBLX); Force_X(ForceX); iLastDX=-1; bBLX=true;}
      if (iDIRX >0)  {Stepper_X_step( iStDX); drDX-=(drXperSTEP+dVRApsX); XY.AtX+=(drXperSTEP+dVRApsX);} //Основное движение в сторону увеличения Х
      if (iDIRX <0)  {Stepper_X_step(-iStDX); drDX-=(drXperSTEP-dVRApsX); XY.AtX-=(drXperSTEP-dVRApsX);} //Основное движение в сторону уменьшения Х
      XY.AtX=NorRad(XY.AtX);
      if ((drDX < drXperSTEP) && bForceX) Force_X(false); //Доводка микрошагами
      if (drDX > PI) drDX = 0; //Перестраховка
     }
    if (drDY >= drYperSTEP)
     {
      XY.FLXY=2; //Далее компенсация люфта У:
      if ((!bBLY) && (iDIRY >0) && (iLastDY==-1)) {Force_Y(false); Stepper_Y_step( iStDY*iBLY); Force_Y(ForceY); iLastDY= 1; bBLY=true;}
      if ((!bBLY) && (iDIRY <0) && (iLastDY== 1)) {Force_Y(false); Stepper_Y_step(-iStDY*iBLY); Force_Y(ForceY); iLastDY=-1; bBLY=true;}
      if (iDIRY > 0) {Stepper_Y_step( iStDY); drDY-=drYperSTEP; XY.AtY+=drYperSTEP; XY.AtX+=dVRApsY;} //Основное движение в сторону увеличения Y
      if (iDIRY < 0) {Stepper_Y_step(-iStDY); drDY-=drYperSTEP; XY.AtY-=drYperSTEP; XY.AtX+=dVRApsY;} //Основное движение в сторону уменьшения Y
      XY.AtY=NorRad(XY.AtY);
      if ((drDY < drYperSTEP) && bForceY) Force_Y(false); //Доводка микрошагами
      if (drDY > PI) drDY = 0; //Перестраховка
     }
   }
  if ((drDX<drXperSTEP)&&(drDY<drYperSTEP)) XY.FLXY=0;
  return XY; 
 }

 void ToAZaH (boolean bForceit)
{
 AzAlt=AtXYtoXY(AzAlt,bForceit,1000);
}

void ToRaDe (boolean bForceit)
{
 RaDe=AtXYtoXY(RaDe,bForceit,1000);
}

int SetSMode(int iSM)
{
 int SetSMode=iSMode;
 if(iSM!=iSMode)
  { 
   if(iSM==3) {SetSMode=3; ulMSPS=double(89317792)*dKMSPS;} //Лунная скорость
   if(iSM==2) {SetSMode=2; ulMSPS=double(86400000)*dKMSPS;} //Солнечная скорость
   if(iSM==1) {SetSMode=1; ulMSPS=double(86164091)*dKMSPS;} //Звездная скорость
   if(iSM==0) {SetSMode=0; ulMSPS=double(86164091)*dKMSPS;} //Stop
   drTperMS = drMaxValue/ulMSPS;         //Изменение часового угла за 1 милисекунду
   Force_X(!bForceX); Force_X(!bForceX); //Пересчет параметров шагов двигателя Х
   Force_Y(!bForceY); Force_Y(!bForceY); //Пересчет параметров шагов двигателя У
 }
 return SetSMode;
}

void SetStDX(void)
{
 if(analogRead(DX_SW_PIN)>512) iStDX= -1; else iStDX= 1;
 if(iTMode==2) iStDX=-iStDX; 
}

void SetStDY(void)
{
 if(analogRead(DY_SW_PIN)>512) iStDY=1; else iStDY=-1; 
}

void Shutting (void) // int iExpoz - выдержка сек, если 0 - то минимальная выдержка 
{
 if(iPictures>0)
 { 
  if(iExpoz>=0&&!bShutting&&(ulShutTimer<millis())) {ulShutTimer=millis()+long(iExpoz)*1000+lMinSh; digitalWrite(SHUTTER_PIN, HIGH); bShutting=true;}
  if(bShutting&&(ulShutTimer<millis())) {digitalWrite(SHUTTER_PIN, LOW); bShutting=false; ulShutTimer=millis()+lShInt; iPictures--;}
 }
}

int InitControl(void)
{
 int i, InitControl=0;

 iMJX=0, iMJY=0, iMJZ=0;
 
 for (i=1; i<=20; i++) iMJX+=analogRead(CX_SENCE); iMJX=iMJX/(i-1); //JOYCONTROL defined X
 if((iMJX>=250)&&(iMJX<=750)) {iXYRate=5; InitControl+=1;}
 for (i=1; i<=20; i++) iMJY+=analogRead(CY_SENCE); iMJY=iMJY/(i-1); //JOYCONTROL defined Y
 if((iMJY>=250)&&(iMJY<=750)) {iXYRate=5; InitControl+=2;}
 for (i=1; i<=20; i++) iMJZ+=analogRead(CZ_SENCE); iMJZ=iMJZ/(i-1); //KEYCONTROL defined Z
 if((iMJZ>=250)&&(iMJZ<=750)) {iZRate=3; InitControl+=4;}

 if((InitControl!=3)&&(InitControl!=7)) InitControl=0; //Если ни джойстик ни пульт, то считаем, что это GUIDEPORT
  
 if(InitControl==0) {iXYRate=1; iZRate=0;} //GUIDEPORT defined
 return InitControl;
}

unsigned long AskControl()
{
 unsigned long iRetValue=0;
 int iA1, iA2, iA3, iA4, iDX, iDY, iN;
  
  iA1 = analogRead(CX_SENCE);
  iA2 = analogRead(CY_SENCE);
  iA3 = analogRead(CZ_SENCE);
  iA4 = analogRead(CS_SENCE);

  if (iCtrlEnable==0) //GUIDEPORT ось Х
   {
    iN=0;
    if ((iA2>100)&&(iA4>100)) //Блокировка одновременного гидирования по осям Х,У
    {
     if ((iA1<100)&&(iA3>100)&&((millis()-ulCtrlTimer)>=iGuidDelay)) {iN= 1; ulCtrlTimer=millis();}
     if ((iA3<100)&&(iA1>100)&&((millis()-ulCtrlTimer)>=iGuidDelay)) {iN=-1; ulCtrlTimer=millis();}
    }
   }
   else
   {
    iDX=(iMJX<512)?(iMJX/9):((1023-iMJX)/9);
    iN=(iA1-iMJX)/iDX;
   }
   if (iCtrlEnable==3) iXYRate=(iN<0)?(-iN):(iN); //JOYCONTROL
   if(iTMode==2) iN=-iN;
   if(iN!=0) {P[0]=2; P[1]=16; P[2]=(iN>0)?36:37; P[3]=iXYRate; P[4]=0; P[5]=0; P[6]=0; p();} else {P[0]=0; p();} // X
   if(iN >0) iLastCtrlKey=1;
   if(iN <0) iLastCtrlKey=2;
    
  if (iCtrlEnable==0) //GUIDEPORT ось У
   {
    iN=0;
    if ((iA1>250)&&(iA3>250)) //Блокировка одновременного гидирования по осям Х,У
    {
     if ((iA2<250)&&(iA4>250)&&((millis()-ulCtrlTimer)>=iGuidDelay)) {iN= 1; ulCtrlTimer=millis();}
     if ((iA4<250)&&(iA2>250)&&((millis()-ulCtrlTimer)>=iGuidDelay)) {iN=-1; ulCtrlTimer=millis();}
    }
   }
  else
   {
    iDY=(iMJY<512)?(iMJY/9):((1023-iMJY)/9);
    iN=(iA2-iMJY)/iDY;
   }
   if (iCtrlEnable==3) iXYRate=(iN<0)?(-iN):(iN); //JOYCONTROL
   if(iN!=0) {P[0]=2; P[1]=17; P[2]=(iN>0)?37:36; P[3]=iXYRate; P[4]=0; P[5]=0; P[6]=0; p();} else {P[0]=0; p();} // Y
   if(iN >0) iLastCtrlKey=3;
   if(iN <0) iLastCtrlKey=4;
  
   if((iCtrlEnable==3)&&((millis()-ulCtrlTimer)>iCtrlDelay)) //JOYCONTROL запускает или останавливает трекинг
    {
     if(iA3<500)
     {
      if(iSMode>0) iSMode=SetSMode(0); else {iSMode=SetSMode(1); ulLoopTimer=millis(); ulMilisec=millis();} //Sideral tracking ON
      ulCtrlTimer=millis();
      ulLoopTimer=millis();
      ulMilisec=millis();
     }
    }

  if(iCtrlEnable>3) //KEYCONTROL
   {
    P[0]=5, P[1]=0, P[4]=0, P[5]=0, P[6]=0;
    if(iA3<250){iLastCtrlKey=5; P[2]=37; P[3]=iZRate;} // Z+
    if((iA3>=250)&&(iA3<=650)) {P[2]=39; P[3]=0;     } // Stop Z
    if(iA3>650){iLastCtrlKey=6; P[2]=36; P[3]=iZRate;} // Z-
   
    if((millis()-ulCtrlTimer)>iCtrlDelay)
     {
      if(iA4<250)
      {
       if(iLastCtrlKey<=4) if(iXYRate>1) {digitalWrite(13, HIGH); iXYRate--;} else iSMode=SetSMode(0);
       if(iLastCtrlKey> 4) if(iZRate> 1) {digitalWrite(13, HIGH); iZRate--;}
      }
      if((iA4>=250)&&(iA4<=650)) PJ[3][0]=0;
      if(iA4>650)
      {
       if(iLastCtrlKey<=4) if(iXYRate<9) {digitalWrite(13, HIGH); iXYRate++;}
       if((iLastCtrlKey<=4)&&(iSMode<=0)&&(analogRead(MOUNT_TYPE_PIN)>=750)) {iSMode=SetSMode(1); ulLoopTimer=millis(); ulMilisec=millis();} //Sideral tracking ON
       if(iLastCtrlKey> 4) if(iZRate <9) {digitalWrite(13, HIGH); iZRate++; }
      }
     digitalWrite(13, LOW);
     ulCtrlTimer=millis();
    }
   }
 return iRetValue; 
}

