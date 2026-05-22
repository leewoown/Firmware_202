#include "DSP28x_Project.h"
#include "BATAlgorithm.h"
#include "stdio.h"
#include "math.h"
#include "string.h"

#if FarasisP52Ah
extern void CalFarasis52AhRegsInit(SocReg *P);
extern void CalFarasis52AhSocInit(SocReg *P);
extern void CalFarasis52AhSocHandle(SocReg *P);
#define C_Farasis52Ah_SOCX2     -138.71
#define C_Farasis52Ah_SOCX1     1195.70
#define C_Farasis52Ah_SOCX0    -2472.30
#define C_FarasisP52AhNorm      0.004//  0.0238//1/42Ah
#endif

#if Frey60Ah
extern void CalFrey60AhRegsInit(SocReg *P);
extern void CalFrey60AhSocInit(SocReg *P);
extern void CalFrey60AhSocHandle(SocReg *P);

#define C_Frey60Ah_SOCX0A    3352.4701
#define C_Frey60Ah_SOCX1A    -20858.4935
#define C_Frey60Ah_SOCX2A    32422.54


#define C_Frey60Ah_SOCX2B    21.8172
#define C_Frey60Ah_SOCX1B    1.8087
#define C_Frey60Ah_SOCX0B    -216.32

#define C_Frey60Ah_SOCX2C    1.4655
#define C_Frey60Ah_SOCX1C    4.8170
#define C_Frey60Ah_SOCX0C    15.83


#define C_Frey60Ah_SOCX2D    -3344.4816
#define C_Frey60Ah_SOCX1D    22607.0234
#define C_Frey60Ah_SOCX0D   - 38111.77

#define C_Frey60Ah_SOCX2E    0
#define C_Frey60Ah_SOCX1E    1499.9998
#define C_Frey60Ah_SOCX0E    -4912.50


#define C_Frey60AhNorm       0.002631//1/60Ah;


#endif

#if Kokam100Ah
void CalKokam100AhRegs(void)
{

}
void CalKokam100AhSocInit(void)
{

}

void Calkokam100AhSocHandle(void)
{

}
#endif

#if Kokam60Ah
void CalKokam60AhRegs(void)
{

}
void CalKokam60AhSocInit(void)
{

}

void Calkokam60AhSocHandle(void)
{

}

#endif



#if FarasisP52Ah
void CalFarasis52AhRegsInit(SocReg *P)
{
    P->SysSOCdtF=0.0;
    P->SysSoCCTF=0.0;
    P->SysPackAhNewF=0.0;
    P->SysPackAhOldF=0.0;
    P->SysPackAhF=0.0;
    P->SysPackSOCBufF1=0.0;
    P->SysPackSOCBufF2=0.0;
    P->SysPackSOCF=5.0;
    P->AVGXF=0.0;
    P->SOCX4InF=0.0;
    P->SOCX3InF=0.0;
    P->SOCX2InF=0.0;
    P->SOCX1InF=0.0;
    P->SOCX4OutF=0.0;
    P->SOCX3OutF=0.0;
    P->SOCX2OutF=0.0;
    P->SOCX1OutF=0.0;
    P->SOCbufF=0.0;
    P->SysSocInitF=0.0;
    P->CellAgvVoltageF=0.0;
    P->SoCStateRegs.all=0;
    P->CTCount=0;
    P->SysTime=0;
    P->SysSoCCTAbsF=0;
    P->state=SOC_STATE_IDLE;
}
void CalFarasis52AhSocInit(SocReg *P)
{

    // 60Ah
     P->AVGXF         =   P->CellAgvVoltageF;
     P->SOCX2InF      =   P->AVGXF  * P->AVGXF;
     P->SOCX1InF      =   P->AVGXF;
     P->SOCX2OutF     =   C_Farasis52Ah_SOCX2 * P->SOCX2InF;
     P->SOCX1OutF     =   C_Farasis52Ah_SOCX1 * P->SOCX1InF;
     P->SOCbufF       =   P->SOCX2OutF + P->SOCX1OutF + C_Farasis52Ah_SOCX0;
     /*
      *  占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 占십울옙占쏙옙
      */
     if((P->SOCbufF >= 0.0)&&(P->SOCbufF < 20.0))
     {
         P->SOCbufF  =   P->SOCbufF-2;
     }
     if((P->SOCbufF >= 20)&&(P->SOCbufF < 40.0))
     {
         P->SOCbufF  =   P->SOCbufF+3;
     }
     if((P->SOCbufF >= 40)&&(P->SOCbufF < 80.0))
     {
         P->SOCbufF  =   P->SOCbufF+3.0;
     }
     if(P->SOCbufF >= 80.0)
     {
         P->SOCbufF  =   P->SOCbufF-3;
     }
     /*
      *
      */
     if(P->SOCbufF <=0.0)
     {
         P->SOCbufF = 0.0;
     }
     else if(P->SOCbufF > 98.0)
     {
         P->SOCbufF = 100.0;
     }
     P->SysSocInitF = P->SOCbufF;
}

