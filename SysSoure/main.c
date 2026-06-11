

/**
 * main.c
 */

#include "DSP28x_Project.h"
#include "parameter.h"
#include "SysVariable.h"
#include "ProtectRelay.h"
#include "BAT_LTC6802.h"
#include "BATAlgorithm.h"
#include "NVRAM.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

/* SOC init : NVR LastSOC trusted range (else reseed from OCV table) */
#define NVR_SOC_VALID_LO    22.0F      /* % : below -> OCV (sharp low end)  */
#define NVR_SOC_VALID_HI    92.0F      /* % : at or above -> OCV (sharp high end) */

/*
 *
 */
void InitGpio(void);

void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);
void PWRHoldHandle(SystemReg *P);
/*
 *
 */
void InitECanaGpio(void);
void InitECana(void);
void CANATX(unsigned int ID, unsigned char Length, unsigned int Data0, unsigned int Data1,unsigned int Data2,unsigned int Data3);

/*
 *
 */
void SysTimerINIT(SystemReg *s);
void SysVarINIT(SystemReg *s);
void CANRegVarINIT(CANAReg *P);
void SysDigitalInput(SystemReg *sys);
void SysDigitalOutput(SystemReg *sys);
void MDCalInit(SystemReg *P);
/*
 *
 */

//void ProtectRlySateCheck(PrtectRelayReg *P);
//void ProtectRlyVarINIT(PrtectRelayReg *P);
//void ProtectRlyOnInit(PrtectRelayReg *P);
//void ProtectRlyOnHandle(PrtectRelayReg *P);
//void ProtectRlyOffInit(PrtectRelayReg *P);
//void ProtectRlyOffHandle(PrtectRelayReg *P);
//void ProtectRlyEMSHandle(PrtectRelayReg *P);

void RlySeqHandle(PrtectRelayReg *P);
void ProtectRlyVarINIT(PrtectRelayReg *P);
void ProtectRlyEMSHandle(PrtectRelayReg *P);

void PWRRlyHoldHandle(SystemReg *p);
void PWRHoldHandle(SystemReg *P);
/*
 *
 */
void SysCalVoltageHandle(SystemReg *s);
void SysCalCurrentHandle(SystemReg *s);
void SysCalTemperatureHandle(SystemReg *s);
void SysCalSocZoneHandle(SystemReg *s);
void MDCalVoltandTemsHandle(SystemReg *P);

//void CalFarasis52AhRegsInit(SocReg *P);
//void CalFarasis52AhSocInit(SocReg *P);
//void CalFarasis52AhSocHandle(SocReg *P);

//extern void CalFrey60AhRegsInit(SocReg *P);
//extern void CalFrey60AhSocInit(SocReg *P);
//extern void CalFrey60AhSocHandle(SocReg *P);
void CalEVE240AhRegsInit(SocReg *P);
void CalEVE240AhSocInit(SocReg *P);
void CalEVE240AhSocHandle(SocReg *P);

void SysFaultCheck(SystemReg *s);
void SysAlarmtCheck(SystemReg *s);
void SysProtectCheck(SystemReg *s);

/*
 *
 */

//void CalFrey60AhRegsInit(SocReg *P);
//void CalFrey60AhSocInit(SocReg *P);
//void CalFrey60AhSocHandle(SocReg *P);
void NVRAM_AZoneSaveHandler(NVRZoneAReg *p);
void NVRAM_AZoneReadHandler(NVRZoneAReg *p);
void NVRAM_RecognizeAndInit(NVRZoneAReg *p);
void NVRAM_StateTest(void);

/*
 *
 */
int float32ToInt(float32 Vaule, Uint32 Num);
/*
 *
 */

/*
 *
 */
void SPI_Write(unsigned int WRData);
unsigned int SPI_Read(void);
//void BAT_InitSPI(void);
void SPI_BATWrite(unsigned int WRData);
/*
 *
 */
int LTC6804_read_cmd(char address, short command, char data[], int len);
int LTC6804_write_cmd(char address, short command, char data[], int len);
void init_PEC15_Table(void);
unsigned short pec15(char *data, int len);
int SlaveBMSIint(SlaveReg *s);
int SlaveBmsBalance(SlaveReg *s);
void SlaveVoltagHandler(SlaveReg *s);
void SlaveVoltagBalaHandler(SlaveReg *s);
void SlaveBMSDigiteldoutOHandler(SlaveReg *P);
void SalveTempsHandler(SlaveReg *s);
void TempTemps(SystemReg *s);

/*
 *  인터럽트 함수 선언
 */
interrupt void cpu_timer0_isr(void);
interrupt void ISR_CANRXINTA(void);
//interrupt void cpu_timer2_isr(void);

SystemReg       SysRegs;
float32 randomCT=0;
float32 randomA=0;
float32 randomC=0;


PrtectRelayReg  PrtectRelayRegs;
SlaveReg        Slave0Regs;
SlaveReg        Slave1Regs;
SlaveReg        Slave2Regs;
SlaveReg        Slave3Regs;

NVRAllReg       NVRAllRegs;
NVRZoneAReg     NVRZoneAWRRegs;
NVRZoneAReg     NVRZoneARDRegs;
//NVRZoneAReg     NVRZoneAInitRegs;


CANAReg         CANARegs;
SocReg          EV240AhSocRegs;
float32         NCMsocTestVoltAGV =3.210;

float32         NUMsocTestVCT =0.0;
float32         LFPsocTestVoltAGV =3.050;
float32         LFPsocTestVCT =0.0;

float32         TesVoltAGV =3.050;
float32         TescutCt =0.0;
float32         TescutCtabs =0.0;



unsigned int    ProtectRelayCyle=0;
//extern unsigned int    CellVoltUnBalaneFaulCount=0;

