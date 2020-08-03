/*
 * File:   161518_temp_main.c
 * Author: dell
 *
 * Created on December 11, 2018, 9:10 PM
 */


#include <xc.h>
#include <pic16f1518.h>
#include "LCD_hd44780u_qy_2004a.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>


// PIC16F1518 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
//#pragma config FOSC = 0x2       // Oscillator external high speed
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable (PWRT enabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = ON   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable bit (VCAP pin function disabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LPBOR = OFF      // Low-Power Brown Out Reset (Low-Power BOR is disabled)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)


char wInterruptText[wInterruptTextSize];

char wHexTemp[20];
uint8_t wTrial=0;


uint8_t  gErrorCode;
void Uint8ToTxt(uint8_t iVal, char* oText )
{
    uint8_t wTen;
    uint8_t wUnity;
    uint8_t wHundred;
    uint8_t wPos=0;

    wHundred = iVal/100;
    iVal = iVal % 100;
    wTen = iVal/10;
    iVal = iVal %10;
    wUnity = iVal;

    if(wHundred !=0 )
    {
        oText[0] = '0' + wHundred;
        wPos++;
    }
    if(wTen != 0)
    {
      oText[wPos] = '0' + wTen;
      wPos++;
    }
    oText[wPos] = '0' + wUnity;
    wPos++;
    oText[wPos] = 0;
}

void Uint16ToTxt(uint16_t iVal, char* oText )
{
    uint8_t wTenThousand;
    uint8_t wThousand;
    uint8_t wTen;
    uint8_t wUnity;
    uint8_t wHundred;
    uint8_t wPos=0;

    wTenThousand = iVal/10000;
    iVal = iVal % 10000;
    wThousand = iVal/1000;
    iVal = iVal % 1000;    
    wHundred = iVal/100;
    iVal = iVal % 100;
    wTen = iVal/10;
    iVal = iVal %10;
    wUnity = iVal;

    if(wTenThousand !=0 )
    {
        oText[0] = '0' + wTenThousand;
        wPos++;
    }
    if(wThousand !=0 )
    {
        oText[wPos] = '0' + wThousand;
        wPos++;
    }
    if(wHundred !=0 )
    {
        oText[wPos] = '0' + wHundred;
        wPos++;
    }
    if(wTen != 0)
    {
      oText[wPos] = '0' + wTen;
      wPos++;
    }
    oText[wPos] = '0' + wUnity;
    wPos++;
    oText[wPos] = '\0';
}

void SetTimer1AndTimer0(float iTime) //Time in Second
{
	uint16_t wPreScaler = 0x1;
	uint8_t wLastGoodPreScaler = -1;
	uint32_t wTimer0Update = _XTAL_FREQ;
	wTimer0Update = wTimer0Update / 4; // Internal instruction cycle clock (FOSC/4)

	const uint32_t wMaxNumberofCycle = 0xFFFF00;

	float wMaxTime;

	for (int8_t wPrescaler = 8; wPrescaler> -1; wPrescaler--)
	{
		uint16_t wTempPrescaler = 1;
		wMaxTime = ((float)wMaxNumberofCycle * (wTempPrescaler << wPrescaler)) / wTimer0Update;
		if (wMaxTime > (iTime))
		{
			wLastGoodPreScaler = wPrescaler;
		}
		else
		{
			break;
		}
	}

	OPTION_REGbits.TMR0CS = 0;  // using Internal instruction cycle clock (FOSC/4) to increment TIMER0
    INTCONbits.TMR0IE = 0; //Interrupt on timer0
    INTCONbits.TMR0IF = 0; //No interrupt on timer0 overflow
    
    T1CONbits.TMR1CS = 0; //Clock select OCS/4
    T1CONbits.T1CKPS = 0; //prescaler 1:1
    T1CONbits.T1OSCEN = 0; //Not used for this setup
    T1CONbits.nT1SYNC = 1; //Sync ,,,
    
    T1GCONbits.T1GSS = 1; // TMR0 overflow GatePin
    T1GCONbits.T1GTM = 0; //disabling the toggle mode
    T1GCONbits.T1GSPM = 1; //Detect single pulse from tmr0 overflow
    T1GCONbits.T1GGO_nDONE = 1;
    T1GCONbits.TMR1GE = 1; //TMr1 counts controller by gate
    T1GCONbits.T1GPOL = 1; //TMr1 counts when gate is high
    PIR1bits.TMR1GIF = 0; //Disabling timer1 Gate interrupt flag
    
    
	uint32_t wNbOfTimer0Increment = 0;
	if (wLastGoodPreScaler == 0)
	{
		OPTION_REGbits.PSA = 1; //No prescaler to timer 0
		wNbOfTimer0Increment = iTime * wTimer0Update;

	}
	else
	{
		OPTION_REGbits.PSA = 0; //Prescaler enable to timer 0
		OPTION_REGbits.PS = wLastGoodPreScaler - 1;
		wNbOfTimer0Increment = iTime * wTimer0Update / (1 << wLastGoodPreScaler);
	}
      char wPrintBuffer[21];
	  Uint16ToTxt(0xffff - (wNbOfTimer0Increment / 256),wPrintBuffer);
      setCursorPosition(2,0);
      lcdWriteText("TMR1:");
      lcdWriteText(wPrintBuffer);
      setCursorPosition(2,11);
      lcdWriteText("TMR0:");
      Uint8ToTxt(255 - (wNbOfTimer0Increment % 256),wPrintBuffer);
      lcdWriteText(wPrintBuffer);
      setCursorPosition(3,0);
      lcdWriteText("Pres:");
      Uint8ToTxt(wLastGoodPreScaler - 1,wPrintBuffer);
      lcdWriteText(wPrintBuffer);

    
    PIE1bits.TMR1IE = 1; //Enabling timer 1 interrupt
    PIE1bits.TMR1GIE = 1; //Enable the gate interrupt to reset the Pulse
    INTCONbits.PEIE = 1; //Enabling Peripheral interrupt
    INTCONbits.GIE = 1; //Enabling interrupt
    T1CONbits.TMR1ON = 1;
    
    TMR1 = 0xffff - (wNbOfTimer0Increment / 256);
    TMR0 = 255 - (wNbOfTimer0Increment % 256);
}



void Uint16DecimalToTxt(uint16_t iVal, char* oText )
{
    uint8_t wThousand;
    uint8_t wHundred;
    uint8_t wTen;
    uint8_t wUnity;
    uint8_t wDecimal;
    uint8_t wPos=0;

    wThousand = iVal/10000;
    iVal = iVal % 10000;
    wHundred = iVal/1000;
    iVal = iVal % 1000;    
    wTen = iVal/100;
    iVal = iVal % 100;
    wUnity = iVal/10;
    iVal = iVal %10;
    wDecimal = iVal;

    if(wThousand !=0 )
    {
        oText[0] = '0' + wThousand;
        wPos++;
    }
    if(wHundred !=0 )
    {
        oText[wPos] = '0' + wHundred;
        wPos++;
    }
    if(wTen != 0)
    {
      oText[wPos] = '0' + wTen;
      wPos++;
    }

    oText[wPos] = '0' + wUnity;
    wPos++;
    oText[wPos] = ',';
    wPos++;
    oText[wPos] = '0' + wDecimal;
    wPos++;
    oText[wPos] = '\0';
}

void PrintLog(char* iText)
{
    char wInterruptTextLen = strlen(iText);
    
    if(wInterruptTextLen !=0)
    {
        lcdWriteText(iText);
        memset(iText,0,wInterruptTextLen);
    }   
}

void Debounce(uint8_t iSwitch,uint16_t* ioTimer, uint8_t* swPressed)
{
    if(iSwitch == 0) //Button pressed
    {
      (*ioTimer)++;
    }
    else
    {
      *ioTimer = 0;   
    }
    if(*ioTimer == 1500)
    {
      *swPressed = 1;
    }
    if(*ioTimer == 8000)
    {
      *ioTimer = 2001;
      *swPressed = 1;
    }
}



#define ENTERBotton              PORTBbits.RB0
#define ENTERBottonDirection     TRISBbits.TRISB0
#define UPBotton                 PORTBbits.RB1
#define UPBottonDirection        TRISBbits.TRISB1
#define DOWNBotton               PORTBbits.RB2
#define DOWNBottonDirection      TRISBbits.TRISB2
#define COMMANDOnButton          PORTBbits.RB4
#define COMMANDOnBottonDirection TRISBbits.TRISB4
#define WELDOut                  PORTBbits.RB5
#define WELDOutBottonDirection   TRISBbits.TRISB5

#define DISPTACHERSIGNAL T1CONbits.TMR1CS

uint8_t wTimer1IntCounter=0;
uint8_t wTimer0Counter=0;
uint8_t wMenuSpotWelding=0;
uint8_t wMenuHeating=0;

int16_t wHumidity=0;
int16_t wTemperature=0;

int16_t wTempSet=210;

enum eMenuSpotWelding{eMenu=0, eSetSpotTime, eManualMode, eRepeatCycle};
enum eMenuHeating{eHMenu=0, eHSetHeatingTime, eHPowerDelevry};

uint16_t wSpotTime=75;
uint8_t wPowerDelevry=0;
uint8_t wRepeatCycle=0;

enum eMode{eSpotWelding=0,eBoltHeating};


uint8_t wMenu=0;

void main(void) 
{
  INTCONbits.GIE = 0; //Disable interrupt
  
  if(PCONbits.nBOR == 1)
  {
    PCONbits.nBOR;  
  }
  OSCCONbits.SCS = 0x2; // Clock determined by FOSC<2:0> in Configuration Word 1
  OSCCONbits.IRCF = 0xf; // Set frequency to 16 mhz not used since using external clock
  gErrorCode =0;
    
  char wPrintBuffer[20];
  int16_t wHumidityPrev=0;
  int16_t wTemperaturePrev=0;
  memset(wInterruptText,0,sizeof(wInterruptText));

  
  uint8_t wUpBottonPressed=0;
  uint8_t wDownBottonPressed=0;
  uint8_t wEnterBottonPressed=0;
  uint8_t wCommandOnPressed=0;
  
  uint8_t wEditingMode=0;

  uint8_t wUpdateMenu=1;
  
  uint16_t wIterationCounter=0;
  uint16_t wDebounceEnter=0;
  uint16_t wDebounceUp=0;
  uint16_t wDebounceDown=0;
  uint16_t wDebounceCommandOn=0;
  
  PORTA = 0x00;
  
  //Dispatcher parameter
  
  T1CONbits.TMR1CS = 0x00; // Using instruction clock Fosc devi 4
  T1CONbits.T1OSCEN = 0x0; //Not used since we are using the internal oscillator
  T1CONbits.T1CKPS = 0x3; // Setting no prescaler
  T1CONbits.nT1SYNC = 0; // Maybe not used since we are using internal oscillation
  T1CONbits.TMR1ON = 0; // disEnabling the timer1
  PIE1bits.TMR1IE =0; //disEnabling the timer1 interrupt
  //INTCONbits.PEIE = 1; // Is done later on

  //TempGetTimer
  OPTION_REGbits.PS = 0x2;
  OPTION_REGbits.TMR0CS = 0;
  OPTION_REGbits.PSA = 0;
  INTCONbits.TMR0IE = 0; //Disable Timer 0 Interrupt
  
  
  //Button configuration
  PORTB = 0x00;
  ANSELB = 0x00; //Setting All to digital
  TRISB = 0x0F; //Setting bit 0 1 2 3 to input
  WPUB = 0x1F; // Activation of weak pull up

    ENTERBotton=0;
    UPBotton=0;
    DOWNBotton=0;
    COMMANDOnButton = 0;
    WELDOut = 0;

    ENTERBottonDirection = 1;     //Set as input
    UPBottonDirection = 1;        //Set as input
    DOWNBottonDirection = 1;      //Set as input
    COMMANDOnBottonDirection = 1; //Set as input
    WELDOutBottonDirection = 0;   //Set as output
    APFCONbits.CCP2SEL=0;

  OPTION_REGbits.nWPUEN = 0; //Enable WeekPull up
  
    //Led configuration
  PORTCbits.RC5 = 0;
  ANSELCbits.ANSC5 = 0; //Setting All to digital
  TRISCbits.TRISC5 = 0; //Setting bit 0 1 2 3 to input

       

  //INTCONbits.GIE = 1; //Enable interrupt
  
  initLCD();

  clearDisplay();
  __delay_ms(100);
  powerOnLcd();
  __delay_ms(100);
  setCursorOff();
  __delay_ms(100);
  moveCursorToHome();
  __delay_ms(100);
  setNotBlinkingCursor();
  __delay_ms(100);
 
  int wCounter=0;
  char wConv[4]={'+',0, 'x',0, };
  int wTemp=0;

  
  clearDisplay();
  moveCursorToHome();
  __delay_ms(30);
  
  setCursorPosition(0,0);      
 
  
/*  while(1)
  {
      Uint16ToTxt(TMR1,wPrintBuffer);
      setCursorPosition(0,0);
      lcdWriteTextFullLine(wPrintBuffer);
      setCursorPosition(0,10);
      lcdWriteTextFullLine("TMR1");
      Uint8ToTxt(TMR0,wPrintBuffer);
      setCursorPosition(1,0);
      lcdWriteTextFullLine(wPrintBuffer);
      setCursorPosition(1,10);
      lcdWriteTextFullLine("TMR0");
      __delay_ms(300);
  }
  
  */
  while(1)
  {  
    if( wUpdateMenu )
    {
      wUpdateMenu = 0;
      
      switch(wMenu ) //{eShowTime=0,eShowTemp,eShowMode,eSetTime=128,eSetTemp=129,eSetMode=130};
      {
        case eSpotWelding:
          switch(wMenuSpotWelding)
          {
              case eMenu:
                  setCursorPosition(0,0);
                  lcdWriteText("Menu SpotWelding   ");
                  break;
              case eSetSpotTime:
                  setCursorPosition(0,0);
                  lcdWriteText("SetSpotTime       ");
                  setCursorPosition(1,0);
                  Uint8ToTxt(wSpotTime,wPrintBuffer);
                  strcat(wPrintBuffer," ms");
                  setCursorPosition(1,0);
                  lcdWriteTextFullLine(wPrintBuffer);
                  break;       
              case eManualMode:
                  setCursorPosition(0,0);
                  lcdWriteText("Manual Mode           ");
                  break;        
              case eRepeatCycle:
                  setCursorPosition(0,0);
                  lcdWriteText("RepeatCycle           ");
                  break;
              default:
                  wMenuSpotWelding = eMenu;
                  break;
          }
          break;
        case eBoltHeating:
          switch(wMenuHeating)
          {
              case eHMenu:
                  setCursorPosition(0,0);
                  lcdWriteText("Menu HeatingBolt    ");
                  break;
              case eHSetHeatingTime:
                  setCursorPosition(0,0);
                  lcdWriteText("SetSpotTime           ");
                  break;       
              case eHPowerDelevry:
                  setCursorPosition(0,0);
                  lcdWriteText("PowerDelevry           ");
                  break;        
              default:
                  wMenuHeating = eHMenu;
                  break;
          }
          break;
        default:
          wMenu = eSpotWelding;
          break;
      }
      if(wEditingMode == 1)
      {
          setCursorPosition(0,19);
          lcdWriteText("E");
      }
      else
      {
          setCursorPosition(0,19);
          lcdWriteText(" ");
      }
    }
    
    wIterationCounter++;
   Debounce(ENTERBotton,&wDebounceEnter,&wEnterBottonPressed);
   Debounce(UPBotton,&wDebounceUp,&wUpBottonPressed);
   Debounce(DOWNBotton,&wDebounceDown,&wDownBottonPressed);
   Debounce(COMMANDOnButton,&wDebounceCommandOn,&wCommandOnPressed);
   

   if(wUpBottonPressed == 1 )
   {
        wUpdateMenu=1;
        wUpBottonPressed = 0;
        if(wEditingMode == 0)
        {
            switch(wMenu)
            {
                case eSpotWelding:
                    switch(wMenuSpotWelding)
                    {
                        case eMenu:
                        case eSetSpotTime:
                        case eManualMode:
                            wMenuSpotWelding++;
                            break;        
                        case eRepeatCycle:
                            wMenuSpotWelding=eMenu;
                            break;
                        default:
                            wMenuSpotWelding=eMenu;
                            break;
                    }
                    break;
                case eBoltHeating:
                    switch(wMenuHeating)
                    {
                        case eHMenu:
                        case eHSetHeatingTime:
                            wMenuHeating++;
                            break;
                        case eHPowerDelevry:
                            wMenuHeating=eHMenu;
                            break;
                        default:
                            wMenuHeating=eHMenu;
                            break;
                    }
                    break;
                default:
                    wMenu=eSpotWelding;
                    break;
            }
        }
        else
        {
            switch(wMenu)
            {
                case eSpotWelding:
                    switch(wMenuSpotWelding)
                    {
                        case eMenu:
                            wMenu++;
                            break;
                        case eSetSpotTime:
                            wSpotTime = wSpotTime+75;
                            break;
                        case eManualMode:
                            wPowerDelevry++;
                            break;
                        case eRepeatCycle:
                            wRepeatCycle++;
                            break; 
                        default:
                            break;
                    }
                    break;
                case eBoltHeating:
                    switch(wMenuHeating)
                    {
                        case eHMenu:
                            wMenu++;
                            break;
                        case eHSetHeatingTime:
                            wSpotTime++;
                            break;
                        case eHPowerDelevry:
                            wPowerDelevry++;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    wMenu=eSpotWelding;
                    break;
            }
        }
   }
   
   if(wDownBottonPressed == 1 )
   {
        wUpdateMenu=1;
        wDownBottonPressed = 0;
       if(wEditingMode == 0)
       {
            switch(wMenu)
            {
                case eSpotWelding:
                    switch(wMenuSpotWelding)
                    {
                        case eMenu:
                            wMenuSpotWelding=eRepeatCycle;
                            break;
                        case eSetSpotTime:
                        case eManualMode:
                        case eRepeatCycle:
                            wMenuSpotWelding--;
                            break; 
                        default:
                            wMenuSpotWelding=eMenu;
                            break;
                    }
                    break;
                case eBoltHeating:
                    switch(wMenuHeating)
                    {
                        case eHMenu:
                            wMenuHeating=eHPowerDelevry;
                            break;
                        case eHSetHeatingTime:
                        case eHPowerDelevry:
                            wMenuHeating--;
                            break;
                        default:
                            wMenuHeating=eHMenu;
                            break;
                    }
                    break;
                default:
                    wMenu = eSpotWelding;
                    break;
            }
        }
        else
        {
            switch(wMenu)
            {
                case eSpotWelding:
                    switch(wMenuSpotWelding)
                    {
                        case eMenu:
                            wMenu--;
                            break;
                        case eSetSpotTime:
                            wSpotTime = wSpotTime-75;
                            break;
                        case eManualMode:
                            wPowerDelevry--;
                            break;
                        case eRepeatCycle:
                            wRepeatCycle--;
                            break; 
                        default:
                            break;
                    }
                    break;
                case eBoltHeating:
                    switch(wMenuHeating)
                    {
                        case eHMenu:
                            wMenu--;
                            break;
                        case eHSetHeatingTime:
                            wSpotTime--;
                            break;
                        case eHPowerDelevry:
                            wPowerDelevry--;
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    wMenu = eSpotWelding;
                    break;
            }
        }
   }
   
   if(wEnterBottonPressed == 1)
   {
       wUpdateMenu=1;
       wEnterBottonPressed = 0;

       if(wEditingMode == 0)
       {
         wEditingMode = 1;
       }
       else
       {
         wEditingMode = 0;   
       }
   }   
   if(wCommandOnPressed == 1 )
   {
       setCursorPosition(3,10);
      lcdWriteTextFullLine("On");
      __delay_ms(1000);
      setCursorPosition(3,10);
      lcdWriteTextFullLine("Off");
        wCommandOnPressed = 0;
        switch(wMenu)
        {
            case eSpotWelding:
                switch(wMenuSpotWelding)
                {
                    case eSetSpotTime:
                        SetTimer1AndTimer0(((float)wSpotTime)/10000);
                        WELDOut = 1;
                        
                        break;
                    case eManualMode:
                        break;
                    case eRepeatCycle:
                        break; 
                    default:
                        break;
                }
                break;
            case eBoltHeating:
                
                break;
            default:
                break;
        }
    }
  }
  
   
  return;
}

char wCounter2=0;
void __interrupt() myint(void)
{
    /*static uint8_t wLast=0;
    if(INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;
    }*/
    if(PIR1bits.TMR1IF == 1)
    {
        WELDOut = 0;
        PIR1bits.TMR1IF = 0;
        PIE1bits.TMR1IE = 0; //Disabling timer 1 interrupt
        PIE1bits.TMR1GIE = 0; //disable the gate interrupt to reset the Pulse
        T1CONbits.TMR1ON = 0;
    }
    if(PIR1bits.TMR1GIF == 1)
    {
        PIR1bits.TMR1GIF =0;        
        T1GCONbits.T1GGO_nDONE =1;
    }
    /*
    if(INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;
        wTimer0Counter++;
    }*/
}