//CalSocKokamInit(&KokamSocRegs);
//KokamSocRegs.state = SOC_STATE_RUNNING;
void CalFarasis52AhSocHandle(SocReg *P)
{
    P->SysTime++;
    if(P->SysTime>=C_SocSamPleCount)
    {
        if(P->SysSoCCTAbsF>=C_SocInitCTVaule)
        {
            P->SoCStateRegs.bit.CalMeth=1;
            P->CTCount=0;
        }
        else
        {
            P->CTCount++;
            if(P->CTCount>6000)
            {
                P->CTCount=6001;
                P->SoCStateRegs.bit.CalMeth=0;
            }
        }
        switch (P->state)
        {

            case SOC_STATE_RUNNING:
                 if(P->SoCStateRegs.bit.CalMeth==0)
                 {
                     /*
                      *
                      */
                     //52Ah

                     P->AVGXF         =   P->CellAgvVoltageF;
                     P->SOCX2InF      =   P->AVGXF  * P->AVGXF;
                     P->SOCX1InF      =   P->AVGXF;
                     P->SOCX2OutF     =   C_Farasis52Ah_SOCX2 * P->SOCX2InF;
                     P->SOCX1OutF     =   C_Farasis52Ah_SOCX1 * P->SOCX1InF;
                     P->SOCbufF       =   P->SOCX2OutF + P->SOCX1OutF + C_Farasis52Ah_SOCX0;
                     P->SOCbufF       =   P->SOCbufF+3.0;
                     /*
                      *  占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙 占십울옙占쏙옙
                      */
                     if((P->SOCbufF >= 0.0)&&(P->SOCbufF < 20.0))
                     {
                         P->SOCbufF  =   P->SOCbufF-2;
                     }
                     if((P->SOCbufF >= 20)&&(P->SOCbufF < 40.0))
                     {
                         P->SOCbufF  =   P->SOCbufF+3;
                     }
                     if((P->SOCbufF >= 40)&&(P->SOCbufF < 80.0))
                     {
                         P->SOCbufF  =   P->SOCbufF+3.0;
                     }
                     if(P->SOCbufF >= 80.0)
                     {
                         P->SOCbufF  =   P->SOCbufF-3;
                     }
                     /*
                      *
                      */
                     if(P->SOCbufF <=0.0)
                     {
                         P->SOCbufF = 0.0;
                     }
                     else if(P->SOCbufF > 98.0)
                     {
                         P->SOCbufF = 100.0;
                     }
                      if(P->SOCbufF <=0.0)
                      {
                          P->SOCbufF = 0.0;
                      }
                      else if(P->SOCbufF > 98.0)
                      {
                          P->SOCbufF = 100.0;
                      }
                      P->SysSocInitF = P->SOCbufF;
                      P->SysPackSOCF = P->SOCbufF;
                 }
                 if(P->SoCStateRegs.bit.CalMeth==1)
                 {
                     /*
                      *
                      */
                     P->SysSOCdtF = C_CTSampleTime*C_SocCumulativeTime; // CumulativeTime(1/3600) -> 占쏙옙占쏙옙占시곤옙
                     P->SysPackAhNewF = P->SysSoCCTF * P->SysSOCdtF;
                     P->SysPackAhF    = P->SysPackAhNewF + P->SysPackAhOldF;
                     P->SysPackAhOldF = P->SysPackAhF;
                     if(P->SysPackAhF <= -250.0)
                     {
                        P->SysPackAhF =-250.0;
                     }
                     if(P->SysPackAhF> 250.0)
                     {
                         P->SysPackAhF= 250.0;
                     }
                     /*
                     * SOC 占쏙옙환
                     */
                     P->SysPackSOCBufF1 = P->SysPackAhF *C_FarasisP52AhNorm;//0.0125 ;// 1/80 --> 0.0125--> 占싹뱄옙화
                     P->SysPackSOCBufF2 = P->SysPackSOCBufF1*100.0; //--> 占쏙옙占쏙옙 占쏙옙환 %
                     P->SysPackSOCF     = P->SysSocInitF+P->SysPackSOCBufF2;
                 }
                 P->state = SOC_STATE_Save;

            break;
            case SOC_STATE_Save:

                P->state = SOC_STATE_RUNNING;

            break;
            case SOC_STATE_CLEAR:

            break;
        }
        P->SysTime=0;
    }
}
#endif