void main(void)
{
//    struct ECAN_REGS ECanaShadow;
    InitSysCtrl();
    /*
     * To check the clock status of the C2000 in operation
     */
  //  GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3; //enable XCLOCKOUT through GPIO mux
  //  SysCtrlRegs.XCLK.bit.XCLKOUTDIV = 0; //XCLOCKOUT = 1/2* SYSCLK

// Step 2. Initalize GPIO:
// This example function is found in the DSP2803x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
// For this example use the following configuration:
// Step 3. Clear all interrupts and initialize PIE vector table:
    DINT;
// Initialize PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP2803x_PieCtrl.c file.
    InitPieCtrl();
// Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();
    EALLOW;  // This is needed to write to EALLOW protected registers

    /*
     *  인터럽트 함수 선언
     */
    PieVectTable.TINT0 = &cpu_timer0_isr;
    PieVectTable.ECAN0INTA  = &ISR_CANRXINTA;
//    PieVectTable.TINT2 = &cpu_timer2_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers
    InitGpio();
    //GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 3; //enable XCLOCKOUT through GPIO mux
    //SysCtrlRegs.XCLK.bit.XCLKOUTDIV = 2; //XCLOCKOUT = SYSCLK
    InitSpiGpio();
    InitSpi();
    InitECanGpio();
    InitECan();

    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
    InitFlash();

    ConfigCpuTimer(&CpuTimer0, 80, 1000);
    //CpuTimer0Regs.PRD.all = 80000;// 90000 is 1msec
    CpuTimer0Regs.PRD.all = 80400;// 90000 is 1msec
    //   ConfigCpuTimer(&CpuTimer1, 80, 1000000);
    //   ConfigCpuTimer(&CpuTimer2, 80, 1000000);
    CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
    //  CpuTimer1Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
    //  CpuTimer2Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
//    InitAdc();
//    AdcOffsetSelfCal();
    EALLOW;
    EDIS;    // This is needed to disable write to EALLOW protected registers
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;
    IER |= M_INT9;//test
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;      // Enable TINT0 in the PIE: Group 1 interrupt 7
    PieCtrlRegs.PIEIER9.bit.INTx5 = 1;      // Enable ECAN-A interrupt of PIE group 9
//  PieCtrlRegs.PIEIER9.bit.INTx1 = 1;      // SCIA RX interrupt of PIE group
    EINT;   // Enable Global interrupt INTM
    ERTM;   // Enable Global realtime interrupt DBGM
    PrtectRelayRegs.RlyMachine= PrtctRly_INIT;
    SysRegs.SysMachine=INIT;
    CANARegs.PMSCMDRegs.all=0x0000;
    CANARegs.HMICMDRegs.all=0x0000;
    SysRegs.SysStateReg.bit.SysDisCharMode=1;
    CANARegs.ChargerResgsStauts.bit.Char_DOSTatue=0;
    while(1)
    {
        SysRegs.Maincount++;
        switch(SysRegs.SysMachine)
        {
            // TODO: [완료] SysRegs.SysMachine의 INIT 루틴
            case INIT: //0
                 CANARegs.DiviceState=0;
                 SysTimerINIT(&SysRegs);
                 SysVarINIT(&SysRegs);
                 CANRegVarINIT(&CANARegs);
                 MDCalInit(&SysRegs);
                 ProtectRlyVarINIT(&PrtectRelayRegs);
                 CalEVE240AhRegsInit(&EV240AhSocRegs);
                 EV240AhSocRegs.state=SOC_STATE_INIT;
                 SysRegs.SysStateReg.Word.DataH=0;
                 SysRegs.SysStateReg.Word.DataL=0;
                 SysRegs.SysAlarmReg.Word.DataH=0;
                 SysRegs.SysAlarmReg.Word.DataL=0;
                 SysRegs.SysProtectReg.Word.DataH=0;
                 SysRegs.SysProtectReg.Word.DataL=0;
                 Slave0Regs.ID=BMS_ID_0;
                 Slave0Regs.SlaveCh=C_Slave_ACh;
                 SlaveBMSIint(&Slave0Regs);

                 Slave1Regs.ID=BMS_ID_1;
                 Slave1Regs.SlaveCh=C_Slave_ACh;
                 SlaveBMSIint(&Slave1Regs);

                 Slave2Regs.ID=BMS_ID_2;
                 Slave2Regs.SlaveCh=C_Slave_ACh;
                 SlaveBMSIint(&Slave2Regs);

                 Slave3Regs.ID=BMS_ID_3;
                 Slave3Regs.SlaveCh=C_Slave_ACh;
                 SlaveBMSIint(&Slave3Regs);
                 SysRegs.SysMachine=STANDBY;
                 NVRAM_AZoneReadHandler(&NVRZoneARDRegs);
                 NVRAllRegs.SysTimeTick=NVRZoneARDRegs.SysTimeTick;
                 EV240AhSocRegs.state=SOC_STATE_SOSINIT;
                // SysRegs.SysStateReg.bit.BSACHAEnable=1;
                 if(SysRegs.SysStateReg.bit.AdminMode==1)  { SysRegs.SysMachine=MANUALMode;}
            break;
            // TODO: [완료] SysRegs.SysMachine의 STANDBY 루틴
            case STANDBY://1
                     CANARegs.DiviceState=1;
                     SysRegs.CanComEable=1;
                     SysRegs.SysStateReg.bit.CANCOMEnable=1;
                     if(SysRegs.SysStateReg.bit.INITOK==0)
                     {  
                         //TODO: [완료] 셀 전압 읽고, 셀 전압 상태 연산
                         for(SysRegs.InitValuleCnt=0;SysRegs.InitValuleCnt<3;SysRegs.InitValuleCnt++)
                         {
                             Slave0Regs.ID=BMS_ID_0;
                             Slave0Regs.SlaveCh=C_Slave_ACh;
                             Slave0Regs.Balance.all = 0x0000;
                             SlaveBmsBalance(&Slave0Regs);
                             Slave0Regs.StateMachine = STATE_BATREAD;
                             Slave0Regs.ID=BMS_ID_0;
                             Slave0Regs.SlaveCh=C_Slave_ACh;
                             SlaveVoltagHandler(&Slave0Regs);
                            
                             Slave1Regs.ID=BMS_ID_1;
                             Slave1Regs.SlaveCh=C_Slave_ACh;
                             Slave1Regs.Balance.all = 0x0000;
                             SlaveBmsBalance(&Slave1Regs);
                             Slave1Regs.StateMachine = STATE_BATREAD;
                             Slave1Regs.ID=BMS_ID_1;
                             Slave1Regs.SlaveCh=C_Slave_ACh;
                             SlaveVoltagHandler(&Slave1Regs);
                             
                             Slave2Regs.ID=BMS_ID_2;
                             Slave2Regs.SlaveCh=C_Slave_ACh;
                             Slave2Regs.Balance.all = 0x0000;
                             SlaveBmsBalance(&Slave2Regs);
                             Slave2Regs.StateMachine = STATE_BATREAD;
                             Slave2Regs.ID=BMS_ID_2;
                             Slave2Regs.SlaveCh=C_Slave_ACh;
                             SlaveVoltagHandler(&Slave2Regs);
                             delay_ms(10);
                             Slave3Regs.ID=BMS_ID_3;
                             Slave3Regs.SlaveCh=C_Slave_ACh;
                             Slave3Regs.Balance.all = 0x0000;
                             SlaveBmsBalance(&Slave3Regs);
                             Slave3Regs.StateMachine = STATE_BATREAD;
                             Slave3Regs.ID=BMS_ID_3;
                             Slave3Regs.SlaveCh=C_Slave_ACh;
                             SlaveVoltagHandler(&Slave3Regs);
                         }
                         memcpy(&SysRegs.SysCellVoltageF[0],        &Slave0Regs.CellVoltageF[0],sizeof(float32)*7);
                         memcpy(&SysRegs.SysCellVoltageF[7],        &Slave1Regs.CellVoltageF[0],sizeof(float32)*8);
                         memcpy(&SysRegs.SysCellVoltageF[15],       &Slave2Regs.CellVoltageF[0],sizeof(float32)*7);
                         memcpy(&SysRegs.SysCellVoltageF[22],       &Slave3Regs.CellVoltageF[0],sizeof(float32)*8);
                        SysCalVoltageHandle(&SysRegs);
                        //TODO: [완료] 셀 온도 읽고, 셀 온도 상태 연산
                        for(SysRegs.InitValuleCnt=0;SysRegs.InitValuleCnt<48;SysRegs.InitValuleCnt++)
                        {
                            Slave0Regs.ID=BMS_ID_0;
                            Slave0Regs.SlaveCh=C_Slave_ACh;
                            Slave0Regs.BATICDO.bit.GPIO1=1;
                            SlaveBMSDigiteldoutOHandler(&Slave0Regs);
                            SalveTempsHandler(&Slave0Regs);
                            //delay_ms(5);
                            Slave1Regs.ID=BMS_ID_1;
                            Slave1Regs.SlaveCh=C_Slave_ACh;
                            Slave1Regs.BATICDO.bit.GPIO1=1;
                            SlaveBMSDigiteldoutOHandler(&Slave1Regs);
                            SalveTempsHandler(&Slave1Regs);
                            //delay_ms(5);
                            Slave2Regs.ID=BMS_ID_2;
                            Slave2Regs.SlaveCh=C_Slave_ACh;
                            Slave2Regs.BATICDO.bit.GPIO1=1;
                            SlaveBMSDigiteldoutOHandler(&Slave2Regs);
                            SalveTempsHandler(&Slave2Regs);
                            //delay_ms(5);
                            Slave3Regs.ID=BMS_ID_3;
                            Slave3Regs.SlaveCh=C_Slave_ACh;
                            Slave3Regs.BATICDO.bit.GPIO1=1;
                            SlaveBMSDigiteldoutOHandler(&Slave3Regs);
                            SalveTempsHandler(&Slave3Regs);
                            //delay_ms(5);
                        }
                        memcpy(&SysRegs.SysCelltemperatureF[0],     &Slave0Regs.CellTemperatureF[0],sizeof(float32)*7);
                        memcpy(&SysRegs.SysCelltemperatureF[7],     &Slave1Regs.CellTemperatureF[0],sizeof(float32)*8);
                        memcpy(&SysRegs.SysCelltemperatureF[15],    &Slave2Regs.CellTemperatureF[0],sizeof(float32)*7);
                        memcpy(&SysRegs.SysCelltemperatureF[22],    &Slave3Regs.CellTemperatureF[0],sizeof(float32)*8);
                        SysCalTemperatureHandle(&SysRegs);
                        /*
                        * soc init 초기화하는 부분
                        */
                        /*------------------------------------------------------------
                         * 비고 : 아래 zone 분기 블록은 폐지 (참고용 주석 보존).
                         *        무조건 NVR에서 SOC 재초기화하도록 변경 (아래 새 코드 사용).
                         *------------------------------------------------------------*/
                        //EV240AhSocRegs.state=SOC_STATE_ZONE;
                        //SysCalSocZoneHandle(&SysRegs);
                        //if(SysRegs.SysSocInitRule == SOC_ZONE_cellVolt)
                        //{
                        //    EV240AhSocRegs.CellAgvVoltageF=SysRegs.SysCellAgvVoltageF;
                        //    CalEVE240AhSocInit(&EV240AhSocRegs);
                        //    EV240AhSocRegs.state=SOC_STATE_SOSINIT;
                        //    SysRegs.SysStateReg.bit.SysSocZone =0;
                        //}
                        //else if(SysRegs.SysSocInitRule == SOC_ZONE_NVR)
                        //{
                        //    //EV240AhSocRegs.state=SOC_STATE_SOSINIT;  /* moved : set state after seed is ready */
                        //    NVRAM_AZoneReadHandler(&NVRZoneARDRegs);
                        //    EV240AhSocRegs.SysSocInitF = (float32)(NVRZoneARDRegs.LastSOC/10.0f);
                        //    EV240AhSocRegs.state=SOC_STATE_SOSINIT;
                        //    SysRegs.SysStateReg.bit.SysSocZone =1;
                        //}
                        //SysRegs.SysStateReg.bit.SysSocMode = EV240AhSocRegs.SoCStateRegs.bit.CalMeth;

                        /* SOC init :
                           1) read NVR LastSOC
                           2) if NVR_SOC_VALID_LO <= NVR < NVR_SOC_VALID_HI : trust NVR
                           3) else (NVR out of range) :
                              - if cell V in flat plateau : keep NVR (OCV inaccurate, avoid edge jump)
                              - else (cell V at sharp ends) : reseed from OCV table 
                        */
                        EV240AhSocRegs.state = SOC_STATE_ZONE;
                        NVRAM_AZoneReadHandler(&NVRZoneARDRegs);
                        EV240AhSocRegs.SysSocInitF = (float32)(NVRZoneARDRegs.LastSOC/10.0f);   // TODO : [완료] 260610_Note1, 1.0 부팅 시 NVR LastSOC를 SOC 초기값으로 사용
                        /* NVR sanity : clamp wrap or corruption to safe 0~100% range */
                        if(EV240AhSocRegs.SysSocInitF <   0.0F) { EV240AhSocRegs.SysSocInitF =   0.0F; }
                        if(EV240AhSocRegs.SysSocInitF > 100.0F) { EV240AhSocRegs.SysSocInitF = 100.0F; }
                        if((EV240AhSocRegs.SysSocInitF >= NVR_SOC_VALID_LO)
                        && (EV240AhSocRegs.SysSocInitF <  NVR_SOC_VALID_HI))
                        {
                            /* NVR within trusted range : trust NVR */
                            SysRegs.SysSocInitRule = SOC_ZONE_NVR;
                            SysRegs.SysStateReg.bit.SysSocZone = 1;
                        }
                        else
                        {
                            /* NVR out of trusted range : check cell V zone before falling back to OCV */
                            if((SysRegs.SysCellAgvVoltageF >= V_FlatStartF)
                            && (SysRegs.SysCellAgvVoltageF <= V_FlatEndF))
                            {
                                /* cell V in flat plateau : OCV is inaccurate here, keep NVR as seed
                                   to avoid sudden SOC jumps near edges (user confusion guard) */
                                SysRegs.SysSocInitRule = SOC_ZONE_NVR;
                                SysRegs.SysStateReg.bit.SysSocZone = 1;
                            }
                            else
                            {
                                /* cell V at sharp ends : reseed from OCV table */
                                EV240AhSocRegs.CellAgvVoltageF = SysRegs.SysCellAgvVoltageF;
                                CalEVE240AhSocInit(&EV240AhSocRegs);
                                SysRegs.SysSocInitRule = SOC_ZONE_cellVolt;
                                SysRegs.SysStateReg.bit.SysSocZone = 0;
                            }
                        }
                        EV240AhSocRegs.state = SOC_STATE_SOSINIT;
                        SysRegs.SysStateReg.bit.SysSocMode = EV240AhSocRegs.SoCStateRegs.bit.CalMeth;
                     }
                     NVRAllRegs.SEQ=NVRAM_AZoneSave;
                     SysRegs.SysMachine=READY;
                     SysRegs.SysStateReg.bit.INITOK=1;
                     if(SysRegs.SysStateReg.bit.AdminMode==1)  { SysRegs.SysMachine=MANUALMode;}
            break;
            case READY://2
                  // TODO: [완료] SysRegs.SysMachine의 READY 루틴
                   PrtectRelayRegs.State.bit.WakeUpEN= 1;
                   SysRegs.SysStateReg.bit.SysDisCharMode     =! CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
                   if(SysRegs.SysStateReg.bit.SysDisCharMode==1)
                   {
                      CANARegs.DiviceState=3;
                   }
                   else
                   {
                      CANARegs.DiviceState=2;
                   }
                   SysRegs.SysDigitalOutPutReg.bit.PWRHOLD=1; //SysRegs.SysStateReg.bit.VCUWakeUpIn;
                   SysRegs.SysStateReg.bit.BSACHAEnable=1;
                   if(SysRegs.SysStateReg.bit.SysPrtct==1)
                   {
                       //PrtectRelayRegs.State.bit.WakeUpEN= 0;
                   }
                   SysRegs.SysMachine=RUNING;
                   if(SysRegs.SysStateReg.bit.AdminMode==1)  { SysRegs.SysMachine=MANUALMode;}
            break;
            case RUNING:
                    // TODO: [완료] SysRegs.SysMachine의 RUNING 루틴
                     SysRegs.SysStateReg.bit.SysDisCharMode     =! CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
                     SysRegs.SysStateReg.bit.killSW             =  SysRegs.SysDigitalInputReg.bit.killSW;
                     SysRegs.SysStateReg.bit.ChargerWakeUpIn    =  CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
                     SysRegs.SysStateReg.bit.VCUWakeUpIn        =  CANARegs.PMSCMDRegs.bit.RlyOFF;
                     SysRegs.SysStateReg.bit.VCUComStatus       =  SysRegs.SysStateReg.bit.VCUComStatus;
                     SysRegs.SysStateReg.bit.CHAComStatus       =  SysRegs.SysStateReg.bit.CHAComStatus;
                     // TODO: [튜닝] PWRRlyHoldHandle 루틴
                     PWRRlyHoldHandle(&SysRegs);
                     PrtectRelayRegs.State.bit.WakeUpEN         = SysRegs.SysStateReg.bit.WakeUpOut;
                     SysRegs.SysDigitalOutPutReg.bit.PWRHOLD    = SysRegs.SysStateReg.bit.PwrHoldState;
                     CANARegs.ChargerStateRegs.bit.BSACHAEnable = SysRegs.SysStateReg.bit.BSACHAEnable;
                     if(SysRegs.SysStateReg.bit.SysDisCharMode==1)
                     {
                        CANARegs.DiviceState=3;
                     }
                     else
                     {
                        CANARegs.DiviceState=2;
                     }
                     if(SysRegs.SysStateReg.bit.SysPrtct==1)
                     {
                         // SysRegs.SysMachine=PROTECTER;
                     }
                     if(SysRegs.SysStateReg.bit.AdminMode==1)  { SysRegs.SysMachine=MANUALMode;}

            break;
            case PROTECTER://5
            // TODO: [완료] SysRegs.SysMachine의 PROTECTER 루틴
                    if(SysRegs.SysStateReg.bit.AdminMode==1) { SysRegs.SysMachine=MANUALMode;}

            case MANUALMode://6
            // TODO: [완료] SysRegs.SysMachine의 MANUALMode 루틴
           //     SysRegs.SysDigitalOutPutReg.bit.NRlyOUT    = CANARegs.HMICMDRegs.bit.N_Rly;
           //     SysRegs.SysDigitalOutPutReg.bit.PRlyOUT    = CANARegs.HMICMDRegs.bit.P_Rly;
           //     SysRegs.SysDigitalOutPutReg.bit.ProRlyOUT  = CANARegs.HMICMDRegs.bit.Pre_Rly;
           //     SysRegs.SysDigitalOutPutReg.bit.PWRHOLD    = CANARegs.HMICMDRegs.bit.PWRHoldRly;
                 if((CANARegs.HMICMDRegs.bit.HMI_MODE==1)&&(CANARegs.HMICMDRegs.bit.HMI_Reset==1))
                 {
                       CANARegs.HMICMDRegs.bit.HMI_Reset=0;
                       CANARegs.PMSCMDRegs.bit.PrtctReset=0;
                       if(SysRegs.SysStateReg.bit.SysPrtct==0)
                       {
                          SysRegs.SysMachine=STANDBY;
                       }
                  }
            break;
            default :
            break;
        }
        if(SysRegs.CellVoltsampling>=CellVoltSampleTime)
        {
            // TODO: [완료] Balance 진입 조건
            if((SysRegs.SysPackCurrentAsbF <= 2.0f) && (SysRegs.SysCellMinVoltageF > 2.8f))
            {
                if(SysRegs.BalanceModeCount < 101u)
                {
                    SysRegs.BalanceModeCount++;
                }
                if(SysRegs.BalanceModeCount>=100)
                {
                    SysRegs.BalanceModeCount=101;
                    SysRegs.SysStateReg.bit.SysBalaMode=1;

                }
            }
            else
            {
                SysRegs.SysStateReg.bit.SysBalaMode=0;
                SysRegs.SysStateReg.bit.SysBalanceEn=0;
                SysRegs.BalanceModeCount=0;
                SysRegs.BalanceTimeCount=0;
            }
           // SysRegs.SysStateReg.bit.SysBalaMode=0;
           // SysRegs.SysStateReg.bit.SysBalanceEn=0;
           // TODO: [완료] Balance 조건
            if(SysRegs.SysStateReg.bit.SysBalaMode==1)
            {
                SysRegs.BalanceTimeCount++;
                if(SysRegs.BalanceTimeCount>50)
                {
                   SysRegs.SysStateReg.bit.SysBalanceEn = !  SysRegs.SysStateReg.bit.SysBalanceEn;
                   SysRegs.BalanceTimeCount=0;
                }
            }
            else
            {
                SysRegs.BalanceTimeCount =0;
                SysRegs.SysStateReg.bit.SysBalanceEn=0;
            }
            // 셀 전압 Balance
            if(SysRegs.SysStateReg.bit.SysBalanceEn==1)
            {
                if(SysRegs.SysStateReg.bit.AdminMode==0)
                {
                    SysRegs.HMICANErrCheck=0;
                    CANARegs.HMICMDRegs.all=0;
                    CANARegs.HMICEllTempsAgv=250;
                    CANARegs.HMICEllVoltMin=4200;
                    SysRegs.BalanceRefVoltageF = SysRegs.SysCellMinVoltageF;
                }
                else
                {
                    //SysRegs.HMICANErrCheck++; CPU 인터럽트 1msec
                    if(SysRegs.HMICANErrCheck < 3001)
                    {
                        if(SysRegs.SysStateReg.bit.AdminMode == 1u)
                        {
                            CANARegs.HMICEllTempsAgv=250;
                            SysRegs.BalanceRefVoltageF = (float32)(CANARegs.HMICEllVoltMin*0.001);
                        }
                        else
                        {
                            SysRegs.BalanceRefVoltageF = SysRegs.SysCellMinVoltageF;
                        }
                    }
                    else
                    {
                        SysRegs.HMICANErrCheck=0;
                        CANARegs.HMICMDRegs.all=0;
                        CANARegs.HMICEllVoltMin=4200;
                        SysRegs.BalanceRefVoltageF = SysRegs.SysCellMinVoltageF;
                    }
                }
                // TODO: [완료] Balance 핸들러 함수 호출
                Slave0Regs.ID=BMS_ID_0;
                Slave0Regs.SlaveCh=C_Slave_ACh;
                Slave0Regs.SysCellMinVoltage = SysRegs.BalanceRefVoltageF;
                SlaveVoltagBalaHandler(&Slave0Regs);
                Slave0Regs.Balance.bit.B_Cell07=0;
                Slave0Regs.Balance.bit.B_Cell08=0;
                Slave0Regs.Balance.bit.B_Cell09=0;
                Slave0Regs.Balance.bit.B_Cell10=0;
                Slave0Regs.Balance.bit.B_Cell11=0;
                SlaveBmsBalance(&Slave0Regs);
                SysRegs.SlaveVoltErrCount[0]=Slave0Regs.ErrorCount;

                Slave1Regs.ID=BMS_ID_1;
                Slave1Regs.SlaveCh=C_Slave_ACh;
                Slave1Regs.SysCellMinVoltage = SysRegs.BalanceRefVoltageF;
                SlaveVoltagBalaHandler(&Slave1Regs);
                Slave1Regs.Balance.bit.B_Cell07=0;
                Slave1Regs.Balance.bit.B_Cell08=0;
                Slave1Regs.Balance.bit.B_Cell09=0;
                Slave1Regs.Balance.bit.B_Cell10=0;
                Slave1Regs.Balance.bit.B_Cell11=0;
                SlaveBmsBalance(&Slave1Regs);
                SysRegs.SlaveVoltErrCount[1]=Slave1Regs.ErrorCount;

                Slave2Regs.ID=BMS_ID_2;
                Slave2Regs.SlaveCh=C_Slave_ACh;
                Slave2Regs.SysCellMinVoltage = SysRegs.BalanceRefVoltageF;
                SlaveVoltagBalaHandler(&Slave2Regs);
                Slave2Regs.Balance.bit.B_Cell07=0;
                Slave2Regs.Balance.bit.B_Cell08=0;
                Slave2Regs.Balance.bit.B_Cell09=0;
                Slave2Regs.Balance.bit.B_Cell10=0;
                Slave2Regs.Balance.bit.B_Cell11=0;
                SlaveBmsBalance(&Slave2Regs);
                SysRegs.SlaveVoltErrCount[2]=Slave2Regs.ErrorCount;

                Slave3Regs.ID=BMS_ID_3;
                Slave3Regs.SlaveCh=C_Slave_ACh;
                Slave3Regs.SysCellMinVoltage = SysRegs.BalanceRefVoltageF;
                SlaveVoltagBalaHandler(&Slave3Regs);
                Slave3Regs.Balance.bit.B_Cell07=0;
                Slave3Regs.Balance.bit.B_Cell08=0;
                Slave3Regs.Balance.bit.B_Cell09=0;
                Slave3Regs.Balance.bit.B_Cell10=0;
                Slave3Regs.Balance.bit.B_Cell11=0;
                SlaveBmsBalance(&Slave3Regs);
                SysRegs.SlaveVoltErrCount[3]=Slave3Regs.ErrorCount;
            }
            // TODO: [완료] Balance 강제로 disable 후에 셀 전압 읽기
            if(SysRegs.SysStateReg.bit.SysBalanceEn==0)
            {
                if(SysRegs.SysStateReg.bit.CellVoltOk==0)
                {
                    if(SysRegs.SlaveReadVoltEn.bit.SlaveBMS00==1)
                    {
                        LEDSysState_H;
                        Slave0Regs.ID=BMS_ID_0;
                        Slave0Regs.SlaveCh=C_Slave_ACh;

                        Slave0Regs.Balance.all = 0x0000;
                        SlaveBmsBalance(&Slave0Regs);
                        SysRegs.SlaveBalanErrCount[0]=Slave0Regs.ErrorCount;
                        if(SysRegs.SlaveBalanErrCount[0]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;
                            SysRegs.SlaveBalanErrCount[0]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00 =0;
                        }
                        Slave0Regs.StateMachine = STATE_BATREAD;
                        SlaveVoltagHandler(&Slave0Regs);
                        
                        SysRegs.SlaveVoltErrCount[0]=Slave0Regs.ErrorCount;
                        if(SysRegs.SlaveVoltErrCount[0]>C_ISOSPIPrtectCont)
                        {
                             SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;
                             SysRegs.SlaveVoltErrCount[0]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                             SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00 =0;
                        }
                        LEDSysState_L;
                    }
                    if(SysRegs.SlaveReadVoltEn.bit.SlaveBMS01==1)
                    {
                        Slave1Regs.ID=BMS_ID_1;
                        Slave1Regs.SlaveCh=C_Slave_ACh;
                        Slave1Regs.Balance.all = 0x0000;
                        SlaveBmsBalance(&Slave1Regs);
                        SysRegs.SlaveBalanErrCount[1]=Slave1Regs.ErrorCount;
                        if(SysRegs.SlaveBalanErrCount[1]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;
                            SysRegs.SlaveBalanErrCount[1]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00 =0;
                        }

                        Slave1Regs.StateMachine = STATE_BATREAD;
                        SlaveVoltagHandler(&Slave1Regs);
                        SysRegs.SlaveVoltErrCount[1]=Slave1Regs.ErrorCount;
                        if(SysRegs.SlaveVoltErrCount[1]>C_ISOSPIPrtectCont)
                        {
                             SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=1;
                             SysRegs.SlaveVoltErrCount[1]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                             SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01 =0;
                        }
                    }
                    if(SysRegs.SlaveReadVoltEn.bit.SlaveBMS02==1)
                    {
                        Slave2Regs.ID=BMS_ID_2;
                        Slave2Regs.SlaveCh=C_Slave_ACh;

                        Slave2Regs.Balance.all = 0x0000;
                        SlaveBmsBalance(&Slave2Regs);
                        SysRegs.SlaveBalanErrCount[2]=Slave2Regs.ErrorCount;
                        if(SysRegs.SlaveBalanErrCount[2]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=1;
                            SysRegs.SlaveBalanErrCount[2]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01 =0;
                        }

                        Slave2Regs.StateMachine = STATE_BATREAD;
                        SlaveVoltagHandler(&Slave2Regs);
                        SysRegs.SlaveVoltErrCount[2]=Slave2Regs.ErrorCount;
                        if(SysRegs.SlaveVoltErrCount[2]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=1;
                            SysRegs.SlaveVoltErrCount[2]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=0;
                        }
                    }
                    if(SysRegs.SlaveReadVoltEn.bit.SlaveBMS03==1)
                    {
                        Slave3Regs.ID=BMS_ID_3;
                        Slave3Regs.SlaveCh=C_Slave_ACh;
                        Slave3Regs.Balance.all = 0x0000;
                        SlaveBmsBalance(&Slave3Regs);
                        SysRegs.SlaveVoltErrCount[3]=Slave3Regs.ErrorCount;
                        if(SysRegs.SlaveVoltErrCount[3]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=1;
                            SysRegs.SlaveVoltErrCount[3]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03 =0;
                        }
                        Slave3Regs.StateMachine = STATE_BATREAD;
                        Slave3Regs.ID=BMS_ID_3;
                        Slave3Regs.SlaveCh=C_Slave_ACh;
                        SlaveVoltagHandler(&Slave3Regs);
                        SysRegs.SlaveVoltErrCount[3]=Slave3Regs.ErrorCount;
                        if(SysRegs.SlaveVoltErrCount[3]>C_ISOSPIPrtectCont)
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=1;
                            SysRegs.SlaveVoltErrCount[3]=C_ISOSPIPrtectCont+10;
                        }
                        else
                        {
                            SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=0;
                        }
                    }
            }

          }
          SysRegs.SysStateReg.bit.CellVoltOk=1;
          SysRegs.CellVoltsampling=0;
       }
        //TODO : [완료] 셀 온도 읽기 루틴
       if(SysRegs.CellTempssampling>CellTempSampleTime)
       {
           if(SysRegs.SysStateReg.bit.CellTempsOk==0)
           {
               if(SysRegs.SlaveReadTempsEn.bit.SlaveBMS00==1)
               {
                   Slave0Regs.ID=BMS_ID_0;
                   Slave0Regs.SlaveCh=C_Slave_ACh;

                   Slave0Regs.BATICDO.bit.GPIO1=1;
                   SlaveBMSDigiteldoutOHandler(&Slave0Regs);

                   SysRegs.SlaveTempsErrCount[0]=Slave0Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[0]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;
                       SysRegs.SlaveTempsErrCount[0]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=0;
                   }

                   SalveTempsHandler(&Slave0Regs);
                   if(SysRegs.SlaveTempsErrCount[0]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;
                       SysRegs.SlaveTempsErrCount[0]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=0;
                   }

               }
               if(SysRegs.SlaveReadTempsEn.bit.SlaveBMS01==1)
               {
                   Slave1Regs.ID=BMS_ID_1;
                   Slave1Regs.SlaveCh=C_Slave_ACh;

                   Slave1Regs.BATICDO.bit.GPIO1=1;
                   SlaveBMSDigiteldoutOHandler(&Slave1Regs);

                   SysRegs.SlaveTempsErrCount[1]=Slave0Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[1]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=1;
                       SysRegs.SlaveTempsErrCount[1]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=0;
                   }

                   SalveTempsHandler(&Slave1Regs);
                   SysRegs.SlaveTempsErrCount[1]=Slave0Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[1]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=1;
                       SysRegs.SlaveTempsErrCount[1]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=0;
                   }
               }
               if(SysRegs.SlaveReadTempsEn.bit.SlaveBMS02==1)
               {
                   Slave2Regs.ID=BMS_ID_2;
                   Slave2Regs.SlaveCh=C_Slave_ACh;

                   Slave2Regs.BATICDO.bit.GPIO1=1;
                   SlaveBMSDigiteldoutOHandler(&Slave2Regs);

                   SysRegs.SlaveTempsErrCount[2]=Slave2Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[2]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=1;
                       SysRegs.SlaveTempsErrCount[2]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=0;
                   }
                   SalveTempsHandler(&Slave2Regs);
                   SysRegs.SlaveTempsErrCount[2]=Slave2Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[2]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=1;
                       SysRegs.SlaveTempsErrCount[2]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=0;
                   }
               }
               if(SysRegs.SlaveReadTempsEn.bit.SlaveBMS03==1)
               {
                   Slave3Regs.ID=BMS_ID_3;
                   Slave3Regs.SlaveCh=C_Slave_ACh;

                   Slave3Regs.BATICDO.bit.GPIO1=1;
                   SlaveBMSDigiteldoutOHandler(&Slave3Regs);
                   SysRegs.SlaveTempsErrCount[3]=Slave3Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[3]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=1;
                       SysRegs.SlaveTempsErrCount[3]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=0;
                   }

                   SalveTempsHandler(&Slave3Regs);
                   SysRegs.SlaveTempsErrCount[3]=Slave3Regs.ErrorCount;
                   if(SysRegs.SlaveTempsErrCount[3]>C_ISOSPIPrtectCont)
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=1;
                       SysRegs.SlaveTempsErrCount[3]=C_ISOSPIPrtectCont+10;
                   }
                   else
                   {
                       SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=0;
                   }
               }

           }
           SysRegs.CellTempssampling=0;
           SysRegs.VoltTempsReadCount++;
           SysRegs.SysStateReg.bit.CellTempsOk=1;
           if(SysRegs.VoltTempsReadCount>=10)
           {
              SysRegs.VoltTempsReadCount=100;
             // SysRegs.SysStateReg.bit.CellInforRead=1;

           }
       }
       //TODO : [완료] 셀 전압과 온도 읽은 에러값에 따른 에러 플래그 설정
       if(Slave0Regs.ErrorCount>200) {SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS00=1;}
       if(Slave1Regs.ErrorCount>200) {SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS01=1;}
       if(Slave2Regs.ErrorCount>200) {SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS02=1;}
       if(Slave3Regs.ErrorCount>200) {SysRegs.SlaveISOSPIErrReg.bit.SlaveBMS03=1;}

       //TODO : [완료] 셀 전압과 온도 읽은 값을 SysRegs에 복사
       memcpy(&CANARegs.SysCellVoltage[0],        &Slave0Regs.CellVoltage[0],sizeof(Uint16)*7);
       memcpy(&CANARegs.SysCellVoltage[7],        &Slave1Regs.CellVoltage[0],sizeof(Uint16)*8);
       memcpy(&CANARegs.SysCellVoltage[15],       &Slave2Regs.CellVoltage[0],sizeof(Uint16)*7);
       memcpy(&CANARegs.SysCellVoltage[22],       &Slave3Regs.CellVoltage[0],sizeof(Uint16)*8);

       //TODO : [완료] 셀 온도 읽은 값을 SysRegs에 복사
       memcpy(&CANARegs.SysCelltemperature[0],    &Slave1Regs.CellTemperature[0],sizeof(int16)*7);
       memcpy(&CANARegs.SysCelltemperature[7],    &Slave1Regs.CellTemperature[0],sizeof(int16)*8);
       memcpy(&CANARegs.SysCelltemperature[15],   &Slave2Regs.CellTemperature[0],sizeof(int16)*7);
       memcpy(&CANARegs.SysCelltemperature[22],   &Slave3Regs.CellTemperature[0],sizeof(int16)*8);

      // NVRAM_StateTest();
       //TODOS : [완료] NVRAM에 주기적으로 쓰는 루틴
       if((g_SysTimeTick>100)&&(SysRegs.SysStateReg.bit.INITOK==1))
       {
           NVRAllRegs.SysTimeTick++;
           switch(NVRAllRegs.SEQ)
           {
               case NVRAM_AZoneSave :
                     //TODO : [검증] NVRAM에 쓰는 루틴에서 LastSOC가 0~1000 사이로 쓰이는지 검증
                    if((CANARegs.HMICMDRegs.bit.HMI_MODE==1) && (CANARegs.HMICMDRegs.bit.Admin_NVRSocInit==1u))
                    {
                        if(CANARegs.HMISocInitValue <= 1000u)
                        {
                            NVRZoneAWRRegs.LastSOC     = CANARegs.HMISocInitValue;
                            NVRAM_AZoneSaveHandler(&NVRZoneAWRRegs);
                            SysRegs.SysMachine=INIT;
                            SysRegs.SysStateReg.bit.INITOK=0;
                            CANARegs.HMICMDRegs.bit.Admin_NVRSocInit=0;
                        }
                    }
                    else 
                    {
                      NVRAllRegs.DebugCount++; //TODO: [완료] NVRAM Read 디버깅
                      NVRZoneAWRRegs.MetaVersion = Product_Version;
                      NVRZoneAWRRegs.SysTimeTick = NVRAllRegs.SysTimeTick;
                      NVRZoneAWRRegs.LastState   = SysRegs.SysStateReg.all;
                      NVRZoneAWRRegs.LastSOC     = (int16)(SysRegs.SysSOCF*10);
                      NVRAM_AZoneSaveHandler(&NVRZoneAWRRegs);
                      NVRAllRegs.SEQ=NVRAM_BZoneSave;
                      EV240AhSocRegs.state =SOC_STATE_SOSINIT;
                    }
               break;
               case NVRAM_BZoneSave :
                    //TODO: [완료] NVRAM에서 읽는 루틴
                    NVRAM_AZoneReadHandler(&NVRZoneARDRegs);
                    //TODO: [완료] NVRAM Read 디버깅
                    if(NVRAllRegs.DebugCount>200)
                    {
                        NVRAllRegs.DebugCount=0;
                    }
                    NVRAllRegs.SEQ=NVRAM_AZoneSave;
                    EV240AhSocRegs.state =SOC_STATE_SOSINIT;
               break;
               case NVRAM_CZoneSave :

               break;
               case NVRAM_AZoneRead :

               break;
               case NVRAM_BZoneRead :

               break;
               case NVRAM_CZoneRead :

               break;
               case NVRAM_MANUALMode :

               break;
               default :
               break;
           }
           g_SysTimeTick=0;
       }
       if(SysRegs.SysStateReg.bit.INITOK==1)
       {
          RlySeqHandle(&PrtectRelayRegs);
       }
      if(SysRegs.Maincount>3000){SysRegs.Maincount=0;}

    }
}

