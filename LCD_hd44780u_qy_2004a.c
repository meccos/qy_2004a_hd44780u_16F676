//LCD_hd44780u_qy_2004a.c

#include "LCD_hd44780u_qy_2004a.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <xc.h>


void initLCD()
{
    //Setting pin to digitals
  DB7_AD = 0;
  DB6_AD = 0;
  DB5_AD = 0;
  DB4_AD = 0;
  RS_AD = 0;
  RW_AD = 0;
  E_AD = 0;
  //Setting initial value
  DB7 = 0;
  DB6 = 0;
  DB5 = 0;
  DB4 = 0;
  RS=0;
  RW=0;
  E=0;
  //Set initial pin direction to ouput
  RSDirection = 0;
  RWDirection = 0;
  EDirection = 0;
  DB7Direction = 0;
  DB6Direction = 0;
  DB5Direction = 0;
  DB4Direction = 0;

  
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
void lcdWriteTextFullLine(char *iText)
{
    uint8_t wTxtLen;
    wTxtLen = strlen(iText);
    lcdWriteText(iText);
    char wBlank[20];
    memset(wBlank,' ',20-wTxtLen);
    wBlank[wTxtLen] = '\0';
    lcdWriteText(wBlank);
}
void lcdWriteText(char *iText)
 {
  unsigned char wCharReadingPos = 0;
  uint8_t wBrakeInfiniteLoop=0;
  while( iText[wCharReadingPos] != 0 && wBrakeInfiniteLoop !=255)
  {
    wBrakeInfiniteLoop++;
    writeTxtChk(iText[wCharReadingPos]);
    wCharReadingPos++;
  }
    
}

void lcdWriteAllText(char *iText)
 {
  unsigned char wCharReadingPos = 0;
  uint8_t wBrakeInfiniteLoop=0;
  while( iText[wCharReadingPos] != 0 && wBrakeInfiniteLoop !=255)
  {
    wBrakeInfiniteLoop++;
    switch(iText[wCharReadingPos])
    {
        case '\r':
            writeTxtChk('/');
            writeTxtChk('r');
            break;
        case '\n':
            writeTxtChk('/');
            writeTxtChk('n');
            break;
        default:
            writeTxtChk(iText[wCharReadingPos]);
            break;
    }
    wCharReadingPos++;
  }
    
}
void lcdWriteRotaryBuffer(char *iRotText, uint8_t iStarPosition, uint8_t iNumOfChar, uint8_t iBufferSize)
 {
    uint8_t wReadPosition = iStarPosition%iBufferSize;

    for(uint8_t i=0; i<iNumOfChar ; i++)
    {

      switch(iRotText[wReadPosition])
      {
          case '\r':
              writeTxtChk('/');
              writeTxtChk('r');
              break;
          case '\n':
              writeTxtChk('/');
              writeTxtChk('n');
              break;
          default:
              writeTxtChk(iRotText[wReadPosition]);
              break;
      }
      wReadPosition++;
      if(wReadPosition == iBufferSize)
      {
          iBufferSize = 0;
      }
    }
    
}

void lcdWriteRotText(char *iRotText, char ioRotReadPtr, char iWritePtr)
{
    uint8_t wBrakeInfiniteLoop =0;
    while((ioRotReadPtr < iWritePtr || (ioRotReadPtr < wInterruptTextSize && ioRotReadPtr > iWritePtr)) && wBrakeInfiniteLoop !=255 )
    {
        wBrakeInfiniteLoop++;
        writeTxtChk(iRotText[ioRotReadPtr]);
        ioRotReadPtr++;
        if(ioRotReadPtr == wInterruptTextSize)
        {
            ioRotReadPtr = 0;
        }
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
    if(iOpCode == '\r')
    {
      return;   
    }
  SetToSendDataToLCD();
  RS = RS_DATA_REGISTER;
  RW = RW_WRITE;
  if(iOpCode == '\n')
  {
   if(mWritingPosition < 20)
   {
       setCursorPosition(1,0);
   }
   else if(mWritingPosition < 40)
   {
       setCursorPosition(2,0);
   }
   else if(mWritingPosition < 60)
   {
       setCursorPosition(3,0);
   }
   else
   {
       setCursorPosition(0,0);
   }
   return;
  }
  switch(mWritingPosition)
  {
    case 20:
      setCursorPosition(1,0);
      break;
    case 40:
      setCursorPosition(2,0);
      break;
    case 60:
      setCursorPosition(3,0);
      break;
    case 80:
      setCursorPosition(0,0);
      mWritingPosition = 0;
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
  uint8_t wTimeout=0;

  SetToReadDataFromLCD();
  
  int busyFlag = 1;
  while(busyFlag == 1 && wTimeout != 255)
  {
    wTimeout++;
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
void setCursorPosition(char iLine, char iPosition)
{
  char wLCDIndex=0;
  
  switch(iLine)
  {
      case 0:
          wLCDIndex = DDRAM_Address_Line_0_Position_0 + iPosition;
          mWritingPosition = iPosition;
          break;
      case 1:
          wLCDIndex = DDRAM_Address_Line_1_Position_0 + iPosition;
          mWritingPosition = 20 + iPosition;
          break;
      case 2:
          wLCDIndex = DDRAM_Address_Line_2_Position_0 + iPosition;
          mWritingPosition = 40 + iPosition;
          break;
      case 3:
          wLCDIndex = DDRAM_Address_Line_3_Position_0 + iPosition;
          mWritingPosition = 60 + iPosition;
          break;
      default:
      break;
  }
  
  waitLCDBusy();
  
  SetToSendDataToLCD();
  RS = RS_INSTRUCTION;
  RW = RW_WRITE;
  setData((wLCDIndex >> 4) | 0x8 );
  setData(wLCDIndex);
  
}