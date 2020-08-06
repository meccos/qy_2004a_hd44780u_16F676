//LCD_hd44780u_qy_2004a.h

#include "stdint.h"

//#define _XTAL_FREQ 16000000 //for __delay_ms it need to know the cpu speed
#define _XTAL_FREQ 4000000 //for __delay_ms it need to know the cpu speed

#define wInterruptTextSize 75
int test =0; 
void initLCD(void);

void setData(char iValue);
void writeInsChk(char iOpCode);
void writeInsNoChk(char iOpCode);
void writeTxtChk(char iOpCode);
void waitLCDBusy(void);
void SetReadDataFromLCD(void);
void SetToSendDataToLCD(void);
void lcdWriteText(char *iText);
void lcdWriteTextFullLine(char *iText);
void lcdWriteAllText(char *iText);
void lcdWriteRotText(char *iRotText, char ioRotReadPtr, char iWritePtr);
void lcdWriteRotaryBuffer(char *iRotText, uint8_t iStarPosition, uint8_t iNumOfChar, uint8_t iBufferSize);
void powerOffLcd();
void powerOnLcd();
void setBlinkingCursor();
void setNotBlinkingCursor();
void setCursorOff();
void setCursorOn();
void setCursorMovingRight();
void setCursorMovingLeft();
void setCursorPosition(char iLine, char iPosition);
void setDisplayMovingRight();
void setDisplayMovingLeft();
void moveCursorRight();
void moveCursorLeft();
void clearDisplay();
void moveCursorToHome();
void delay2us();



char mDisplayOnOffReg;
char mCursorDisplayShiftReg;
char mWritingPosition;

#define DDRAM_Address_Line_0_Position_0 0x00
#define DDRAM_Address_Line_1_Position_0 0x40
#define DDRAM_Address_Line_2_Position_0 0x14
#define DDRAM_Address_Line_3_Position_0 0x54

#define RS PORTCbits.RC2
#define RSDirection TRISCbits.TRISC2
#define RS_AD ANSELCbits.ANSC2
#define RW PORTCbits.RC1
#define RWDirection TRISCbits.TRISC1
#define RW_AD ANSELCbits.ANSC1
#define E PORTCbits.RC0
#define EDirection TRISCbits.TRISC0
#define E_AD ANSELCbits.ANSC0
#define DB7 PORTAbits.RA0
#define DB7Direction TRISAbits.TRISA0
#define DB7_AD ANSELAbits.ANSA0
#define DB6 PORTAbits.RA1
#define DB6Direction TRISAbits.TRISA1
#define DB6_AD ANSELAbits.ANSA1
#define DB5 PORTAbits.RA2
#define DB5Direction TRISAbits.TRISA2
#define DB5_AD ANSELAbits.ANSA2
#define DB4 PORTAbits.RA3
#define DB4Direction TRISAbits.TRISA3
#define DB4_AD ANSELAbits.ANSA3

#define FROM_LCD 1
#define TO_LCD 0;

#define RW_WRITE 0
#define RW_READ 1
#define RS_INSTRUCTION 0
#define RS_DATA_REGISTER 1
