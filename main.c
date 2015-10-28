
#include "misc.h"
#include "stm32l1xx.h"
#include "stm32l1xx_rcc.h"
#include "stm32l1xx_gpio.h"
#include "stdbool.h"
#include "ASCII_characters.h"
#include "stdio.h"
#include "CodersCrux_22px_varWidth.h"

//define used pins

#define SCE GPIO_Pin_8 	//chip enable active LOW
#define RES GPIO_Pin_9	//reset active LOW
#define DC GPIO_Pin_10	//data / command(LOW)
#define SCLK GPIO_Pin_11//serial clock
#define SDIN GPIO_Pin_12//serial data

#define fSet 			0x20	//function Set
#define dControl 	0x08	//display Control
#define yAddress 	0x40	//y Address register 3 bits
#define xAddress 	0x80	//x Address register 7 bits
#define tControl 	0x04	//temperature control
#define setBias 	0x10	//set bias system	(1:48 standard)
#define Vop 			0x80	//set voltage of operation (CONTRAST, 0xB1 good @3.3V)
#define LCD_X 84
#define LCD_Y 48

char LCDbuff[504];
float value=12.024;
int count = 1;
int count2=10;

void writeLCD(bool data_command, char data){
/*
 * data_command = 0-command to lcd 1-data to lcd
 */
int x = 0;
	if(data_command == 1){		 
	        GPIO_SetBits(GPIOA,DC); //set DC pin for Data
	    }
	    else {
	        GPIO_ResetBits(GPIOA, DC); //set DC pin for command
	    }

	    GPIO_ResetBits(GPIOA, SCE);
	   
	    for(x=0; x<8; x++){
	        if(data & 0x80){
	            GPIO_SetBits(GPIOA,SDIN);    //set SDIN high

	        }
	        else {
	            GPIO_ResetBits(GPIOA,SDIN);    //set SDIN low

	        }
	     //pulse clock line
	        GPIO_SetBits(GPIOA,SCLK);
	        //delay
	        GPIO_ResetBits(GPIOA,SCLK);
	        data= data<<1;
	        }

	     GPIO_SetBits(GPIOA, SCE);
}

void funcSet(bool cState, bool addressing, bool insSet){
/*
 * cState = chip state 0-active 1-power-down
 * addressing = 0-horizontal addressing 1-vertical addressing
 * insSet = 0-basic instruction set 1-extended instruction set
 */
 
	int temp = fSet;
    if(cState == 1){
        temp |= 0x04;
    }
    else {
        temp &= ~0x04;
    }
    if(addressing == 1){
        temp |= 0x02;
    }
    else{
        temp &= ~0x02;
    }
    if(insSet == 1){
        temp |= 0x01;
    }
    else{
        temp &= ~0x01;
    }
    writeLCD(0, temp);
}


void setXY(char x, char y){
/*
 * x = x location of cursor according to pixel 0-83
 * y = y location of cursor according to bank 0-5
 */
writeLCD(0, xAddress | x);    //set cursor to x address, 0-83
	writeLCD(0, (yAddress | y));    //set cursor to y address, 0-5
}

void dispContr(bool D, bool E){
/*
 * D E
 * 0 0    display blank
 * 1 0    normal mode
 * 0 1    all display segments on
 * 1 1     inverse video mode
 */
 int temp = dControl;
    if(D == 1){
        temp |= 0x04;
    }
    else{
        temp &= ~0x04;
    }
    if(E == 1){
        temp |= 0x01;
    }
    else{
        temp &= ~0x01;
    }
    writeLCD(0, temp);
}

void tempCoef(bool coef1, bool coef0){
 /*
 * coef1 coef0
 *   0      0 Vlcd temp coef 0
 *   0      1 Vlcd temp coef 1
 *   1      0 Vlcd temp coef 2
 *   1      1 Vlcd temp coef 3
 */
int temp = tControl;
    if(coef1 == 1){
        temp |= 0x02;
    }
    else{
        temp &= ~0x02;
    }
    if(coef0 == 1){
        temp |= 0x01;
    }
    else{
        temp &= ~0x01;
    }
    writeLCD(0, temp);
}

void biasSys(char bias){
 /*
  *set bias value, default used, 0x13, 1:48
  */
  int temp = setBias;
    temp |= bias;
    writeLCD(0,temp);
}