#if Frey60Ah
void CalFrey60AhRegsInit(SocReg *P)
{
    P->SysSOCdtF=0.0;
    P->SysSoCCTF=0.0;
    P->SysPackAhNewF=0.0;
    P->SysPackAhOldF=0.0;
    P->SysPackAhF=0.0;
    P->SysPackSOCBufF1=0.0;
    P->SysPackSOCBufF2=0.0;
    P->SysPackSOCF=5.0;
    P->AVGXF=0.0;
    P->SOCX2InF=0.0;
    P->SOCX1InF=0.0;
    P->SOCX3InF=0.0;
    P->SOCX4InF=0.0;
    P->SOCX2OutF=0.0;
    P->SOCX1OutF=0.0;
    P->SOCX3OutF=0.0;
    P->SOCX4OutF=0.0;

    P->SOCX4InFAZore=0.0;
    P->SOCX3InFAZore=0.0;
    P->SOCX2InFAZore=0.0;
    P->SOCX1InFAZore=0.0;
    P->SOCX4OutFAZore=0.0;
    P->SOCX3OutFAZore=0.0;
    P->SOCX2OutFAZore=0.0;
    P->SOCX1OutFAZore=0.0;
    P->AZoreCalCout=0;



    P->SOCX4InFBZore=0.0;
    P->SOCX3InFBZore=0.0;
    P->SOCX2InFBZore=0.0;
    P->SOCX1InFBZore=0.0;
    P->SOCX4OutFBZore=0.0;
    P->SOCX3OutFBZore=0.0;
    P->SOCX2OutFBZore=0.0;
    P->SOCX1OutFBZore=0.0;
    P->BZoreCalCout=0.0;


    P->SOCX4InFCZore=0.0;
    P->SOCX3InFCZore=0.0;
    P->SOCX2InFCZore=0.0;
    P->SOCX1InFCZore=0.0;
    P->SOCX4OutFCZore=0.0;
    P->SOCX3OutFCZore=0.0;
    P->SOCX2OutFCZore=0.0;
    P->SOCX1OutFCZore=0.0;
    P->CZoreCalCout=0;

    P->SOCX4InFDZore=0.0;
    P->SOCX3InFDZore=0.0;
    P->SOCX2InFDZore=0.0;
    P->SOCX1InFDZore=0.0;
    P->SOCX4OutFDZore=0.0;
    P->SOCX3OutFDZore=0.0;
    P->SOCX2OutFDZore=0.0;
    P->SOCX1OutFDZore=0.0;
    P->DZoreCalCout=0;


    P->SOCX4InFEZore=0.0;
    P->SOCX3InFEZore=0.0;
    P->SOCX2InFEZore=0.0;
    P->SOCX1InFEZore=0.0;
    P->SOCX4OutFEZore=0.0;
    P->SOCX3OutFEZore=0.0;
    P->SOCX2OutFEZore=0.0;
    P->SOCX1OutFEZore=0.0;
    P->EZoreCalCout=0;




    P->SOCbufF=0.0;
    P->SysSocInitF=0.0;
    P->CellAgvVoltageF=0.0;
    P->SoCStateRegs.all=0;
    P->CTCount=0;
    P->SysTime=0;
    P->SysSoCCTAbsF=0;
    P->state=SOC_STATE_IDLE;
}
void CalFrey60AhSocInit(SocReg *P)
{

    /*
     * #define C_Frey60Ah_SOCX0A    3352.4701
       #define C_Frey60Ah_SOCX1A    -20858.4935
       #define C_Frey60Ah_SOCX2A    32422.54


       #define C_Frey60Ah_SOCX2B    12365.5914
       #define C_Frey60Ah_SOCX1B    -80393.0108
       #define C_Frey60Ah_SOCX0B    130685.77

       #define C_Frey60Ah_SOCX2C    1.4655
       #define C_Frey60Ah_SOCX1C    4.8170
       #define C_Frey60Ah_SOCX0C    15.83


       #define C_Frey60Ah_SOCX2D    -3344.4816
       #define C_Frey60Ah_SOCX1D    22607.0234
       #define C_Frey60Ah_SOCX0D   - 38111.77

       #define C_Frey60Ah_SOCX2E    0
       #define C_Frey60Ah_SOCX1E    1499.9998
       #define C_Frey60Ah_SOCX0E    -4912.50
     */


    // 60Ah
      P->AVGXF         =   P->CellAgvVoltageF;
         //IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_A_BOT, LFP_VOLT_A_TOP)  ((A) >  (MIN) && (A) <= (MAX))  // 占십곤옙 ~ 占쏙옙占쏙옙
      if(IS_ABOVE_AND_UNDER(P->AVGXF , LFP_VOLT_A_BOT, LFP_VOLT_A_TOP))
      {
          if(P->AVGXF<=3.03)
          {
              P->SOCbufF =0.0;
          }
          if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.03, 3.2))
          {
              P->SOCbufF =5.0;
          }
          if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.2, 3.21))
          {
              P->SOCbufF =10.0;
          }
          if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.21, 3.215))
          {
              P->SOCbufF =15.0;
          }
          //#define LFP_VOLT_A_BOT   3.030
          //#define LFP_VOLT_A_TOP   3.215
          //#define C_Frey60Ah_SOCX0A    3352.4701
          //#define C_Frey60Ah_SOCX1A    -20858.4935
          //#define C_Frey60Ah_SOCX2A    32422.54
      /*    P->AZoreCalCout++;
          P->SOCX4InFAZore = 0;
          P->SOCX3InFAZore = 0;
          P->SOCX2InFAZore = P->AVGXF*P->AVGXF;
          P->SOCX1InFAZore = P->AVGXF;

          P->SOCX4OutFAZore = 0;
          P->SOCX3OutFAZore = 0;
          P->SOCX2OutFAZore = C_Frey60Ah_SOCX2A* P->SOCX2InFAZore;
          P->SOCX1OutFAZore = C_Frey60Ah_SOCX1A* P->SOCX1InFAZore;
          P->SOCbufF        = P->SOCX2OutFAZore+P->SOCX1OutFAZore+ C_Frey60Ah_SOCX0A;*/
          if(P->AZoreCalCout>3600)
          {
              P->AZoreCalCout=0;
          }
      }
     // IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_B_BOT, LFP_VOLT_B_TOP)  ((A) >  (MIN) && (A) <= (MAX))  // 占십곤옙 ~ 占쏙옙占쏙옙
      if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_B_BOT, LFP_VOLT_B_TOP))
      {
          //#define C_Frey60Ah_SOCX2B    2566.7
          //#define C_Frey60Ah_SOCX1B    -16384
          //#define C_Frey60Ah_SOCX0B     26156
          P->BZoreCalCout++;
          P->SOCX4InFBZore = 0;
          P->SOCX3InFBZore = 0;

          P->SOCX2InFBZore = P->AVGXF*P->AVGXF;
          P->SOCX1InFBZore = P->AVGXF;

          P->SOCX4OutFBZore = 0;
          P->SOCX3OutFBZore = 0;
          P->SOCX2OutFBZore = C_Frey60Ah_SOCX2B*P->SOCX2InFBZore;
          P->SOCX1OutFBZore = C_Frey60Ah_SOCX1B*P->SOCX1InFBZore;
          P->SOCbufF        = P->SOCX2OutFBZore + P->SOCX1OutFBZore+C_Frey60Ah_SOCX0B;
          if(P->BZoreCalCout>3600)
           {
               P->BZoreCalCout=0;
           }
      }

      if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_C_BOT, LFP_VOLT_C_TOP))
      {

          P->CZoreCalCout++;
          //#define C_Frey60Ah_SOCX1C    1313.1
          //#define C_Frey60Ah_SOCX0C   -4276.6
          P->SOCX4InFCZore = 0;
          P->SOCX3InFCZore = 0;
          P->SOCX2InFCZore = P->AVGXF*P->AVGXF;
          P->SOCX1InFCZore = P->AVGXF;

          P->SOCX4OutFCZore = 0;
          P->SOCX3OutFCZore = 0;
          P->SOCX2OutFCZore = C_Frey60Ah_SOCX2C*P->SOCX2InFCZore;
          P->SOCX1OutFCZore = C_Frey60Ah_SOCX1C*P->SOCX1InFCZore;
          P->SOCbufF        = P->SOCX2OutFCZore+P->SOCX1OutFCZore + C_Frey60Ah_SOCX0C;

          if(P->CZoreCalCout>3600)
           {
               P->CZoreCalCout=0;
           }
      }
      if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_D_BOT, LFP_VOLT_D_TOP))
      {
          P->DZoreCalCout++;
          P->SOCX4InFDZore = 0;
          P->SOCX3InFDZore = 0;
          P->SOCX2InFDZore = P->AVGXF*P->AVGXF;
          P->SOCX1InFDZore = P->AVGXF;

          P->SOCX4OutFDZore = 0;
          P->SOCX3OutFDZore = 0;
          P->SOCX2OutFDZore = C_Frey60Ah_SOCX2D*P->SOCX2InFDZore;
          P->SOCX1OutFDZore = C_Frey60Ah_SOCX1D*P->SOCX1InFDZore;
          P->SOCbufF        = P->SOCX2OutFDZore+P->SOCX1OutFDZore + C_Frey60Ah_SOCX0D;
          if(P->DZoreCalCout>3600)
           {
               P->DZoreCalCout=0;
           }
      }
      if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_E_BOT, LFP_VOLT_E_TOP))
      {
          P->EZoreCalCout++;
          P->SOCX4InFEZore = 0;
          P->SOCX3InFEZore = 0;
          P->SOCX2InFEZore = 0;
          P->SOCX1InFEZore = P->AVGXF;

          P->SOCX4OutFEZore = 0;
          P->SOCX3OutFEZore = 0;
          P->SOCX2OutFEZore = 0;
          P->SOCX1OutFEZore = C_Frey60Ah_SOCX1E*P->SOCX1InFEZore;
          P->SOCbufF        = P->SOCX1OutFEZore + C_Frey60Ah_SOCX0E;
          if(P->DZoreCalCout>3600)
           {
               P->DZoreCalCout=0;
           }
      }
     if(P->SOCbufF <=0.0)
     {
         P->SOCbufF = 0.0;
     }
     else if(P->SOCbufF > 90.0)
     {
         P->SOCbufF = 100.0;
     }
     P->SysSocInitF = P->SOCbufF;
}