interrupt void cpu_timer0_isr(void)
{
   //LEDSysState_T;
   //TODO : [완료] 1msec마다 실행되는 인터럽트 루틴
   SysRegs.MainIsr1++;
   g_SysTimeTick++;
   //TODO : [완료] 타이머 카운트 증가 루틴
   SysRegs.SysRegTimer5msecCount++;
   SysRegs.SysRegTimer10msecCount++;
   SysRegs.SysRegTimer50msecCount++;
   SysRegs.SysRegTimer100msecCount++;
   SysRegs.SysRegTimer300msecCount++;
   SysRegs.SysRegTimer500msecCount++;
   SysRegs.SysRegTimer1000msecCount++;
   SysRegs.CellVoltsampling++;
   SysRegs.CellTempssampling++;
   if(SysRegs.HMICANErrCheck          <3000u)                {SysRegs.HMICANErrCheck++;}
   if(SysRegs.SysRegTimer5msecCount   >SysRegTimer5msec)     {SysRegs.SysRegTimer5msecCount=0;}
   if(SysRegs.SysRegTimer10msecCount  >SysRegTimer10msec)    {SysRegs.SysRegTimer10msecCount=0;}
   if(SysRegs.SysRegTimer50msecCount  >SysRegTimer50msec)    {SysRegs.SysRegTimer50msecCount=0;}
   if(SysRegs.SysRegTimer100msecCount >SysRegTimer100msec)   {SysRegs.SysRegTimer100msecCount=0;}
   if(SysRegs.SysRegTimer300msecCount >SysRegTimer300msec)   {SysRegs.SysRegTimer300msecCount=0;}
   if(SysRegs.SysRegTimer1000msecCount>SysRegTimer1000msec)  {SysRegs.SysRegTimer1000msecCount=0;}
   /*
    *
    */
  // SysRegs.SysStateReg.bit.PwrHoldRlyDOStatus = (SysRegs.SysCellDivVoltageF > 0.009f) ? 1u : 0u;

   //TODO: [완료] 디지털 입력 상태 읽기
   SysDigitalInput(&SysRegs);

  /*
   * current sensing detection
  */
  //  SysRegs.SysStateReg.bit.VCUWakeUpIn =CANARegs.PMSCMDRegs.bit.RlyOFF;// 1=off, 0=On
    //TODO: [완료] 전류 센싱 루틴
    SysCalCurrentHandle(&SysRegs);
    //TODO: [완료] 경고, 장애, 차단
    if(SysRegs.SysStateReg.bit.INITOK==1)
    {
        SysAlarmtCheck(&SysRegs);
        SysFaultCheck(&SysRegs);
       // SysProtectCheck(&SysRegs)
    }
   /*
    * Battery Alarm & Fault & Protect Check
    */
   if((SysRegs.SysAlarmReg.all > 0)&&(SysRegs.SysStateReg.bit.INITOK==1))
   {
       SysRegs.SysStateReg.bit.SysAalarm=1;
       CANARegs.ProtectState=1;
   }
   else
   {
       SysRegs.SysStateReg.bit.SysAalarm=0;
       CANARegs.ProtectState=0;
   }
   if((SysRegs.SysFaultReg.all > 0)&&(SysRegs.SysStateReg.bit.INITOK==1))
   {
       SysRegs.SysStateReg.bit.SysFault=1;
       CANARegs.ProtectState=2;
   }
   else
   {
       SysRegs.SysStateReg.bit.SysFault=0;
       CANARegs.ProtectState=0;
   }
   if((SysRegs.SysProtectReg.all> 0)&&(SysRegs.SysStateReg.bit.INITOK==1))
   {
      CANARegs.ProtectState=3;
      SysRegs.SysStateReg.bit.SysPrtct=1;
   }
   //TODO: [완료] SoC 계산 루틴
   if(SysRegs.SysStateReg.bit.INITOK==1)
   {
       EV240AhSocRegs.CellAgvVoltageF         = SysRegs.SysCellAgvVoltageF;
       EV240AhSocRegs.SysSoCCTF               = SysRegs.SysPackCurrentF;
       EV240AhSocRegs.SysSoCCTAbsF            = SysRegs.SysPackCurrentAsbF;
       EV240AhSocRegs.SoCStateRegs.bit.INITOK = SysRegs.SysStateReg.bit.INITOK;
       EV240AhSocRegs.state =  SOC_STATE_RUN;
       CalEVE240AhSocHandle(&EV240AhSocRegs);
       /*------------------------------------------------------------
        * 비고 : 아래 zone 재시드 분기는 폐지 (참고용 주석 보존).
        *        BMS ON 유지 중에는 전류적산 결과(SysPackSOCF)를 그대로 사용한다.
        *        - SOC 재시드는 BMS OFF->ON 부팅 init에서만 수행
        *        - NVRAM 저장은 별도 시퀀스에서 수행
        *        - 전류 측정 0.002% / SOC 허용 +/-5% 사양 내 안전
        *------------------------------------------------------------*/
       //if(EV240AhSocRegs.SoCStateRegs.bit.CalMeth==0)
       //{
       //    //260524: 런타임 zone 재판정 시 state는 CalEVE240AhSocHandle가 설정(SOSINIT/RUN)하므로 ZONE 중복 -> 제거
       //    //EV240AhSocRegs.state =  SOC_STATE_ZONE;
       //    SysCalSocZoneHandle(&SysRegs);
       //    if(SysRegs.SysSocInitRule == SOC_ZONE_cellVolt)
       //    {
       //        SysRegs.SysSOCF = EV240AhSocRegs.SysSocInitF;
       //        SysRegs.SysStateReg.bit.SysSocZone =0;
       //    }
       //    else if(SysRegs.SysSocInitRule == SOC_ZONE_NVR)
       //    {
       //        SysRegs.SysSOCF = (float32)(NVRZoneARDRegs.LastSOC/10.0f);
       //        SysRegs.SysStateReg.bit.SysSocZone =1;
       //    }
       //    /* sync seed to current SOC so charge/discharge start has no jump */
       //    EV240AhSocRegs.SysSocInitF = SysRegs.SysSOCF;
       //}
       //else if(EV240AhSocRegs.SoCStateRegs.bit.CalMeth==1)
       //{
       //    SysRegs.SysSOCF=EV240AhSocRegs.SysPackSOCF;
       //    //260524: SysSocZone은 직전 휴지 기준 zone 유지(적산 출발점). 적산 여부=SysSocMode(CC), 충/방전=SysDisCharMode 참조
       //}

       /* BMS ON hold : always use coulomb-counting result */
       /* keep call for downstream SysSocInitRule reference*/
       SysCalSocZoneHandle(&SysRegs);                    
       SysRegs.SysSOCF = EV240AhSocRegs.SysPackSOCF;
       SysRegs.SysStateReg.bit.SysSocMode = EV240AhSocRegs.SoCStateRegs.bit.CalMeth;
      // SysRegs.SysStateReg.bit.SysSocZone =1;
   }
   // TODO : [완료] VCU에서 CAN으로 데이터 수신 카운트 및 통신 상태 플래그 설정 루틴
   CANARegs.VCURxCout++;
   CANARegs.CharRxCout++;
   if(CANARegs.VCURxCout>=3000)
   {
       SysRegs.SysStateReg.bit.VCUComStatus=0;
       CANARegs.VCURxCout=3010;
   }
   else
   {
       SysRegs.SysStateReg.bit.VCUComStatus=1;
   }
   //TODO : [완료] 충전기에서 CAN으로 데이터 수신 카운트 및 통신 상태 플래그 설정 루틴
   if(CANARegs.CharRxCout>=4000)
   {
       SysRegs.SysStateReg.bit.CHAComStatus=0;
       CANARegs.CharRxCout=4010;
   }
   else
   {
       SysRegs.SysStateReg.bit.CHAComStatus=1;
   }
   switch(SysRegs.SysRegTimer5msecCount)
   {
       case 1:
//               SysRegs.SysStateReg.bit.PrtectStatus=0;
               if(SysRegs.SysStateReg.bit.SysAalarm==1)
               {
//                   SysRegs.SysStateReg.bit.PrtectStatus=1;
                   SysRegs.SysDigitalOutPutReg.bit.LEDAlarmOUT=1;
               }
               if(SysRegs.SysStateReg.bit.SysPrtct==1)
               {
 //                  SysRegs.SysStateReg.bit.PrtectStatus=2;
                   SysRegs.SysDigitalOutPutReg.bit.LEDProtectOUT=1;
                 //  SysRegs.SysMachine=PROTECTER;
               }
       break;
       default :
       break;

   }
   switch(SysRegs.SysRegTimer10msecCount)
   {
       case 1:
                //TODO: [완료] 셀 전압 읽은 값을 SysRegs에 복사하여
                   memcpy(&SysRegs.SysCellVoltageF[0],        &Slave0Regs.CellVoltageF[0],sizeof(float32)*7);
                   memcpy(&SysRegs.SysCellVoltageF[7],        &Slave1Regs.CellVoltageF[0],sizeof(float32)*8);
                   memcpy(&SysRegs.SysCellVoltageF[15],       &Slave2Regs.CellVoltageF[0],sizeof(float32)*7);
                   memcpy(&SysRegs.SysCellVoltageF[22],       &Slave3Regs.CellVoltageF[0],sizeof(float32)*8);
                  //TODO: [완료] 셀 전압 최소, 최대, 평균, 편차, 팩 전압 계산 루틴
                   SysCalVoltageHandle(&SysRegs);
                   SysRegs.SysStateReg.bit.CellVoltOk=0;
       break;
       case 2:
                   // TODO: [완료] 셀 온도 읽은 값을 SysRegs에 복사하여
                   memcpy(&SysRegs.SysCelltemperatureF[0],     &Slave0Regs.CellTemperatureF[0],sizeof(float32)*7);
                   memcpy(&SysRegs.SysCelltemperatureF[7],     &Slave1Regs.CellTemperatureF[0],sizeof(float32)*8);
                   memcpy(&SysRegs.SysCelltemperatureF[15],    &Slave2Regs.CellTemperatureF[0],sizeof(float32)*7);
                   memcpy(&SysRegs.SysCelltemperatureF[22],    &Slave3Regs.CellTemperatureF[0],sizeof(float32)*8);
                   // TODO: [완료] 셀 온도 최소, 최대, 평균 계산 루틴
                   SysCalTemperatureHandle(&SysRegs);
                   SysRegs.SysStateReg.bit.CellTempsOk=0;
       break;
       case 3:

       break;
       case 4:
               if(SysRegs.SysStateReg.bit.INITOK==1)
               {
                   MDCalVoltandTemsHandle(&SysRegs);
                   SysRegs.MDNumber++;
               }
       break;
       case 5:

            //   CANARegs.CharCONSTVolt=540;
            //   CANARegs.CahrConstantCurrt =300;
            //   CANARegs.SysPackPT  = (unsigned int)(SysRegs.SysPackParallelVoltageF*10);
            //   CANARegs.SysPackCT  = (unsigned int)(SysRegs.SysPackCurrentAsbF*10);

            //   if(SysRegs.CanComEable==1)
            //   {
            //       CANATX(0x61E,8,CANARegs.CharCONSTVolt,CANARegs.CahrConstantCurrt,CANARegs.CharCONSTVolt,CANARegs.CahrConstantCurrt);
            //   }
       break;
       case 6:

       break;
       case 7:
               //CANARegs.ChargerStateRegs.all=0;
/*
              CANARegs.VcuCharRxCout++;
               if(CANARegs.CharRxFlg==1)
               {
                   //SysRegs.SysStateReg.bit.SysDisCharMode=0;
               }
               else
               {
                  // SysRegs.SysStateReg.bit.SysDisCharMode=1;
               }
               if(CANARegs.VcuCharRxCout>=20)
               {
                 //  CANARegs.VcuRxFlg=0;
                   //CANARegs.VcuCharRxCout=0;
                  // CANARegs.CharRxFlg=0;
               }
               //1CANARegs.ChargerStateRegs.bit.BSACHAEnable=CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
               CANARegs.ChargerStateRegs.bit.BSACHAEnable=1;
               CANARegs.ChargerStateRegs.bit.BatNRly=1;
               CANARegs.ChargerStateRegs.bit.BatPRly=1;
               if(SysRegs.SysStateReg.bit.SysDisCharMode==0)
               {
           ///        CANARegs.ChargerStateRegs.bit.BSACHAEnable=1;
               }
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x61f,8,CANARegs.ChargerStateRegs.all,0x0000,0x0000,0x0000);
               }
*/
       break;
       default :
       break;
   }
   switch(SysRegs.SysRegTimer50msecCount)
   {
       case 1:

       break;
       case 5:
               // LEDSysState_H;
               // LEDSysState_L;
       break;
       case 10:
               // LEDSysState_H;
               // At 80MHZ, operation time is 33usec
               // Cal80VSysVoltageHandle(&SysRegs);
               // LEDSysState_L;
       break;
       case 20:
               // LEDSysState_H;
               // At 80MHZ, operation time is 33usec
               // LEDSysState_L;
       break;
       case 30 :


       break;
       default :
       break;
   }

   switch(SysRegs.SysRegTimer100msecCount)
   {
       case 5:
          //     memcpy(&CANARegs.Salve1VoltageCell[0], &Slave1Regs.CellVoltage[0],sizeof(unsigned int)*12);
       break;
       case 8:

       break;
       case 10:
               // TODO: [완료] SOC, SOH, 팩 전압, 전류 CAN 전송
               SysRegs.SysSOHF=100.0;
               CANARegs.SysPackPT  = (unsigned int)(SysRegs.SysPackParallelVoltageF*10);
               CANARegs.SysPackCT  = (int)(SysRegs.SysPackCurrentF*10);
               CANARegs.SysPackSOC = (int)(SysRegs.SysSOCF*10);
               CANARegs.SysPackSOH = (unsigned int)(SysRegs.SysSOHF*10);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x611,8,CANARegs.SysPackPT,CANARegs.SysPackCT,CANARegs.SysPackSOC,CANARegs.SysPackSOH);
               }
       break;
       case 12:
                //TODO: [완료] 시스템 상태, 경고, 장애, 차단 플래그 CAN 전송 루틴
                CANARegs.SysState                          = ComBine(CANARegs.ProtectState,CANARegs.DiviceState);
                SysRegs.SysStateReg.bit.SysSeqState        = SysRegs.SysMachine;
                SysRegs.SysStateReg.bit.RlySeqState        = PrtectRelayRegs.RlyMachine;
                SysRegs.SysStateReg.bit.SocSeqState        = EV240AhSocRegs.state;
                SysRegs.SysStateReg.bit.ChargerWakeUpIn    = CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
                SysRegs.SysStateReg.bit.SysSocMode         = EV240AhSocRegs.SoCStateRegs.bit.CalMeth;
                SysRegs.SysFaultReg.bit.PackComErr         = SysRegs.SysProtectReg.bit.PackCharCAN_Err|SysRegs.SysProtectReg.bit.PackVCUCAN_Err|SysRegs.SysProtectReg.bit.PackCTCAN_Err;

            //    SysRegs.SysStateReg.bit.ISOSPICOMERR       = SysRegs.SysProtectReg.bit.PackIOSPI_Err;
                SysRegs.SysStateReg.bit.CANCOMEnable       = SysRegs.CanComEable;
                CANARegs.SysStatus.bit.BalanceMode         = SysRegs.SysStateReg.bit.SysBalaMode;
                CANARegs.SysStatus.bit.NegRly              = SysRegs.SysDigitalOutPutReg.bit.NRlyOUT;
                CANARegs.SysStatus.bit.PoRly               = SysRegs.SysDigitalOutPutReg.bit.PRlyOUT;
                CANARegs.SysStatus.bit.PreCharRly          = SysRegs.SysDigitalOutPutReg.bit.ProRlyOUT;
                CANARegs.SysStatus.bit.WakeUpEn            = PrtectRelayRegs.State.bit.WakeUpEN;
                CANARegs.SysStatus.bit.KillSWstatus        = SysRegs.SysDigitalInputReg.bit.killSW;
                CANARegs.SysStatus.bit.ChargerEn           = CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
                CANARegs.SysStatus.bit.PWMBMS_Hold         = SysRegs.SysStateReg.bit.PwrHoldState;
             //   CANARegs.SysStatus.bit.KillSWstatus=1;
                if(SysRegs.CanComEable==1)
                {
                    CANATX(0x612,8,CANARegs.SysState,CANARegs.SysStatus.all,SysRegs.SysStateReg.Word.DataL,SysRegs.SysStateReg.Word.DataH);
                }
       break;
       case 14:
                //TODO: [완료] 시스템 경고, 장애, 차단 플래그 CAN 전송 루틴
               if(SysRegs.CanComEable==1)
               {

                   CANATX(0x613,8,SysRegs.SysAlarmReg.Word.DataL,SysRegs.SysFaultReg.Word.DataL,SysRegs.SysProtectReg.Word.DataL,SysRegs.SysProtectReg.Word.DataH);
               }
       break;
       case 17:
               // TODO: [완료] 충전 및 방전 전력 최대값과 연속값 CAN 통신 전송

               SysRegs.SysCHARGPWRPeakF        = 12.0;
               SysRegs.SysDISCHAPWRPeakF       = 36.0;
               SysRegs.SysCHARGPWRContintyF    = 6.0;
               SysRegs.SysDISCHAPWRContintyF   = 21.0;
               CANARegs.SysCHARGPWRContinty    = (Uint16)(SysRegs.SysCHARGPWRContintyF*10);
               CANARegs.SysDISCHAPWRContinty   = (Uint16)(SysRegs.SysDISCHAPWRContintyF*10);
               CANARegs.SysCHARGPWRPeak        = (Uint16)(SysRegs.SysCHARGPWRPeakF*10);
               CANARegs.SysDISCHAPWRPeak       = (Uint16)(SysRegs.SysDISCHAPWRPeakF*10);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x614,8,CANARegs.SysCHARGPWRContinty,CANARegs.SysDISCHAPWRContinty,CANARegs.SysCHARGPWRPeak,CANARegs.SysDISCHAPWRPeak);
               }
       break;
       case 20:
               //TODO: [완료] 셀 전압 최대값, 최소값, 평균값, 편차값 CAN 통신 전송
               CANARegs.CellVoltageMax          = (Uint16)(SysRegs.SysCellMaxVoltageF*1000);
               CANARegs.CellVoltageMin          = (Uint16)(SysRegs.SysCellMinVoltageF*1000);
               CANARegs.CellVoltageAgv          = (Uint16)(SysRegs.SysCellAgvVoltageF*1000);
               CANARegs.CellVoltageDiv          = (Uint16)(SysRegs.SysCellDivVoltageF*1000);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x615,8,CANARegs.CellVoltageMax,CANARegs.CellVoltageMin,CANARegs.CellVoltageAgv,CANARegs.CellVoltageDiv);
               }
       break;
       case 23:
               //TODO: [완료] 셀 온도 최대값, 최소값, 평균값, 편차값 CAN 통신 전송
               CANARegs.CellTemperaturelMAX    = (int16)(SysRegs.SysCellMaxTemperatureF*10);
               CANARegs.CellTemperaturelMIN    = (int16)(SysRegs.SysCellMinTemperatureF*10);
               CANARegs.CellTemperatureAVG     = (int16)(SysRegs.SysCellAgvTemperatureF*10);
               CANARegs.CellTemperatureDiv     = (Uint16)(SysRegs.SysCellDivTemperatureF*10);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x616,8,CANARegs.CellTemperaturelMAX,CANARegs.CellTemperaturelMIN,CANARegs.CellTemperatureAVG,CANARegs.CellTemperatureDiv);
               }
       break;
       case 26:
               //TODO: [완료] 셀 전압 최대값, 최소값, 평균값, 편차값 위치 값 CAN 통신 전송
               CANARegs.CellVoltageMaxNum      = SysRegs.SysVoltageMaxNum;
               CANARegs.CellVoltageMinNum      = SysRegs.SysVoltageMinNum;
               CANARegs.CellTemperatureMaxNum  = SysRegs.SysTemperatureMaxNum;
               CANARegs.CellTemperatureMinNUM  = SysRegs.SysTemperatureMinNum;
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x617,8,CANARegs.CellVoltageMaxNum,CANARegs.CellVoltageMinNum,CANARegs.CellTemperatureMaxNum,CANARegs.CellTemperatureMinNUM);
               }
       break;
       case 30:
                // TODO: [완료] 주요 연산 검증 위한 Ah연산값  SysTimeTick,NVRZoneARDRegs.LastSOC CAN 통신 전송 루틴
                CANARegs.SysPackAh = (int16)(EV240AhSocRegs.SysPackAhF*10.0);
                CANARegs.SysTimeTickDataL   =  (Uint16)(NVRZoneARDRegs.SysTimeTick & 0xFFFFu);
                CANARegs.SysTimeTickDataH   =  (Uint16)((NVRZoneARDRegs.SysTimeTick  >> 16) & 0xFFFFu);
                if(SysRegs.CanComEable==1)
                {
                    CANATX(0x618,8,CANARegs.SysPackAh,CANARegs.SysTimeTickDataL, CANARegs.SysTimeTickDataH, NVRZoneARDRegs.LastSOC);
                }
       break;
       case 35:
               CANARegs.MDVoltage[CANARegs.MDNumber]              = (Uint16)(SysRegs.MDVoltageF[CANARegs.MDNumber]*10);
               CANARegs.MDCellVoltAgv[CANARegs.MDNumber]          = (Uint16)(SysRegs.MDCellVoltAgvF[CANARegs.MDNumber]*1000);
               CANARegs.MDCellTempsAgv[CANARegs.MDNumber]         = (Uint16)(SysRegs.MDCellTempsAgvF[CANARegs.MDNumber]*10);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x619,8,CANARegs.MDNumber,
                                  CANARegs.MDVoltage[CANARegs.MDNumber],CANARegs.MDCellVoltAgv[CANARegs.MDNumber],CANARegs.MDCellTempsAgv[CANARegs.MDNumber]);
               }
               CANARegs.MDNumber++;
               if(CANARegs.MDNumber>3)
               {
                   CANARegs.MDNumber=0;
               }
       break;
       case 40:
               //TODO: [완료] CellNumCount 따른 셀 전압과 온도, 내부저항 값
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x61A,8,CANARegs.CellNumCount,
                                  CANARegs.SysCellVoltage[CANARegs.CellNumCount],CANARegs.SysCelltemperature[CANARegs.CellNumCount],CANARegs.SysCellInRegs[CANARegs.CellNumCount]);
               }
               CANARegs.CellNumCount++;
               if(CANARegs.CellNumCount>29) // C_CellNum=30;
               {
                   CANARegs.CellNumCount=0;
               }
       break;
       case 45:

       break;
       case 50:

            //   CANARegs.MDVoltage[4]              = (Uint16)(SysRegs.MDVoltageF[4]*10);
            //   CANARegs.MDCellVoltAgv[4]          = (Uint16)(SysRegs.MDCellVoltAgvF[4]*1000);
            //   CANARegs.MDCellTempsAgv[4]         = (Uint16)(SysRegs.MDCellTempsAgvF[4]*10);
            //   CANATX(0x61C,8,CANARegs.MDVoltage[4],CANARegs.MDCellVoltAgv[4],CANARegs.MDCellTempsAgv[4],0x0000);
       break;
       case 55:
               // TODO: [완료] 슬레이브 BMS 오류 카운트 CAN 통신 전송
               CANARegs.SlaveBMSErrCout[0]=Slave0Regs.ErrorCount;
               CANARegs.SlaveBMSErrCout[1]=Slave1Regs.ErrorCount;
               CANARegs.SlaveBMSErrCout[2]=Slave2Regs.ErrorCount;
               CANARegs.SlaveBMSErrCout[3]=Slave3Regs.ErrorCount;

               CANARegs.CANCom_0x61DDate0 = ComBine(CANARegs.SlaveBMSErrCout[CANARegs.SlaveBMSNumCout], CANARegs.SlaveBMSNumCout);
               CANARegs.CANCom_0x61DDate1 = ComBine(CANARegs.MailBox1RxCount, CANARegs.MailBox0RxCount);
               CANARegs.CANCom_0x61DDate2 = ComBine(CANARegs.MailBox3RxCount, CANARegs.MailBox2RxCount);
               CANARegs.CANCom_0x61DDate3 = 0x0000;
               CANATX(0x61B,8,CANARegs.CANCom_0x61DDate0,CANARegs.CANCom_0x61DDate1,CANARegs.CANCom_0x61DDate2,CANARegs.CANCom_0x61DDate3);
               CANARegs.SlaveBMSNumCout++;
               if(CANARegs.SlaveBMSNumCout>3)
               {
                   CANARegs.SlaveBMSNumCout=0;
               }
       break;
       case 60:
               if(CANARegs.HMICMDRegs.bit.HMI_MODE==1)
               {
                  /*
                   *    unsigned int     SysStatus              :3; // 0,1,2
                        unsigned int     SysRlyStatus           :3; // 3,4,5
                        unsigned int     SysProtectStatus       :2; // 6,7
                        unsigned int     SysSOCStatus           :2; // 8,9
                        unsigned int     SysDisCharMode         :1; // 10
                        unsigned int     HMICOMEnable           :1; // 11
                        unsigned int     HMIBalanceMode         :1; // 12
                        unsigned int     NRlyDOStatus           :1; // 13
                        unsigned int     PRlyDOStatus           :1; // 14
                        unsigned int     PreRlyDOStatus         :1; // 15
                        unsigned int     ISOSPICOMERR           :1; // 16
                        unsigned int     INCANCOMERR            :1; // 17
                        unsigned int     TCPIPTOMERR            :1; // 18
                        unsigned int     RS485COMERR            :1; // 19
                        unsigned int     ISORegERR              :1; // 20
                        unsigned int     MSDERR                 :1; // 21
                        unsigned int     RlyERR                 :1; // 22
                        unsigned int     INITOK                 :1; // 23
                        unsigned int     SysBalanceMode         :1; // 24
                        unsigned int     SysBalanceEn           :1; // 25
                        unsigned int     SysAalarm              :1; // 26
                        unsigned int     SysFault               :1; // 27
                        unsigned int     SysProtect             :1; // 28
                        unsigned int     CellVoltOk             :1; // 29
                        unsigned int     CellTempsOk            :1; // 30
                        unsigned int     SW31                   :1; // 31
                   */
                // CANATX(0x701,8,SysRegs.SysStateReg.Word.DataL,SysRegs.SysStateReg.Word.DataH,0X000,0x0000);
               }
       break;
       case 65:
               if((CANARegs.HMICMDRegs.bit.HMI_MODE==1)&&(CANARegs.HMICMDRegs.bit.HMI_CellVoltReq==1))
               {
                 CANARegs.HMICellVoltCout++;
                 if(CANARegs.HMICellVoltCout>C_HmiCellVoltCount)
                 {
                     CANARegs.HMICellVoltCout=0;
                 }
           //      CANARegs.HMICellVoltNum=CANARegs.HMICellVoltCout*3;
           //      CANATX(0x702,8,CANARegs.HMICellVoltNum,
           //                     CANARegs.SysCellVoltage[CANARegs.HMICellVoltNum],
           //                     CANARegs.SysCellVoltage[CANARegs.HMICellVoltNum+1],
           //                     CANARegs.SysCellVoltage[CANARegs.HMICellVoltNum+2]);
               }
       break;
       case 70:
               if((CANARegs.HMICMDRegs.bit.HMI_MODE==1)&&(CANARegs.HMICMDRegs.bit.HMI_CellTempsReq==1))
               {
                   CANARegs.HMICellTempsCout++;
                   if(CANARegs.HMICellTempsCout >C_HmiCellTempCount)
                   {
                       CANARegs.HMICellTempsCout=0;
                   }
                   CANARegs.HMICellTempsNum=CANARegs.HMICellTempsCout*3;
                   //CANATX(0x703,8,CANARegs.HMICellTempsNum,
                   //               CANARegs.SysCelltemperature[CANARegs.HMICellTempsNum],
                   //               CANARegs.SysCelltemperature[CANARegs.HMICellTempsNum+1],
                   //               CANARegs.SysCelltemperature[CANARegs.HMICellTempsNum+2]);
               }
       break;
       case 75:
               if(CANARegs.HMICMDRegs.bit.HMI_MODE==1)
               {
                  // CANATX(0x704,8,CANARegs.MailBox0RxCount,CANARegs.MailBox1RxCount,CANARegs.MailBox2RxCount,0x000);
               }
       break;
       case 80:

       break;
       default:
       break;
   }

   switch(SysRegs.SysRegTimer300msecCount)
   {
       case 1:
              // PWRRlyHoldHandle(&SysRegs);
       break;
       case 2:
               if(CANARegs.HMICMDRegs.bit.HMI_MODE==1)
               {
                   CANARegs.HMIISOSPIErrCount++;
                   if(CANARegs.HMIISOSPIErrCount>=C_HMIISOSPIErrCount)
                   {
                       CANARegs.HMIISOSPIErrCount=0;
                   }
                   CANARegs.HMIISOSPIErrNum=CANARegs.HMIISOSPIErrCount*3;
                   //CANATX(0x705,8,CANARegs.HMIISOSPIErrNum,
                   //               SysRegs.SlaveVoltErrCount[CANARegs.HMIISOSPIErrNum],
                   //               SysRegs.SlaveVoltErrCount[CANARegs.HMIISOSPIErrNum+1],
                   //               SysRegs.SlaveVoltErrCount[CANARegs.HMIISOSPIErrNum+2]);
               }

       break;
       case 10:
               //TODO : [완료] 충전 설정값 충전기 CAN 전송
               //TODO : [완료] 동적CV 게이트(CHA=1&VCU=0) 동작 / SOC<85% 52.0V CC / 85~90% 52.0->50.4V 선형강하 / SOC>=90% 50.4V CV
               /* Run charge target only when charger connected (CHA=1) and VCU not (VCU=0).
                  VCU=1 and CHA=1 together is a system fault. */
               if((SysRegs.SysStateReg.bit.CHAComStatus == 1u) && (SysRegs.SysStateReg.bit.VCUComStatus == 0u))
               {
                   if(SysRegs.SysSOCF < 85.0F)
                   {
                       CANARegs.CharCONSTVolt = 520u;                            /* 52.0V */
                   }
                   //else if(SysRegs.SysSOCF < 93.0F)
                   else if((SysRegs.SysSOCF >= 85.0F) && (SysRegs.SysSOCF < 93.0F))
                   {
                       /* 520 at 85%, 504 at 93% : slope = (520-504)/(93-85) = 2.0 per % */
                       CANARegs.CharCONSTVolt = (Uint16)(520.0F - (SysRegs.SysSOCF - 85.0F) * 2.0F);
                   }
                   // TODO : 
                   if(SysRegs.SysSOCF >= 93.0F)
                   {
                       CANARegs.CharCONSTVolt = 504u;                            /* 50.4V */
                   }
                   //TODO CANARegs.CharCONSTVolt 선형식 오류 방지 위함
                   if(CANARegs.CharCONSTVolt>521u){CANARegs.CharCONSTVolt = 504u;}               
               }
               else
               {
                   /* not charging, or system fault : safe default target */
                   CANARegs.CharCONSTVolt = 520u;
               }
               //TODO :[튜닝] 충전기 CC 전류값 25.0A, 30.0A 시에 충전기 자체적으로 중지됨
               CANARegs.CahrConstantCurrt =300;
               CANARegs.CharCONSTSOC=1000;
               CANARegs.SysPackPT  = (unsigned int)(SysRegs.SysPackParallelVoltageF*10);//545;//(unsigned int)(SysRegs.SysPackParallelVoltageF*10);
               CANARegs.SysPackCT  = (unsigned int)(SysRegs.SysPackCurrentAsbF*10);//300;(unsigned int)(SysRegs.SysPackCurrentAsbF*10);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x61E,8,CANARegs.CharCONSTVolt,CANARegs.CahrConstantCurrt,CANARegs.SysPackPT,CANARegs.SysPackCT);
               }

       break;
       case 30:
              // CANARegs.ChargerStateRegs.bit.BSACHAEnable=CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
               CANARegs.ChargerStateRegs.bit.BatNRly=1;
               CANARegs.ChargerStateRegs.bit.BatPRly=1;
              // CANARegs.ChargerStateRegs.bit.BSACHAEnable =1;//SysRegs.SysStateReg.bit.BSACHAEnable;
               CANARegs.ChargerStateRegs.bit.BSACHAEnable =1;
               SysRegs.TargetPackVoltF  = (float32)(CANARegs.CharCONSTVolt/10.0f);
               SysRegs.TargetPackSocF   = (float32)(CANARegs.CharCONSTSOC/10.0f);
               CANATX(0x61f,8,CANARegs.ChargerStateRegs.all,0x0000,0x0000,0x0000);
               //CANARegs.ChargerStateRegs.all=0;
               /*
               CANARegs.VcuCharRxCout++;
               if(CANARegs.CharRxFlg==1)
               {
                   //SysRegs.SysStateReg.bit.SysDisCharMode=0;
               }
               else
               {
                  // SysRegs.SysStateReg.bit.SysDisCharMode=1;
               }
               if(CANARegs.VcuCharRxCout>=20)
               {
                 //  CANARegs.VcuRxFlg=0;
                   //CANARegs.VcuCharRxCout=0;
                  // CANARegs.CharRxFlg=0;
               }
               //1CANARegs.ChargerStateRegs.bit.BSACHAEnable=CANARegs.ChargerResgsStauts.bit.Char_DOSTatue;
               CANARegs.ChargerStateRegs.bit.BSACHAEnable=1;
               CANARegs.ChargerStateRegs.bit.BatNRly=1;
               CANARegs.ChargerStateRegs.bit.BatPRly=1;
               if(SysRegs.SysStateReg.bit.SysDisCharMode==0)
               {
           ///        CANARegs.ChargerStateRegs.bit.BSACHAEnable=1;
               }
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x61f,8,CANARegs.ChargerStateRegs.all,0x0000,0x0000,0x0000);
               }
               */
       break;
       default :
       break;
   }
   switch(SysRegs.SysRegTimer1000msecCount)
   {
       case 1:
               CANARegs.CharRxCount++;
               if(CANARegs.CharRxCount>15)
               {
                   CANARegs.ChargerResgsStauts.all=0;
                   CANARegs.CharRxCount=25;
               }
       break;
       case 10:
               CANARegs.ProductInfro = ComBine(Product_Version,Product_Type);
               CANARegs.SysConFig    = ComBine(Product_SysCellVauleP,Product_SysCellVauleS);
               if(SysRegs.CanComEable==1)
               {
                   CANATX(0x610,8,CANARegs.ProductInfro,CANARegs.SysConFig,Product_Voltage,Product_Capacity);
               }
       break;
       case 100:
               /*--------------------------------------------------------------
                * PwrHoldCount 카운터 처리 (1초 주기 실행)
                *
                * 동작:
                * - PWRRly(WakeUpOut) = 0 이면 카운트 증가
                *   → 전원 출력 OFF 상태에서 Hold 유지 시간 누적
                *
                * - PWRRly(WakeUpOut) = 1 이면 카운터 초기화
                *   → 전원 ON 상태에서는 Hold 타이머 의미 없음
                *
                * - 최대 14400초 (4시간)까지 카운트
                *--------------------------------------------------------------*/
               if(SysRegs.SysStateReg.bit.WakeUpOut == 0u)
               {
                   if(SysRegs.PwrHoldCount <= 14400u)
                   {
                       SysRegs.PwrHoldCount++;
                   }
               }
               else
               {
                   SysRegs.PwrHoldCount = 0u;
               }


       break;
       default :
       break;
   }



  if(SysRegs.SysStateReg.bit.INITOK==1)
  {
       SysRegs.SysDigitalOutPutReg.bit.NRlyOUT=PrtectRelayRegs.State.bit.NRlyDO;
       SysRegs.SysDigitalOutPutReg.bit.PRlyOUT=PrtectRelayRegs.State.bit.PRlyDO;
       SysRegs.SysDigitalOutPutReg.bit.ProRlyOUT=PrtectRelayRegs.State.bit.PreRlyDO;
       SysRegs.SysDigitalOutPutReg.bit.PWRHOLD=SysRegs.SysStateReg.bit.PwrHoldState;
       SysDigitalOutput(&SysRegs);
       //SysRegs.SysDigitalOutPutReg.bit.PWRHOLD=SysRegs.SysStateReg.bit.PwrHoldRlyDOStatus;
  }

