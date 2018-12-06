//LCD_hd44780u_qy_2004a.h

void initLCD(void);


void FlashLed(int iNumberofTime);
void setData(char iValue);
void writeInsChk(char iOpCode);
void writeInsNoChk(char iOpCode);
void writeTxtChk(char iOpCode);
void waitLCDBusy(void);
void SetReadDataFromLCD(void);
void SetToSendDataToLCD(void);
void lcdWriteText(char *iText);
void powerOffLcd();
void powerOnLcd();
void setBlinkingCursor();
void setNotBlinkingCursor();
void setCursorOff();
void setCursorOn();
void setCursorMovingRight();
void setCursorMovingLeft();
void setCursorPosition(char iPosition);
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

#define DDRAM_Address_Line_1_Position_1 0x00
#define DDRAM_Address_Line_2_Position_1 0x40
#define DDRAM_Address_Line_3_Position_1 0x14
#define DDRAM_Address_Line_4_Position_1 0x54

#define RS PORTAbits.RA4
#define RW PORTCbits.RC5
#define E PORTCbits.RC4
#define DB7 PORTCbits.RC3
#define DB7Direction TRISCbits.TRISC3
#define DB6 PORTCbits.RC2
#define DB6Direction TRISCbits.TRISC2
#define DB5 PORTCbits.RC1
#define DB5Direction TRISCbits.TRISC1
#define DB4 PORTCbits.RC0
#define DB4Direction TRISCbits.TRISC0

#define FROM_LCD 1;
#define TO_LCD 0;

#define RW_WRITE 0
#define RW_READ 1
#define RS_INSTRUCTION 0
#define RS_DATA_REGISTER 1