void CalFrey60AhSocHandle(SocReg *P)
{
    P->SysTime++;
    P->AVGXF         =   P->CellAgvVoltageF;
    if(P->SysTime>=C_SocSamPleCount)
     {
         if(P->SysSoCCTAbsF>=C_SocInitCTVaule)
         {
             P->SoCStateRegs.bit.CalMeth=1;
             P->CTCount=0;
         }
         else
         {
             P->CTCount++;
             if(P->CTCount>6000)
             {
                 P->CTCount=6001;
                 P->SoCStateRegs.bit.CalMeth=0;
             }
         }
         switch (P->state)
         {

             case SOC_STATE_RUNNING:
                  if(P->SoCStateRegs.bit.CalMeth==0)
                  {
                      // 60Ah
                      P->AVGXF         =   P->CellAgvVoltageF;
                          //IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_A_BOT, LFP_VOLT_A_TOP)  ((A) >  (MIN) && (A) <= (MAX))  // 占십곤옙 ~ 占쏙옙占쏙옙
                       if(IS_ABOVE_AND_UNDER(P->AVGXF , LFP_VOLT_A_BOT, LFP_VOLT_A_TOP))
                       {
                           if(P->AVGXF<=3.03)
                           {
                               P->SOCbufF =0.0;
                           }
                           if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.03, 3.2))
                           {
                               P->SOCbufF =5.0;
                           }
                           if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.2, 3.21))
                           {
                               P->SOCbufF =10.0;
                           }
                           if(IS_ABOVE_AND_UNDER(P->AVGXF ,3.21, 3.215))
                           {
                               P->SOCbufF =15.0;
                           }
                           //#define LFP_VOLT_A_BOT   3.030
                           //#define LFP_VOLT_A_TOP   3.215
                           //#define C_Frey60Ah_SOCX0A    3352.4701
                           //#define C_Frey60Ah_SOCX1A    -20858.4935
                           //#define C_Frey60Ah_SOCX2A    32422.54
                       /*    P->AZoreCalCout++;
                           P->SOCX4InFAZore = 0;
                           P->SOCX3InFAZore = 0;
                           P->SOCX2InFAZore = P->AVGXF*P->AVGXF;
                           P->SOCX1InFAZore = P->AVGXF;

                           P->SOCX4OutFAZore = 0;
                           P->SOCX3OutFAZore = 0;
                           P->SOCX2OutFAZore = C_Frey60Ah_SOCX2A* P->SOCX2InFAZore;
                           P->SOCX1OutFAZore = C_Frey60Ah_SOCX1A* P->SOCX1InFAZore;
                           P->SOCbufF        = P->SOCX2OutFAZore+P->SOCX1OutFAZore+ C_Frey60Ah_SOCX0A;*/
                           if(P->AZoreCalCout>3600)
                           {
                               P->AZoreCalCout=0;
                           }
                       }
                      // IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_B_BOT, LFP_VOLT_B_TOP)  ((A) >  (MIN) && (A) <= (MAX))  // 占십곤옙 ~ 占쏙옙占쏙옙
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_B_BOT, LFP_VOLT_B_TOP))
                       {
                           //#define C_Frey60Ah_SOCX2B    2566.7
                           //#define C_Frey60Ah_SOCX1B    -16384
                           //#define C_Frey60Ah_SOCX0B     26156
                           P->BZoreCalCout++;
                           P->SOCX4InFBZore = 0;
                           P->SOCX3InFBZore = 0;

                           P->SOCX2InFBZore = P->AVGXF*P->AVGXF;
                           P->SOCX1InFBZore = P->AVGXF;

                           P->SOCX4OutFBZore = 0;
                           P->SOCX3OutFBZore = 0;
                           P->SOCX2OutFBZore = C_Frey60Ah_SOCX2B*P->SOCX2InFBZore;
                           P->SOCX1OutFBZore = C_Frey60Ah_SOCX1B*P->SOCX1InFBZore;
                           P->SOCbufF        = P->SOCX2OutFBZore + P->SOCX1OutFBZore+C_Frey60Ah_SOCX0B;
                           if(P->BZoreCalCout>3600)
                            {
                                P->BZoreCalCout=0;
                            }
                       }

                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_C_BOT, LFP_VOLT_C_TOP))
                       {

                           P->CZoreCalCout++;
                           //#define C_Frey60Ah_SOCX1C    1313.1
                           //#define C_Frey60Ah_SOCX0C   -4276.6
                           P->SOCX4InFCZore = 0;
                           P->SOCX3InFCZore = 0;
                           P->SOCX2InFCZore = P->AVGXF*P->AVGXF;
                           P->SOCX1InFCZore = P->AVGXF;

                           P->SOCX4OutFCZore = 0;
                           P->SOCX3OutFCZore = 0;
                           P->SOCX2OutFCZore = C_Frey60Ah_SOCX2C*P->SOCX2InFCZore;
                           P->SOCX1OutFCZore = C_Frey60Ah_SOCX1C*P->SOCX1InFCZore;
                           P->SOCbufF        = P->SOCX2OutFCZore+P->SOCX1OutFCZore + C_Frey60Ah_SOCX0C;

                           if(P->CZoreCalCout>3600)
                            {
                                P->CZoreCalCout=0;
                            }
                       }
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_D_BOT, LFP_VOLT_D_TOP))
                       {
                           P->DZoreCalCout++;
                           P->SOCX4InFDZore = 0;
                           P->SOCX3InFDZore = 0;
                           P->SOCX2InFDZore = P->AVGXF*P->AVGXF;
                           P->SOCX1InFDZore = P->AVGXF;

                           P->SOCX4OutFDZore = 0;
                           P->SOCX3OutFDZore = 0;
                           P->SOCX2OutFDZore = C_Frey60Ah_SOCX2D*P->SOCX2InFDZore;
                           P->SOCX1OutFDZore = C_Frey60Ah_SOCX1D*P->SOCX1InFDZore;
                           P->SOCbufF        = P->SOCX2OutFDZore+P->SOCX1OutFDZore + C_Frey60Ah_SOCX0D;
                           if(P->DZoreCalCout>3600)
                            {
                                P->DZoreCalCout=0;
                            }
                       }
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_E_BOT, LFP_VOLT_E_TOP))
                       {
                           P->EZoreCalCout++;
                           P->SOCX4InFEZore = 0;
                           P->SOCX3InFEZore = 0;
                           P->SOCX2InFEZore = 0;
                           P->SOCX1InFEZore = P->AVGXF;

                           P->SOCX4OutFEZore = 0;
                           P->SOCX3OutFEZore = 0;
                           P->SOCX2OutFEZore = 0;
                           P->SOCX1OutFEZore = C_Frey60Ah_SOCX1E*P->SOCX1InFEZore;
                           P->SOCbufF        = P->SOCX1OutFEZore + C_Frey60Ah_SOCX0E;
                           if(P->DZoreCalCout>3600)
                            {
                                P->DZoreCalCout=0;
                            }
                       }
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_F_BOT, LFP_VOLT_F_TOP))
                       {
                           P->FZoreCalCout++;
                           P->SOCbufF        = 92.0;
                           if(P->FZoreCalCout>3600)
                            {
                                P->FZoreCalCout=0;
                            }
                       }
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_G_BOT, LFP_VOLT_G_TOP))
                       {
                           P->GZoreCalCout++;
                           P->SOCbufF        = 95.0;
                           if(P->GZoreCalCout>3600)
                            {
                                P->GZoreCalCout=0;
                            }
                       }
                       if(IS_ABOVE_AND_UNDER(P->AVGXF, LFP_VOLT_H_BOT, LFP_VOLT_H_TOP))
                       {
                           P->HZoreCalCout++;
                           P->SOCbufF        = 98.0;
                           if(P->HZoreCalCout>3600)
                            {
                                P->HZoreCalCout=0;
                            }
                       }
                       P->SysSocInitF = P->SOCbufF;
                      // P->SysPackSOCF = P->SOCbufF;
                  }
                  if(P->SoCStateRegs.bit.CalMeth==1)
                  {
                      /*
                       *
                       */
                      P->SysSOCdtF = C_CTSampleTime*C_SocCumulativeTime; // CumulativeTime(1/3600) -> 占쏙옙占쏙옙占시곤옙
                      P->SysPackAhNewF = P->SysSoCCTF * P->SysSOCdtF;
                      P->SysPackAhF    = P->SysPackAhNewF + P->SysPackAhOldF;
                      P->SysPackAhOldF = P->SysPackAhF;
                      if(P->SysPackAhF <= -380.0)
                      {
                         P->SysPackAhF =-380.0;
                      }
                      if(P->SysPackAhF> 380.0)
                      {
                          P->SysPackAhF= 380.0;
                      }
                      /*
                      * SOC 占쏙옙환
                      */
                      P->SysPackSOCBufF1 = P->SysPackAhF *C_EVE368AhNorm;// TODO: Change this number when we use Frey60Ah cell.
                      P->SysPackSOCBufF2 = P->SysPackSOCBufF1*100.0; //--> 占쏙옙占쏙옙 占쏙옙환 %
                      P->SysPackSOCF     = P->SysSocInitF+P->SysPackSOCBufF2;
                  }
                  P->state = SOC_STATE_Save;

             break;
             case SOC_STATE_Save:

                 P->state = SOC_STATE_RUNNING;

             break;
             case SOC_STATE_CLEAR:

             break;
         }
         P->SysTime=0;
     }
}

