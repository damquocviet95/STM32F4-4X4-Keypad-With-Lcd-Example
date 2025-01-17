#include <stdint.h>
#include "lcd.h"
#include "stm32f4xx_hal.h"
/*
  size is 1*16
  if do not need to read busy, then you can tie R/W=ground
  ground = pin 1    Vss
  power  = pin 2    Vdd   +3.3V or +5V depending on the device
  ground = pin 3    Vlc   grounded for highest contrast
  PE1    = pin 4    RS    (1 for data, 0 for control/status)
  ground = pin 5    R/W   (1 for read, 0 for write)
  PE0    = pin 6    E     (enable)
  -    	 = pin 7    DB0   (8-bit data)
  -	     = pin 8    DB1
  -   	 = pin 9    DB2
  -      = pin 10   DB3
  PD0    = pin 11   DB4
  PD1    = pin 12   DB5
  PD2    = pin 13   DB6
  PD3    = pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/
#define LCDDATA      GPIOD->ODR  /* Port 7 Output */
#define LCD_RS_ON    HAL_GPIO_WritePin(GPIOE,LCD_RS_Pin,1) /* Port 9.6 Output */
#define LCD_RS_OFF   HAL_GPIO_WritePin(GPIOE,LCD_RS_Pin,0)   /* Port 9.6 Output */
#define LCD_E_ON     HAL_GPIO_WritePin(GPIOE,LCD_E_Pin,1)  /* Port 9.7 Output */
#define LCD_E_OFF    HAL_GPIO_WritePin(GPIOE,LCD_E_Pin,0)   /* Port 9.7 Output */
#define LCD_Delay    HAL_Delay

void static OutCmd(unsigned char command){
  LCDDATA = command>>4;//HIGH 4 BIT 

    //LCD_RS_OFF;                    // E=0, R/W=0, RS=0
	LCD_RS_OFF;
    LCD_Delay(1);
    //LCD_E = 1;                     // E=1, R/W=0, RS=0
	LCD_E_ON;
    LCD_Delay(1);
    // LCD_E_OFF;                     // E=0, R/W=0, RS=0
	LCD_E_OFF;
    LCD_Delay(2);
	
	LCDDATA = command&0x0F; //LOW 4 BIT
 
    LCD_RS_OFF;                    // E=0, R/W=0, RS=0
    LCD_Delay(1);
    LCD_E_ON;                     // E=1, R/W=0, RS=0
    LCD_Delay(1);
    LCD_E_OFF;                     // E=0, R/W=0, RS=0
           // E=0, R/W=0, RS=0
          // wait 40us
}

// Initialize LCD
// Inputs: none
// Outputs: none


void LCD_Init(void){
 
  LCD_E_OFF;
  LCD_RS_OFF;                    // E=0, R/W=0, RS=0
  LCD_Delay(5);                // Wait >15 ms after power is applied
  OutCmd(0x03);               // command 0x30 = Wake up
  LCD_Delay(5);              // must wait 5ms, busy flag not available
  OutCmd(0x03);             // command 0x30 = Wake up #2
  LCD_Delay(5);             // must wait 160us, busy flag not available
  OutCmd(0x03);            // command 0x30 = Wake up #3
  LCD_Delay(5);            // must wait 160us, busy flag not available
  OutCmd(0x02);                 
  OutCmd(0x28); 			      // Function set: 4-bit/2-line
  OutCmd(0x08);					 // Display off, cursor off
  OutCmd(0x0F);                  // Display on, cursor blinking
  OutCmd(0x06);                  // Increment cursor (shift cursor to right)
  OutCmd(0x80);                  // Increment cursor (shift cursor to right)
	
 OutCmd(0x01);					//Clear display screen
}

// Output a character to the LCD
// Inputs: letter is ASCII character, 0 to 0x7F
// Outputs: none
void LCD_OutChar(char letter){
  LCDDATA = letter>>4;
 
 LCD_RS_ON;                    // E=0, R/W=0, RS=1
 LCD_Delay(1);           // wait 6us
 LCD_E_ON;                     // E=1, R/W=0, RS=1
 LCD_Delay(1);            // wait 6us
 LCD_E_OFF;                     // E=0, R/W=0, RS=1
 LCD_Delay(2);           // wait 40us
	
 LCDDATA = letter&0x0F;
 
 LCD_RS_ON;                    // E=0, R/W=0, RS=1
 LCD_Delay(1);           // wait 6us
 LCD_E_ON;                     // E=1, R/W=0, RS=1
 LCD_Delay(1);            // wait 6us
 LCD_E_OFF;                     // E=0, R/W=0, RS=1
 LCD_Delay(2);           // wait 40us
}

// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear(void){
  OutCmd(0x01);          // Clear Display
  LCD_Delay(2); // wait 1.6ms
  OutCmd(0x02);          // Cursor to home
  LCD_Delay(2); // wait 1.6ms
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString(char *pt,unsigned char line){

	
	if(line==2){//move second line
     OutCmd(0xc0); 
	}
	else if(line==1){//move first line
	 OutCmd(0x80); 
	}
	else//must be 1 or 2 
	 return;//fail 
	
  while(*pt){
    LCD_OutChar(*pt);
    pt++;
  }
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    LCD_OutUDec(n/10);
    n = n%10;
  }
  LCD_OutChar(n+'0'); /* n is between 0 and 9 */
}

//--------------------------LCD_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    LCD_OutUHex(number/0x10);
    LCD_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      LCD_OutChar(number+'0');
     }
    else{
      LCD_OutChar((number-0x0A)+'A');
    }
  }
}

void lcd_setcursor(void)
{

OutCmd(0x38);

}
