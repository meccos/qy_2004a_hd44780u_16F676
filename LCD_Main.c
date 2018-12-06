



#include <xc.h>

#define _XTAL_FREQ 4000000 //for __delay_ms it need to know the cpu speed

#include "LCD_hd44780u_qy_2004a.h"

//FOSC = 0x4 setup as internal oscillator
//WDTE = 0x01 watch dog timer enable
//PWRTE = 0x1 Power up timer, will wait 72 ms before executing the code (waiting to stabilise the power)) 1 mean disable
//MCLRE = 0x1  Disable master clear
//BOREN = 0x1 Brown out enable bit detect power decreasing and will reboot the sotware
// CP CPD is code protection, if set to 1 code protection is disable.



#pragma config FOSC = 0x4, WDTE = 0x0, PWRTE = 0x1, MCLRE = 0x0, BOREN = 0x0, CP = 0x1, CPD = 0x1



void main(void)
{
	
  INTCONbits.GIE = 0; //Disable interrupt
  PORTA = 0x00;
  PORTC = 0x00;
  CMCONbits.CM0 = 1; //No comparator all set to digital
  CMCONbits.CM1 = 1;
  CMCONbits.CM2 = 1;
  ANSEL = 0x00; //Setting All to digital
  TRISA = 0x00; //Setting the portA as output
  TRISC = 0x00; //Setting the portA as output
  int flash=1;

  initLCD();
  int wTestCase = 1;
  while(1)
  {
    clearDisplay();
    __delay_ms(1000);
    powerOnLcd();
    __delay_ms(1000);
    setCursorOn();
    __delay_ms(1000);
    moveCursorToHome();
    __delay_ms(1000);
    setBlinkingCursor();
    __delay_ms(1000);

    switch(wTestCase)
    {
      case 1:
        lcdWriteText("Test 1: setCursorON()");
        //setCursorOn();
      break;
      case 2:
        lcdWriteText("Test 2: enable Cursor");
        //setCursorOn();
        //setBlinkingCursor();
      break;
      default:
       wTestCase = 0;
       lcdWriteText("Program Completed");
       FlashLed(3);
       break;
    }

      wTestCase++;
      __delay_ms(5000);

  }


}