#endif
#define C_EVE368AhNorm        0.0027174f// 1/368Ah (Pack 460Ah * DoD 80%)
extern void CalEVE240AhRegsInit(SocReg *P);
extern void CalEVE240AhSocInit(SocReg *P);
extern void CalEVE240AhSocHandle(SocReg *P);
//void hermite_soc_40_60(SocReg *P);

#if EVE24060Ah

const OCVPoint EVE_LF230_OCV_TABLE[OCV_TABLE_SIZE] =
{
    /*  OCV(V),   Disp SOC(%) */
    {  3.160f,    0.00f  },   /* Phys 10% = Empty       */
    {  3.210f,    6.25f  },   /* Phys 15%               */
    {  3.250f,   12.50f  },   /* Phys 20%               */
    {  3.280f,   18.75f  },   /* Phys 25%               */
    {  3.295f,   25.00f  },   /* Phys 30% = Flat starts */
    {  3.305f,   31.25f  },   /* Phys 35%               */
    {  3.310f,   37.50f  },   /* Phys 40%               */
    {  3.315f,   43.75f  },   /* Phys 45%               */
    {  3.318f,   50.00f  },   /* Phys 50%               */
    {  3.320f,   56.25f  },   /* Phys 55%               */
    {  3.322f,   62.50f  },   /* Phys 60%               */
    {  3.325f,   68.75f  },   /* Phys 65%               */
    {  3.328f,   75.00f  },   /* Phys 70%               */
    {  3.330f,   81.25f  },   /* Phys 75%               */
    {  3.333f,   87.50f  },   /* Phys 80%               */
    {  3.340f,   93.75f  },   /* Phys 85% = Flat ends   */
    {  3.360f,  100.00f  }    /* Phys 90% = Full        */
};