void operVolt(char volt){
/*
 *Vop0-Vop6 set the operation voltage of the LCD, sets LCD contrast, default used 0xB1
 */
 int temp = Vop;
    temp |= volt;
    writeLCD(0, temp);
}

void Delay(unsigned int time){
	while(time--){}
}
void writeChar(char character){
	int index;
	for (index = 0 ; index < 5 ; index++)
	    writeLCD(1, ASCII[character - 0x20][index]);
	    //0x20 is the ASCII character for Space (' '). The font table starts with this character

	  writeLCD(1, 0x00); //Blank vertical line padding
}
void LCDString(char * characters){
	while (*characters){
	    writeChar(*characters++);}
}

void LCDClear(void){
	int index;
	for (index = 0 ; index < (LCD_X * LCD_Y / 8) ; index++)
	    writeLCD(1, 0x00);

	  setXY(0, 0); //After we clear the display, return to the home position
}

void writeCharInv(char character){
	int index;
	for (index = 0 ; index < 5 ; index++)
	    writeLCD(1, ~ASCII[character - 0x20][index]);
	    //0x20 is the ASCII character for Space (' '). The font table starts with this character

	  writeLCD(1, ~0x00); //Blank vertical line padding
}
void newFont(char character, int x, int y){
	char index=0;
	int offset=0;
	int length=0;
	setXY(x, y-1);
	offset = codersCrux_22ptDescriptors[character-0x2c][1];
	length = codersCrux_22ptDescriptors[character-0x2c][0] * 2;
	
	for (index = 1 ; index < length ; index += 2)
		    writeLCD(1, codersCrux_22ptBitmaps[offset+index]);
	
	
	setXY(x, y);
	for (index = 0 ; index < length ; index += 2)
		   writeLCD(1, codersCrux_22ptBitmaps[offset+index]);
	/*
	int index;
	setXY(x, y-1);
	for (index = 1 ; index < 29 ; index += 2)
		    writeLCD(1, codersCrux_22pt[character - 0x2b][index]);
		    //0x20 is the ASCII character for Space (' '). The font table starts with this character
	setXY(x, y);
	for (index = 0 ; index < 30 ; index += 2)
		   writeLCD(1, codersCrux_22pt[character - 0x2b][index]);
	//writeLCD(1, 0x00); //Blank vertical line padding*/
}
void newString(char * characters, int x, int y){
	int cnt=0;
	while (*characters){
		    newFont(*characters, x, y);
		x = x+codersCrux_22ptDescriptors[*characters-0x2c][0]+1;
		if(cnt>6){y+=2;x=0; cnt=0;}
		*characters++;
	cnt++;
	}
}

void floatToString(float n){
	char str[8]={0};
	sprintf (str, "%2.4f", n);
	newString(str, 0,5);
}

void LCDinit(void){
	int index; 
	//pulse reset line for initializing
	GPIO_ResetBits(GPIOA, RES);
	Delay(100);
    GPIO_SetBits(GPIOA, RES);
    //hold reset line high
    //GPIO_SetBits(GPIOA, RES);
    GPIO_SetBits(GPIOA, SCE);
    //Delay(500);
    funcSet(0,0,1); //enter extended mode
    operVolt(0xBB);
    tempCoef(0,0);
    biasSys(0x14);
    funcSet(0,0,0); //enter basic mode
    dispContr(1,0); //normal mode
   // delay(5000);
    //dispContr(0,1); // all segments on for testing
    LCDClear();
			
	for(index=0; index<504; index++){LCDbuff[index]=0;}

}


void drawLine(int x1,int y1,int x2,int y2, int thikness){
	
}

int main(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); //enable gpioB peripherial clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	//enable gpioA peripherial clock

	GPIO_InitStructure.GPIO_Pin = SCE|RES|DC|SCLK|SDIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA, SCE|RES|DC|SCLK|SDIN);

	LCDinit(); //initialize LCD parameters


    while(count--)
    {
    	LCDString("Hello worlds! ");
    }
    setXY(0,5);
    //while(count2--){

    	//writeCharInv('C');
   // }

    newString("0.123457",0,2);
	
		
    floatToString(value-count2);
		newFont('V',55,5);

		/*To Do:
			
		*/
		
    return 0;
	
}
