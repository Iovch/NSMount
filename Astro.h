/*
* Astro.h (Mount) Written by Igor Ovchinnikov 26/11/2017
*/

struct sJDate    {long N; long MN; double H; double MH;} sJD;

double NorRad (double pRad)
 {
  double NorRad=pRad;
  while(NorRad<0) NorRad+=drMaxValue;
  while(NorRad>drMaxValue) NorRad-=drMaxValue;
  return NorRad;
 }

void GJD(void) // Юлианская дата
{
  int A, M;
  long Y;
  AskClock();
  A=(14.0-sDT.MH)/12.0;
  Y=sDT.YY+2000+4800-A;
  M=sDT.MH+12*A-3;
  sJD.N=sDT.DD+long((153*M+2)/5)+365*Y+Y/4-Y/100+Y/400-32045;
  if((sDT.HH-12)<iZH) sJD.N-=1;
  sJD.H=(sDT.HH+sDT.MM/60.0+sDT.SS/3600.0-iZH)/24.0+0.5;
  if(sJD.H>=1.0) sJD.H-=1.0;
}

void MJD(void) //Модифицированная Юлианская дата sJD.N-=2400000 sJD.H-=0.5
{                                 
  GJD();
  sJD.MN=sJD.N-2400000;
  sJD.MH=sJD.H-0.5;
  if(sJD.MH<0.0) {sJD.MH+=1.0;}
  if(sJD.MH>=0.5&&sJD.MH<=1.0) {sJD.MN-=1;}
 }

double GST(void)
 {
   double dT, dUT, dGST, dMJD2000;
   MJD();
   dMJD2000=sJD.N-2451545; //
   dUT=sJD.H*24.0;
   dT =dMJD2000/36525.0;
   dGST=1.0027379093*dUT;
   dGST+=6.697374558;
   dGST+=(8640184.0/3600.0*dT);
   dGST+=(0.812866*dT/3600.0);
   dGST+=(0.093104*dT*dT/3600.0);
   dGST-=(6.2E-6*dT*dT*dT/3600.0);
   dGST=dGST-int(dGST/24.0)*24;
   dGST-=12.0;
   while (dGST < 0.0) dGST+=24.0;
   while (dGST>=24.0) dGST-=24.0;
   return dGST;
 }

double LST(void)
 {
  double dLST;
  dLST=GST();
  dLST=dLST-Longitude/15.0;
  while(dLST <  0.0) dLST+=24.0;
  while(dLST > 24.0) dLST-=24.0;
  return dLST;
 }

double RLST()
{
 return LST()/24.0*drMaxValue; //LST перевели в радианы!
}

double DeFromAzAltLa(double pAz, double pAlt, double pFi)   //Склонение из высоты, азимута и широты (проверено)
{
  double dDe = 0.0;
  dDe = asin(sin(pAlt)*sin(pFi)+cos(pAlt)*cos(pFi)*cos(pAz)); // [-PI/2;+PI/2]
//  if(pAlt>pFi) dDe=PI-dDe; // Проверять это предположение !!!
  return dDe;
};

double TFromAzAltDeLa(double pAz, double pAlt, double pDe, double pFi) //Часовой угол 0 в точке Юг! Азимут 0 в точке Север!
{
  double dT = 0.0;
  while(pAz>(2.0*PI)) pAz-=(2.0*PI);
  dT=acos((sin(pAlt)-sin(pFi)*sin(pDe))/(cos(pFi)*cos(pDe))); // [0,PI]
  if(pAz==0.0)        dT=PI;
  if(pAz>0.0&&pAz<PI) dT=2.0*PI-dT;
  if(pAz==PI)         dT=0.0;
  if(pAz >PI)        {dT-=PI; if(dT<0.0) dT==0.0;}
  if(pAz==(2.0*PI))   dT=PI;
  return dT;
};

double AltFromDeFiT(double pDe, double pFi, double pT)   //Высота из DE и Az
{
 double dAlt = 0.0;
 dAlt=asin(sin(pDe)*sin(pFi)+cos(pDe)*cos(pT)*cos(pFi)); //Это высота
 return dAlt; 
}

double AzFromAltDeFiT(double pAlt, double pDe, double pFi, double pT) //Азимут из склонения и часового угла
{
 double dAz=0.0;
 dAz=sin(pDe)-sin(pAlt)*sin(pFi);
 dAz=acos(dAz/cos(pAlt)/cos(pFi)); // [0,PI]
 if(pT<PI) dAz=2*PI-dAz; // Проверять здесь!
 if(pT==0) dAz==PI;
 if(pT==PI)dAz==0.0;
 return dAz;
}

void RaDeFromAzAlt (void)
{
  RaDe.AtY=DeFromAzAltLa(AzAlt.AtX, AzAlt.AtY, drFi);
  RaDe.AtY=NorRad(RaDe.AtY);
  RaDe.ToY=RaDe.AtY;
  drTH=TFromAzAltDeLa(AzAlt.AtX,AzAlt.AtY, RaDe.AtY, drFi);
  drTH=NorRad(drTH);
  RaDe.AtX=RLST()-drTH;
  RaDe.AtX=NorRad(RaDe.AtX);
  if((cos(AzAlt.AtX)>0)&&(sin(AzAlt.AtY)>sin(drFi*cos(AzAlt.AtX)))) {RaDe.AtX+=PI; drTH+=PI;} //Что-то типа догадки
  if((cos(AzAlt.AtX)<0)&&(sin(AzAlt.AtY)<sin(drFi*cos(AzAlt.AtX)))) {RaDe.AtX+=PI; drTH+=PI;} //Что-то типа догадки
  drTH=NorRad(drTH);
  drToTH=drTH;
  RaDe.AtX=NorRad(RaDe.AtX);
  RaDe.ToX=RaDe.AtX;
}

void AzAltFromRaDe (int iMode)
{
 //iMode==1 расчитываются азимутальные координаты только текущего положения 
 //iMode==2 расчитываются азимутальные координаты только целевого положения 
 //iMode==3 расчитываются азимутальные координаты и целевого и текущего положения 
 
  if (iMode==1||iMode==3)
   { 
    drTH=RLST()-RaDe.AtX;
    drTH=NorRad(drTH);     
    AzAlt.AtY=AltFromDeFiT(RaDe.AtY, drFi, drTH);
    AzAlt.AtY=NorRad(AzAlt.AtY);
    AzAlt.AtX=AzFromAltDeFiT(AzAlt.AtY,RaDe.AtY,drFi,drTH);
    AzAlt.AtX=NorRad(AzAlt.AtX);
   }
  if (iMode==2||iMode==3)
   { 
    drToTH=RLST()-RaDe.ToX;
    drToTH=NorRad(drToTH);
    AzAlt.ToY=AltFromDeFiT(RaDe.ToY, drFi, drToTH);
    AzAlt.ToY=NorRad(AzAlt.ToY);
    AzAlt.ToX=AzFromAltDeFiT(AzAlt.ToY,RaDe.ToY,drFi,drToTH);
    AzAlt.ToX=NorRad(AzAlt.ToX);
   }
}