void CalEVE240AhRegsInit(SocReg *P)
{
    P->SysSOCdtF=0.0;
    P->SysSoCCTF=0.0;
    P->SysPackAhNewF=0.0;
    P->SysPackAhOldF=0.0;
    P->SysPackAhF=0.0;
    P->SysPackSOCBufF1=0.0;
    P->SysPackSOCBufF2=0.0;
    P->SysPackSOCF=5.0;
    P->AVGXF=0.0;

    P->SOCX2InF=0.0;
    P->SOCX1InF=0.0;
    P->SOCX3InF=0.0;
    P->SOCX4InF=0.0;

    P->SOCX2OutF=0.0;
    P->SOCX1OutF=0.0;
    P->SOCX3OutF=0.0;
    P->SOCX4OutF=0.0;

    P->SOCX4InFAZore=0.0;
    P->SOCX3InFAZore=0.0;
    P->SOCX2InFAZore=0.0;
    P->SOCX1InFAZore=0.0;
    P->SOCX4OutFAZore=0.0;
    P->SOCX3OutFAZore=0.0;
    P->SOCX2OutFAZore=0.0;
    P->SOCX1OutFAZore=0.0;
    P->AZoreCalCout=0;



    P->SOCX4InFBZore=0.0;
    P->SOCX3InFBZore=0.0;
    P->SOCX2InFBZore=0.0;
    P->SOCX1InFBZore=0.0;
    P->SOCX4OutFBZore=0.0;
    P->SOCX3OutFBZore=0.0;
    P->SOCX2OutFBZore=0.0;
    P->SOCX1OutFBZore=0.0;
    P->BZoreCalCout=0.0;


    P->SOCX4InFCZore=0.0;
    P->SOCX3InFCZore=0.0;
    P->SOCX2InFCZore=0.0;
    P->SOCX1InFCZore=0.0;
    P->SOCX4OutFCZore=0.0;
    P->SOCX3OutFCZore=0.0;
    P->SOCX2OutFCZore=0.0;
    P->SOCX1OutFCZore=0.0;
    P->CZoreCalCout=0;

    P->SOCX4InFDZore=0.0;
    P->SOCX3InFDZore=0.0;
    P->SOCX2InFDZore=0.0;
    P->SOCX1InFDZore=0.0;
    P->SOCX4OutFDZore=0.0;
    P->SOCX3OutFDZore=0.0;
    P->SOCX2OutFDZore=0.0;
    P->SOCX1OutFDZore=0.0;
    P->DZoreCalCout=0;


    P->SOCX4InFEZore=0.0;
    P->SOCX3InFEZore=0.0;
    P->SOCX2InFEZore=0.0;
    P->SOCX1InFEZore=0.0;
    P->SOCX4OutFEZore=0.0;
    P->SOCX3OutFEZore=0.0;
    P->SOCX2OutFEZore=0.0;
    P->SOCX1OutFEZore=0.0;
    P->EZoreCalCout=0;

    P->SOCbufF=0.0;
    P->SysSocInitF=0.0;
    P->CellAgvVoltageF=0.0;
    P->SoCStateRegs.all=0;
    P->CTCount=0;
    P->SysTime=0;
    P->SysSoCCTAbsF=0;
    P->state=SOC_STATE_IDLE;

}
// TODO: Implement Hermite interpolation for SOC calculation

