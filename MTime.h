/*
* MTime.h (For Mount) Written by Igor Ovchinnikov 28/09/2017
*/

struct sDateTime {int SS; int MM; int HH; int DD; int MH; int YY; unsigned long MS;} sDT;

void SetTime(void)
{
  sDT.HH=H[0];
  sDT.MM=H[1];
  sDT.SS=H[2];
  sDT.MH=H[3];
  sDT.DD=H[4];
  sDT.YY=H[5];
  if(H[6]<=12) iZH=H[6]; else iZH=-(256-H[6]);
  if(H[7]==1)  sDT.HH+=12;
  sDT.MS=millis();
}

void AskClock(void) // Заполняет структуру sDT данными о текущем времени
{
  unsigned long ulTick;
  ulTick=millis()-sDT.MS;
  if(sDT.DD==0) sDT.DD=1;
  if(sDT.MH==0) sDT.MH=1;
  if(ulTick>=1000)
   {
    sDT.MS+=ulTick; sDT.SS+=ulTick/1000;
    while (sDT.SS>=60) {sDT.MM+=1; sDT.SS-=60;}
    while (sDT.MM>=60) {sDT.HH+=1; sDT.MM-=60;}
    while (sDT.HH>=24) {sDT.DD+=1; sDT.HH-=24;}
    if(sDT.DD>28&&sDT.MH==2&&(sDT.YY%4!=0)) {sDT.MH=3; sDT.DD-=28;}
    if(sDT.DD>29&&sDT.MH==2&&(sDT.YY%4==0)) {sDT.MH=3; sDT.DD-=29;}
    if(sDT.DD>30&&(sDT.MH==4||sDT.MH==6||sDT.MH==9||sDT.MH==11)) {sDT.MH+=1; sDT.DD-=30;}
    if(sDT.DD>31&&(sDT.MH==1||sDT.MH==3||sDT.MH==5||sDT.MH==7||sDT.MH==8||sDT.MH==10||sDT.MH==12)) {sDT.MH+=1; sDT.DD-=31;}
    if(sDT.MH>12) {sDT.MH=1; sDT.YY+=1;}
   }
}

void SendTime(void)
{
 // Serial.flush();
  AskClock();
  H[0]=sDT.HH;
  H[1]=sDT.MM;
  H[2]=sDT.SS;
  H[3]=sDT.MH;
  H[4]=sDT.DD;
  H[5]=sDT.YY;
  if(iZH<0) H[6]=256-iZH; else H[6]=iZH;
  H[7]==0;
  Serial.write(H,8);
}