//   LEDSysState_L;
   if(SysRegs.MainIsr1>3000) {SysRegs.MainIsr1=0;}


   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
interrupt void ISR_CANRXINTA(void)
{
    struct ECAN_REGS ECanaShadow;
    if(ECanaRegs.CANGIF0.bit.GMIF0 == 1)
    {
        CANARegs.MailBoxRxCount++;
        if(CANARegs.MailBoxRxCount>250){CANARegs.MailBoxRxCount=0;LEDCANState_T;}
        if(ECanaRegs.CANRMP.bit.RMP0==1)
        {
            if(ECanaMboxes.MBOX0.MSGID.bit.STDMSGID==0x3C2)
            {
                SysRegs.CTCANErrCheck=0;
                CANARegs.MailBox0RxCount++;
                if(CANARegs.MailBox0RxCount>100){CANARegs.MailBox0RxCount=0;}
                SysRegs.SysCurrentData.byte.CurrentH   = (ECanaMboxes.MBOX0.MDL.byte.BYTE0<<8)|(ECanaMboxes.MBOX0.MDL.byte.BYTE1);
                SysRegs.SysCurrentData.byte.CurrentL   = (ECanaMboxes.MBOX0.MDL.byte.BYTE2<<8)|(ECanaMboxes.MBOX0.MDL.byte.BYTE3);
            }
            ECanaRegs.CANRMP.bit.RMP0 = 1;
        }
        if(ECanaRegs.CANRMP.bit.RMP1==1)
        {
            if(ECanaMboxes.MBOX1.MSGID.bit.STDMSGID==0x600)
            {
               CANARegs.MailBox1RxCount++;
               if(CANARegs.MailBox1RxCount>250){CANARegs.MailBox1RxCount=0;}
               CANARegs.PMSCMDRegs.all      =  (ECanaMboxes.MBOX1.MDL.byte.BYTE1<<8)|(ECanaMboxes.MBOX1.MDL.byte.BYTE0);
               CANARegs.VCURxCout=0;
            }
            ECanaRegs.CANRMP.bit.RMP1 = 1;
        }
        if(ECanaRegs.CANRMP.bit.RMP2==1)
        {
            if(ECanaMboxes.MBOX2.MSGID.bit.STDMSGID==0x700)
            {
                SysRegs.HMICANErrCheck=0;
                CANARegs.MailBox2RxCount++;
                if(CANARegs.MailBox2RxCount>250){CANARegs.MailBox2RxCount=0;}

                 CANARegs.HMICMDRegs.all      =  (ECanaMboxes.MBOX2.MDL.byte.BYTE1<<8)|(ECanaMboxes.MBOX2.MDL.byte.BYTE0);
                 CANARegs.HMISocInitValue     =  (ECanaMboxes.MBOX2.MDL.byte.BYTE3<<8)|(ECanaMboxes.MBOX2.MDL.byte.BYTE2);
                // CANARegs.HMICEllTempsAgv     =  (ECanaMboxes.MBOX2.MDH.byte.BYTE5<<8)|(ECanaMboxes.MBOX2.MDH.byte.BYTE4);
                // CANARegs.HMICEllVoltMin      =  (ECanaMboxes.MBOX2.MDH.byte.BYTE7<<8)|(ECanaMboxes.MBOX2.MDH.byte.BYTE6);
                //if(CANARegs.HMICMDRegs.bit.HMI_Reset==1)
                //{
                //    CANARegs.HMICMDRegs.bit.HMI_RlyEN=0;
                //}
            }
            ECanaRegs.CANRMP.bit.RMP2 = 1;
        }

         if(ECanaRegs.CANRMP.bit.RMP3==1)
        {
            if(ECanaMboxes.MBOX3.MSGID.bit.STDMSGID==0x702)
            {
                CANARegs.MailBox3RxCount++;
                CANARegs.CharRxCount=0;
                if(CANARegs.MailBox3RxCount>250){CANARegs.MailBox3RxCount=0;}
                CANARegs.ChargerSWVer              =  (ECanaMboxes.MBOX3.MDL.byte.BYTE1<<8)|(ECanaMboxes.MBOX3.MDL.byte.BYTE0);
                CANARegs.ChargerConstVoltage       =  (ECanaMboxes.MBOX3.MDL.byte.BYTE2<<8)|(ECanaMboxes.MBOX3.MDL.byte.BYTE3);
                CANARegs.ChargerConstCurrent       =  (ECanaMboxes.MBOX3.MDH.byte.BYTE5<<8)|(ECanaMboxes.MBOX3.MDH.byte.BYTE4);
                CANARegs.ChargerResgsStauts.all    =  (ECanaMboxes.MBOX3.MDH.byte.BYTE7<<8)|(ECanaMboxes.MBOX3.MDH.byte.BYTE6);
            }
            CANARegs.CharRxCout=0;
            CANARegs.CharRxFlg=1;
            ECanaRegs.CANRMP.bit.RMP3 = 1;
        }


    }
    ECanaRegs.CANGIF0.all = ECanaRegs.CANGIF0.all;
    ECanaRegs.CANGIF1.all = ECanaRegs.CANGIF1.all;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

   // IER |= 0x0100;                  // Enable INT9
   // EINT;

}//EOF
/*
interrupt void cpu_timer2_isr(void)
{  EALLOW;
   CpuTimer2.InterruptCount++;
   // The CPU acknowledges the interrupt.
  // A_OVCHACurrent;
   EDIS;
}
*/