// void hermite_soc_40_60(SocReg *P)
// {
//     // Hermite basis with position & slope matching at both ends
//     const float32 x0 = H_V0;
//     const float32 x1 = H_V1;
//     const float32 y0 = H_S0;
//     const float32 y1 = H_S1;
//     const float32 m0 = H_M0;
//     const float32 m1 = H_M1;

//     // 占쏙옙占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙占쏙옙 클占쏙옙占쏙옙
//     P->AVGXF = CLAMP(P->CellAgvVoltageF , x0, x1);

//     const float32 dx = x1 - x0;             // ~0.0099 V
//     const float32 t  = (P->AVGXF  - x0)/ dx; // 0..1
//     const float32 t2= t*t; //bufB = bufA  * bufA ;
//     const float32 t3= t2*t;//bufC = bufB  * bufA ;


//     const float32 h00 = 2.f*t3 - 3.f*t2 + 1.f;    // 2t^3 - 3t^2 + 1
//     const float32 h10 =  t3 - 2.f*t2 + t;         // t^3 - 2t^2 + t
//     const float32 h01 = -2.f*t3 + 3.f*t2;         // -2t^3 + 3t^2
//     const float32 h11 = t3 -     t2;              // t^3 - t^2

//     P->SOCX1OutFCZore = h00*y0 + h10*dx*m0 + h01*y1 + h11*dx*m1;
// }

