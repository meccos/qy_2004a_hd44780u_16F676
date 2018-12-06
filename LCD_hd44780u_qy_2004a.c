//LCD_hd44780u_qy_2004a.c

#include "LCD_hd44780u_qy_2004a.h"
#include <xc.h>

#define _XTAL_FREQ 4000000 //for __delay_ms it need to know the cpu speed

void FlashLed(int iNumberofTime)
{
  int i =0;
  __delay_ms(1000); //Need the definition of _XTAL_FREQ
  iNumberofTime = iNumberofTime << 1;
  for(i=0;i<iNumberofTime;i++)
  {
    if(PORTAbits.RA0 == 1)
    {
      PORTAbits.RA0 = 0;
    }
    else
    {
       PORTAbits.RA0 = 1;  
    }
    __delay_ms(200); //Need the definition of _XTAL_FREQ
   }
}

void initLCD()
{
  //Set all the control pins to logic Zero
  RS = RS_INSTRUCTION;
  RW = RW_WRITE;
  E = 0;
  
  //Do the wake up call
  __delay_ms(15);
  setData(0x3);
  __delay_ms(5);
  setData(0x3);
  __delay_us(150);
  setData(0x3);
  __delay_us(10);
  setData(0x2);
  __delay_us(10);
  writeInsChk(0x28);  //4 Bits, N=1, 5x7Ddot
  mDisplayOnOffReg = 0x0F; //Display ON.
  mCursorDisplayShiftReg= 0x06;
  writeInsChk(mDisplayOnOffReg);

  clearDisplay();
  writeInsChk(0x06); //Entry mode set - I/D = 1 (increment cursor) & S = 0 (no shift) 
  mWritingPosition=1;

}

void lcdWriteText(char *iText)
 {
       
 	while( *iText)
  {
    writeTxtChk(*iText++);
  }
}

void setData(char iValue)
{
  E = 1;
  DB7 = (iValue & 0x8) >> 3;
  DB6 = (iValue & 0x4) >> 2;
  DB5 = (iValue & 0x2) >> 1;
  DB4 = (iValue & 0x1) ;
  __delay_us(1);            //to meet Tdsw + Tpw
  E = 0;
  __delay_us(1);
}

void writeTxtChk(char iOpCode)
{
  switch(mWritingPosition)
  {
    case 21:
      setCursorPosition(DDRAM_Address_Line_2_Position_1);
      break;
    case 41:
      setCursorPosition(DDRAM_Address_Line_3_Position_1);
      break;
    case 61:
      setCursorPosition(DDRAM_Address_Line_4_Position_1);
      break;
    case 81:
      setCursorPosition(DDRAM_Address_Line_1_Position_1);
      break;
    default:
      break;
  }
  waitLCDBusy();

  SetToSendDataToLCD();
  RS = RS_DATA_REGISTER;
  RW = RW_WRITE;
  setData(iOpCode >> 4);
  setData(iOpCode);
  __delay_us(1);            //to meet Tc + Th
  mWritingPosition++;
}

void writeInsChk(char iOpCode)
{
  waitLCDBusy();
  writeInsNoChk(iOpCode);
}

void writeInsNoChk(char iOpCode)
{
  SetToSendDataToLCD();
  RS = RS_INSTRUCTION;
  RW = RW_WRITE;
  setData(iOpCode >> 4);
  setData(iOpCode);
}

void SetToReadDataFromLCD()
{
  DB7Direction = FROM_LCD;
  DB6Direction = FROM_LCD;
  DB5Direction = FROM_LCD;
  DB4Direction = FROM_LCD;
}
void SetToSendDataToLCD()
{
  DB7Direction = TO_LCD;
  DB6Direction = TO_LCD;
  DB5Direction = TO_LCD;
  DB4Direction = TO_LCD;
}

void waitLCDBusy()
{
  RS = RS_INSTRUCTION;
  RW = RW_READ;
  SetToReadDataFromLCD();
  
  int busyFlag = 1;
  while(busyFlag == 1)
  {
    //The data should be read while Enable pin is HIGH
    E = 1; 
    __delay_us(1);            //to meet Tddr + Tpw
    busyFlag = DB7;
    E = 0;                 
    __delay_us(2);            //to meet Tc
    // Request the next 4 bit even if we dont need them
    E = 1;
    __delay_us(1);            //to meet Tddr + Tpw
  }
}
void powerOffLcd()
{
  mDisplayOnOffReg = mDisplayOnOffReg & 0xFB;
  writeInsChk(mDisplayOnOffReg);
}
void powerOnLcd()
{
  mDisplayOnOffReg = mDisplayOnOffReg | 0x04;
  writeInsChk(mDisplayOnOffReg);  
}
void setBlinkingCursor()
{
  mDisplayOnOffReg = mDisplayOnOffReg | 0x01;
  writeInsChk(mDisplayOnOffReg);  
}
void setNotBlinkingCursor()
{
  mDisplayOnOffReg = mDisplayOnOffReg & 0xFE;
  writeInsChk(mDisplayOnOffReg);  
}
void setCursorOff()
{
  mDisplayOnOffReg = mDisplayOnOffReg & 0xFD;
  writeInsChk(mDisplayOnOffReg);  
}
void setCursorOn()
{
  mDisplayOnOffReg = mDisplayOnOffReg | 0x02;
  writeInsChk(mDisplayOnOffReg);  
}

void setCursorMovingLeft()
{
  mCursorDisplayShiftReg = 0x10;
  writeInsChk(mCursorDisplayShiftReg);  
}
void setCursorMovingRight()
{
  mCursorDisplayShiftReg = 0x14;
  writeInsChk(mCursorDisplayShiftReg);  
}
void setDisplayMovingRight()
{
  mCursorDisplayShiftReg = 0x18;
  writeInsChk(mCursorDisplayShiftReg); 
}
void setDisplayMovingLeft()
{
  mCursorDisplayShiftReg = 0x1C;
  writeInsChk(mCursorDisplayShiftReg); 
}

void moveCursorRight()
{
  mCursorDisplayShiftReg = mCursorDisplayShiftReg | 0x04;
  writeInsChk(mCursorDisplayShiftReg);  
}
void moveCursorLeft()
{
  mCursorDisplayShiftReg = mCursorDisplayShiftReg | 0x04;
  writeInsChk(mCursorDisplayShiftReg);  
}
void clearDisplay()
{
  writeInsChk(0x01);
}
void moveCursorToHome()
{
  writeInsChk(0x02);
  mWritingPosition=1;
}
void setCursorPosition(char iPosition)
{
  waitLCDBusy();
  
  SetToSendDataToLCD();
  RS = RS_INSTRUCTION;
  RW = RW_WRITE;
  setData((iPosition >> 4) | 0x8 );
  setData(iPosition);
}