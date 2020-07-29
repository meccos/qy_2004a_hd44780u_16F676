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



#define ENTERBotton PORTBbits.RB0
#define UPBotton    PORTBbits.RB1
#define DOWNBotton  PORTBbits.RB2
#define COMMANDOn PORTBbits.RB3

#define DISPTACHERSIGNAL T1CONbits.TMR1CS

uint8_t wTimer1IntCounter=0;
uint8_t wTimer0Counter=0;
uint8_t wMenuSpotWelding=0;
uint8_t wMenuHeating=0;

int16_t wHumidity=0;
int16_t wTemperature=0;

int16_t wTempSet=210;

enum eMenuSpotWelding{eMenu=0, eSetSpotTime, ePowerDelevry, eRepeatCycle};
enum eMenuHeating{eHMenu=0, eHSetHeatingTime, eHPowerDelevry};

uint8_t wSpotTime=16;
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
  uint8_t wCommandOn=0;
  
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
  INTCONbits.TMR0IE = 0; //Enable Timer 0 Interrupt
  
  
  //Button configuration
  PORTB = 0x00;
  ANSELB = 0x00; //Setting All to digital
  TRISB = 0x0F; //Setting bit 0 1 2 3 to input
  WPUB = 0x0F; // Activation of weak pull up
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
              case ePowerDelevry:
                  setCursorPosition(0,0);
                  lcdWriteText("PowerDelevry           ");
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
   Debounce(COMMANDOn,&wDebounceCommandOn,&wCommandOn);
   

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
                        case ePowerDelevry:
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
                            wSpotTime++;
                            break;
                        case ePowerDelevry:
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
                        case ePowerDelevry:
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
                            wSpotTime--;
                            break;
                        case ePowerDelevry:
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
  }
  
   if(wCommandOn == 1 )
   {
        wCommandOn = 0;
        switch(wMenu)
        {
            case eSpotWelding:
                switch(wMenuSpotWelding)
                {
                    case eSetSpotTime:
                        OPTION_REGbits.PSA=1; //Activation of the presacler on timer0
                        OPTION_REGbits.PS0=1; //  1:256 prescaler
                        OPTION_REGbits.PS1=1;
                        OPTION_REGbits.PS2=1;
                        TMR0=0;
                        INTCONbits.TMR0IF=0;
                        INTCONbits.TMR0IE=1;
                        break;
                    case ePowerDelevry:
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
  return;
}

char wCounter2=0;
void __interrupt() myint(void)
{
    if(INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;
        INTCONbits.TMR0IE=1;
        switch(wMenu)
        {
            case eSpotWelding:
                switch(wMenuSpotWelding)
                {
                    case eSetSpotTime:
                    case ePowerDelevry:
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
    if(PIR1bits.TMR1IF == 1)
    {
        wTimer1IntCounter++;
        PIR1bits.TMR1IF = 0;
        
        if(wTimer1IntCounter == 7)
        {
            TMR1H = 0x4C;
            TMR1L = 0x83;
        }
        if(wTimer1IntCounter == 8)
        {
            wTimer1IntCounter = 0;
        }
    }
    if(INTCONbits.TMR0IF == 1)
    {
        INTCONbits.TMR0IF = 0;
        wTimer0Counter++;
    }
}