void CalEVE240AhSocInit(SocReg *P)
{
    Uint16  i;
    float32 v0;
    float32 v1;
    float32 s0;
    float32 s1;
    float32 ocv;

    if(P == (SocReg *)0)
    {
        return;
    }

    ocv = P->CellAgvVoltageF;

    /* Limit OCV inside table range */
    if(ocv <= EVE_LF230_OCV_TABLE[0].ocv)
    {
        P->AVGXF       = EVE_LF230_OCV_TABLE[0].ocv;
        P->SysSocInitF = EVE_LF230_OCV_TABLE[0].dispSoc;
        return;
    }
    if(ocv >= EVE_LF230_OCV_TABLE[OCV_TABLE_SIZE - 1u].ocv)
    {
        P->AVGXF       = EVE_LF230_OCV_TABLE[OCV_TABLE_SIZE - 1u].ocv;
        P->SysSocInitF = EVE_LF230_OCV_TABLE[OCV_TABLE_SIZE - 1u].dispSoc;
        return;
    }

    P->AVGXF = ocv;

    /* Find SOC by linear method between two table points */
    for(i = 0u; i < (OCV_TABLE_SIZE - 1u); i++)
    {
        if(ocv < EVE_LF230_OCV_TABLE[i + 1u].ocv)
        {
            v0 = EVE_LF230_OCV_TABLE[i].ocv;
            v1 = EVE_LF230_OCV_TABLE[i + 1u].ocv;
            s0 = EVE_LF230_OCV_TABLE[i].dispSoc;
            s1 = EVE_LF230_OCV_TABLE[i + 1u].dispSoc;

            P->SysSocInitF = s0 + ((s1 - s0) * (ocv - v0)) / (v1 - v0);
            return;
        }
    }

    /* Safety - this line should not run */
    P->SysSocInitF = EVE_LF230_OCV_TABLE[OCV_TABLE_SIZE - 1u].dispSoc;
}
void CalEVE240AhSocHandle(SocReg *P)
{
    /* 1ms tick 누적 */
    P->SysTime++;
    /* 평균 셀 전압 갱신 */
    P->AVGXF = P->CellAgvVoltageF;

    /* 50ms 주기 게이팅 : 50회 미만이면 대기 상태로 종료 */
    if(P->SysTime < (Uint16)C_SocSamPleCount)
    {
        P->state = SOC_STATE_CalWaitMode;
        return;
    }
    P->SysTime = 0u;

    /* INITOK 확인 : 셀 정보(전압/온도) 1회 이상 수집 전에는 SOC 산출 안 함 */
    if(P->SoCStateRegs.bit.INITOK == 0u)
    {
        return;
    }

    /* ---- CalMeth 결정 (전류 크기 기준) ---- */
    if(P->SysSoCCTAbsF >= C_SocInitCTVaule)    /* |전류| >= 2.5A : 충방전 */
    {
        P->SoCStateRegs.bit.CalMeth = 1u;      /* 전류적산 모드 */
        P->CTCount = 0u;                        /* 휴지 카운터 초기화 */
    }
    else                                        /* |전류| < 2.5A : 휴지 */
    {
        P->CTCount++;                           /* 휴지 시간 누적 (50ms 단위) */
        if(P->CTCount > 6000u)                  /* 6000 x 50ms = 300초 = 5분 */
        {
            P->CTCount = 6001u;                 /* 카운터 포화 (오버플로 방지) */
            P->SoCStateRegs.bit.CalMeth = 0u;   /* 휴지 5분 이상 -> OCV 재초기화 */
          //P->SoCStateRegs.bit.CalMeth = 1u;   /* (구) 적산 유지 - 비활성 */
        }
    }

    /* CalMeth 전환 감지 시 Ah 누적 변수 리셋 (새 기준점 확보) */
    {
        static Uint16 prevCalMeth_u16 = 0u;     /* 함수 진입 간 상태 보존 */
        Uint16 curCalMeth_u16 = (Uint16)P->SoCStateRegs.bit.CalMeth;
        if(curCalMeth_u16 != prevCalMeth_u16)
        {
            /* 모드 전환 순간 Ah/SOC 버퍼 초기화 */
            P->SysPackAhNewF    = 0.0F;
            P->SysPackAhF       = 0.0F;
            P->SysPackAhOldF    = 0.0F;
            P->SysPackSOCBufF1  = 0.0F;
            P->SysPackSOCBufF2  = 0.0F;
            P->state = SOC_STATE_InitRegs;
        }
        prevCalMeth_u16 = curCalMeth_u16;
    }

    /* ---- SOC 산출 ---- */
    if(P->SoCStateRegs.bit.CalMeth == 0u)       /* OCV 재초기화 모드 (휴지) */
    {
        //P->AVGXF = P->CellAgvVoltageF;         /* 중복 - 함수 시작에서 이미 설정 */

        /* OCV 룩업 테이블로 초기 SOC 산출 */
        CalEVE240AhSocInit(P);
        P->state = SOC_STATE_InitSos;
        /* 산출된 OCV SOC를 팩 SOC에 반영 */
        P->SysPackSOCF = P->SysSocInitF;
    }
    else                                         /* CalMeth == 1 : 전류적산 모드 */
    {
        /* 적산 시간 : 0.05s x (1/3600) [시간 h 단위] */
        P->SysSOCdtF = (C_CTSampleTime * C_SocCumulativeTime);
        /* 전류[A] x 시간[h] = 전하량[Ah] (부호로 충/방전 구분) */
        P->SysPackAhNewF = (P->SysSoCCTF * P->SysSOCdtF);
        P->SysPackAhF    = (P->SysPackAhNewF + P->SysPackAhOldF);   /* 누적 */
        P->SysPackAhOldF = P->SysPackAhF;

        /* Ah 누적 클램프 (Pack 460Ah x DoD 80% = 368Ah 기준) */
        if(P->SysPackAhF <= -368.0F)
        {
            P->SysPackAhF = -368.0F;
        }
        else if(P->SysPackAhF >= 368.0F)
        {
            P->SysPackAhF = 368.0F;
        }

        /* Ah -> SOC[%] 변환 : (Ah / 368) x 100 */
        P->SysPackSOCBufF1 = (P->SysPackAhF * C_EVE368AhNorm);  /* (Ah)*(1/368) */
        P->SysPackSOCBufF2 = (P->SysPackSOCBufF1 * 100.0F);     /* % 환산 */
        /* 최종 SOC = 초기 SOC(OCV 기준점) + 적산 변화량 */
        P->SysPackSOCF     = (P->SysSocInitF + P->SysPackSOCBufF2);
        P->state = SOC_STATE_CalAhSos;
    }

    /* SOC 0~100% 클램프 */
    if(P->SysPackSOCF < 0)
    {
        P->SysPackSOCF = 0;
    }
    else if(P->SysPackSOCF > 100)
    {
        P->SysPackSOCF = 100;
    }
}


#endif
