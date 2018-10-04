/*
 * Ten chuong trinh: Snake_Lab3
 * Mo ta: Su dung keypad dieu khien ran chay tren led ma tran
 * Tac gia: Phan Nguyen Vu
 * Ngay viet: 4/10/2015
 */

/*
 * So do noi chan keypad:
 * 		- hang 1 noi PC4
 * 		- hang 2 noi PC5
 * 		- hang 3 noi PC6
 * 		- hang 4 noi PC7
 *
 * 		- Cot 1 noi PA2
 * 		- Cot 2 noi PA3
 * 		- Cot 3 noi PA4
 * 		- Cot 4 noi PA5
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"

#define dataIn 	GPIO_PIN_0	// DIN  	- Noi voi chan PB0
#define load 	GPIO_PIN_1	// LOAD(CS) - Noi voi chan PD1
#define clock 	GPIO_PIN_2	// CLK 		- Noi voi chan PE2

// Dinh nghia huong di cua diem
#define up 0
#define right 1
#define down 2
#define left 3

// Khoi tao mang doc keypad
unsigned char readArray[4] = {224, 208, 176, 112};

// Thiet lap he truc toa do cho led ma tran
unsigned char mangx[8] = {1, 2, 3, 4, 5, 6, 7, 8};
unsigned char mangy[8] = {1, 2, 4, 8, 16, 32, 64, 128};

// Tao bien lap delay ran
volatile uint32_t ui32Loop;
volatile uint32_t ui32LoopIn;

// Toa do dau cua fruit
int fruitx = 5;
int fruity = 6;

// Chieu dai ran
volatile uint8_t ui8Length = 3;

// Toa do than ran
int snakex[25] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int snakey[25] = {2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Co kiem tra cham
int flag = 0;

// Tao bien dem nguoc chuan bi
int begin = 1;

// Tao bien bat dau
int start = 0;

// Bien dem
int i = 0;
// Trang thai mac dinh nut nhan la down
volatile uint8_t ui8Status = down;
volatile uint8_t ui8Speed = 1;

// Tao mang luu anh
unsigned char savePicture[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

/*
 * Dinh nghia cac thanh ghi cua MAX7219.
 * Xem them datasheet: Table 2 Register Address Map.
 */

#define max7219_reg_decodeMode  0x09
#define max7219_reg_intensity   0x0a
#define max7219_reg_scanLimit   0x0b
#define max7219_reg_shutdown    0x0c
#define max7219_reg_displayTest 0x0f

int maxInUse = 4;	// Thong so khai bao so luong MAX7219 duoc su dung
int e = 0;			// Thong so ban dau

/*
 * Tao thu vien ki tu
 * Mo ta: Gom mangChu[][8] va mangSo[][8]
 */
unsigned int mangChu[][8] = {
		{0,0xFE,0xFF,0x11,0x11,0xFF,0xFE,0},	// A
		{0,0xFF,0xFF,0x89,0x89,0xFF,0x76,0},	// B
		{0,0x7E,0xFF,0x81,0x81,0x81,0x42,0},	// C
		{0,0xFF,0xFF,0x81,0x81,0xFF,0x7E,0},	// D
		{0,0xFF,0xFF,0x89,0x89,0x89,0x81,0},	// E
		{0,0xFF,0xFF,0x09,0x09,0x01,0x01,0},	// F
		{0,0x7E,0xFF,0x81,0x91,0xF7,0x76,0},	// G
		{0,0xFF,0xFF,0x08,0x08,0xFF,0xFF,0},	// H
		{0,0x81,0x81,0xFF,0xFF,0x81,0x81,0},	// I
		{0,0x60,0xE0,0x81,0xFF,0x7F,0x01,0},	// J
		{0,0xFF,0xFF,0x1C,0x36,0xE3,0xC1,0},	// K
		{0,0xFF,0xFF,0x80,0x80,0x80,0x80,0},	// L
		{0,0xFF,0xFE,0x04,0x04,0xFE,0xFF,0},	// M
		{0,0xFF,0xFE,0x08,0x10,0x7F,0xFF,0},	// N
		{0,0x7E,0xFF,0x81,0x81,0xFF,0x7E,0},	// 0
		{0,0x80,0xFF,0xFF,0x91,0x1F,0x0E,0},	// P
		{0,0x7E,0xFF,0x81,0x7F,0xFE,0x80,0},	// Q
		{0,0xFF,0xFF,0x39,0x69,0xCF,0x86,0},	// R
		{0,0x46,0x8F,0x89,0x89,0xF9,0x72,0},	// S
		{0,0x01,0x01,0xFF,0xFF,0x01,0x01,0},	// T
		{0,0x7F,0xFF,0x80,0x80,0xFF,0x7F,0},	// U
		{0,0x3F,0x7F,0x80,0x80,0x7F,0x3F,0},	// V
		{0,0xFF,0x7F,0x20,0x20,0x7F,0xFF,0},	// W
		{0,0xC3,0xE7,0x18,0x18,0xE7,0xC3,0},	// X
		{0,0x07,0x0F,0xF0,0xF0,0x0F,0x07,0},	// Y
		{0,0xE1,0xF1,0x99,0x8D,0x87,0x83,0}		// Z
};

unsigned int mangSo[][8] = {
		{0,0x84,0x82,0xFF,0xFF,0x80,0x80,0},	// 1
		{0,0xC6,0xE7,0xB1,0x9F,0x8E,0x80,0},	// 2
		{0,0x62,0xC3,0x89,0x89,0xFF,0x76,0},	// 3
		{0,0x18,0x9C,0x92,0xFF,0xFF,0x80,0},	// 4
		{0,0x4F,0xCF,0x89,0x89,0xF9,0x71,0},	// 5
		{0,0x7E,0xFF,0x89,0x89,0xFB,0x72,0},	// 6
		{0,0xC1,0xE1,0x31,0x19,0x0F,0x07,0},	// 7
		{0,0x76,0xFF,0x89,0x89,0xFF,0x76,0},	// 8
		{0,0x46,0xCF,0x89,0x89,0xFF,0x7E,0},	// 9
		{0,0x7E,0xFF,0x81,0x81,0xFF,0x7E,0}		// 0
};

// Mang ki tu vui
unsigned int mangKiTuVui[][8] = {
		{0x04,0x22,0x44,0x40,0x40,0x44,0x22,0x04},	// ^v^
		{0x04,0x44,0x24,0x20,0x20,0x24,0x44,0x04},	// -_-
		{0,0x06,0x66,0x90,0x90,0x66,0x06,0},		// *o*
		{0,0x54,0x38,0x10,0x38,0x54,0,0},			// *
		{0x40,0x34,0x14,0x1C,0x14,0x16,0x15,0}		// #
};

/*
 * Chuong trinh con: putByte(uint8_t ui8Data)
 * Mo ta:  Su dung xung clock de nhap dataIn vao thanh ghi cua MAX7219:
 *  	  	     	   _____     _____     _____     _____     _____
 *         clock:    _|     |___|     |___|     |___|     |___|     |___
 *                   _____     _____     _____     _____     _____
 *         dataIn: _|     |___|     |___|     |___|     |___|     |___
 *         			  D0		D1  	  D2		D3		   D4 		...
 */

void putByte(int ui8Data)
{
	int i = 8;
	int ui8Mask;

	while(i>0)
	{
		ui8Mask = 0x01 << (i-1);						// Tao bit 1 o vi tri i-1
		GPIOPinWrite(GPIO_PORTE_BASE, clock, 0);		// Dua xung clock ve muc 0 de tao canh xuong

		// Kiem tra bit 7 cua du lieu nhap vao la 0 hay 1
		if (ui8Data & ui8Mask)
			GPIOPinWrite(GPIO_PORTB_BASE, dataIn, 1);	// Gui 1 vao thanh ghi
		else
			GPIOPinWrite(GPIO_PORTB_BASE, dataIn, 0);	// Gui 0 vao thanh ghi

		GPIOPinWrite(GPIO_PORTE_BASE, clock, 0x04);		// Bat canh len cho CLK de gui dataIn vao
		--i;
	}
}

/*
 * Chuong trinh con: maxSingle( uint8_t ui8Reg, uint8_t ui8Col)
 * Mo ta: Khoi tao cho MAX7219. Dung ham nay khi su dung chi 1 con MAX7219
 */

void maxSingle( int ui8Reg, int ui8Col)
{
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);				// Chuyen canh xuong cho load
	putByte (ui8Reg);    								// Cac thanh ghi dac biet cua MAX7219
	putByte (ui8Col);    								// ((ui8Data & 0x01) * 256) + data >> 1); --> put data
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0x02);
}

/*
 * Chuong trinh con: maxAll (uint8_t ui8Reg, uint8_t ui8Col)
 * Mo ta: Khoi tao cho tat ca MAX7219 co trong he thong
 */

void maxAll (int ui8Reg, int ui8Col)
{
	int c = 0;
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);

	// Khoi tao cho tat ca MAX7219 trong he thong
	for (c=0; c<maxInUse; c++)
	{
		putByte (ui8Reg);    							// Cac thanh ghi dac biet cua MAX7219
		putByte (ui8Col);    							// ((ui8Data & 0x01) * 256) + data >> 1); --> put data
	}
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0x02);
}

/*
 * Chuong trinh con: maxOne(uint8_t ui8MaxNr, uint8_t ui8Reg, uint8_t ui8Col)
 * Mo ta: Khoi tao cho MAX7219 duoc chon
 */

void maxOne(int ui8MaxNr, int ui8Reg, int ui8Col)
{
	int c = 0;
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);

	for (c=maxInUse; c>ui8MaxNr; c--)
	{
		putByte (0);    							// Khong thuc thi voi cac MAX7219 trong khoang maxInUse toi ui8MaxNr
		putByte (0);    							// Khong thuc thi
	}

	putByte (ui8Reg);    							// Cac thanh ghi dac biet cua MAX7219
	putByte (ui8Col);    							// ((ui8Data & 0x01) * 256) + data >> 1); --> put data

	for (c= (ui8MaxNr - 1); c>=1; c--)
	{
		putByte (0);    							// Khong thuc thi voi cac MAX7219 trong khoang maxInUse toi ui8MaxNr
		putByte (0);    							// Khong thuc thi
	}

	GPIOPinWrite(GPIO_PORTD_BASE, load, 0);
	GPIOPinWrite(GPIO_PORTD_BASE, load, 0x02);
}

/*
 * Ten chuong trinh: Init_System
 * Mo ta: Cau hinh xung he thong va cap xung vao cac port
 */
void Init_System(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_5| SYSCTL_USE_PLL| SYSCTL_OSC_MAIN| SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	SysCtlDelay(5);

	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, dataIn);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, load);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, clock);
}

/*
 * Ten chuong trinh: Init_Matrix
 * Mo ta: Khoi tao cho led ma tran
 */

void Init_Matrix(void)
{
	maxAll (max7219_reg_scanLimit, 0x07);
	maxAll (max7219_reg_decodeMode, 0x00);
	maxAll (max7219_reg_shutdown, 0x01);
	maxAll (max7219_reg_displayTest, 0x00);

	// Tat tat ca cac LED
	for (e=1; e<=8; e++)
		maxAll(e, 0);
	// Chon do sang cho led ma tran
	maxAll (max7219_reg_intensity, 0x0f);	// scale: 0 - 15
}

void Init_KeyPad(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7, 0xF0);

	GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_2| GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5, GPIO_DIR_MODE_IN);
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2| GPIO_PIN_3| GPIO_PIN_4| GPIO_PIN_5, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
}

/*
 * Chuong trinh con: Snake
 * Mo ta: Lay trang thai nut nhan, dieu khien huong cho ran
 */
void Snake(void)
{
	int j = 0;

	switch(ui8Status)
	{
	case down:
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		// Don than ran
		for (i=14; i>0; i--)
		{
			snakex[i] = snakex[i-1];
			snakey[i] = snakey[i-1];
		}

		snakey[0]++;
		if(snakey[0]>7)
			snakey[0] = 0;
		break;

	case right:
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		for (i=14; i>0; i--)
		{
			snakex[i] = snakex[i-1];
			snakey[i] = snakey[i-1];
		}

		snakex[0]++;
		if(snakex[0]>7)
			snakex[0] = 0;
		break;

	case up:
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		for (i=14; i>0; i--)
		{
			snakex[i] = snakex[i-1];
			snakey[i] = snakey[i-1];
		}

		snakey[0]--;
		if(snakey[0]<0)
			snakey[0] = 7;
		break;

	case left:
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		for (i=14; i>0; i--)
		{
			snakex[i] = snakex[i-1];
			snakey[i] = snakey[i-1];
		}

		snakex[0]--;
		if(snakex[0]<0)
			snakex[0] = 7;
		break;
	}
}

/*
 * Chuong trinh con: Fruit
 * Mo ta: Tao moi cho ran, su dung ham rand trong thu vien stdlib
 */
void Fruit(void)
{
	int j = 0;

	for (ui32Loop=0; ui32Loop<5; ui32Loop++)
	{
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		savePicture[fruitx] = savePicture[fruitx] + mangy[fruity];

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		SysCtlDelay(10000);

		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}

		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;

		SysCtlDelay(1000);
	}

	if((fruitx==snakex[0])&&(fruity==snakey[0]))
	{
		ui8Length++;
		for (i=0; i<8; i++)
		{
			for (j=0; j<ui8Length; j++)
			{
				if (snakex[j]==i)
					savePicture[i] = savePicture[i] + mangy[snakey[j]];
			}
		}
		for (i=0; i<8; i++)
			maxAll(mangx[i], savePicture[i]);

		for (i=0; i<8; i++)
			savePicture[i] = 0;
	}

	if ((fruitx==snakex[0])&&(fruity==snakey[0]))
	{
		fruitx = rand()%8;
		fruity = rand()%8;
	}

	for (i=1; i<15; i++)
	{
		if((fruitx==snakex[i])&&(fruity==snakey[i]))
		{
			fruitx = rand()%8;
			fruity = rand()%8;
		}
	}
}

/*
 * Chuong trinh con: Win
 * Mo ta: Khi ui8Length = 10 --> Chien thang va ui8Length = 3 tro lai
 */
void Win(void)
{
	int j = 0;
	begin = 1;

	unsigned char winEnd[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char winStart[][8] = {
			{0,0xFF,0x7F,0x20,0x20,0x7F,0xFF,0},
			{0xFF,0x7F,0x20,0x20,0x7F,0xFF,0,0},
			{0x7F,0x20,0x20,0x7F,0xFF,0,0,0x81},
			{0x20,0x20,0x7F,0xFF,0,0,0x81,0x81},
			{0x20,0x7F,0xFF,0,0,0x81,0x81,0xFF},
			{0x7F,0xFF,0,0,0x81,0x81,0xFF,0xFF},
			{0xFF,0,0,0x81,0x81,0xFF,0xFF,0x81},
			{0,0,0x81,0x81,0xFF,0xFF,0x81,0x81},
			{0,0x81,0x81,0xFF,0xFF,0x81,0x81,0},
			{0x81,0x81,0xFF,0xFF,0x81,0x81,0,0},
			{0x81,0xFF,0xFF,0x81,0x81,0,0,0xFF},
			{0xFF,0xFF,0x81,0x81,0,0,0xFF,0xFE},
			{0xFF,0x81,0x81,0,0,0xFF,0xFE,0x08},
			{0x81,0x81,0,0,0xFF,0xFE,0x08,0x10},
			{0x81,0,0,0xFF,0xFE,0x08,0x10,0x7F},
			{0,0,0xFF,0xFE,0x08,0x10,0x7F,0xFF},
			{0,0xFF,0xFE,0x08,0x10,0x7F,0xFF,0}
	};

	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for(j=0; j<17; j++)
	{
		for (i=0; i<8; i++)
			maxAll(mangx[i], winStart[j][i]);

		SysCtlDelay(SysCtlClockGet()/12);
	}

	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for (i=0; i<8; i++)
	{
		maxAll(mangx[i], winEnd[i]);
		SysCtlDelay(SysCtlClockGet()/12);
	}
	ui8Length = 3;
}

/*
 * Chuong trinh con: Lose
 * Mo ta: Khi dau ran cham bat ki than ran --> hien thi Lose --> tro ve ui8Length=3
 */
void Lose(void)
{
	begin = 1;
	start = 0;
	flag = 0;
	int j = 0;

	unsigned char loseEnd[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char lose[8] = {0x81,0x42,0x24,0x18,0x18,0x24,0x42,0x81};
	unsigned char loseStart[][8] = {
			{0,0xFF,0xFF,0x80,0x80,0x80,0x80,0},	// L
			{0xFF,0xFF,0x80,0x80,0x80,0x80,0,0},
			{0xFF,0x80,0x80,0x80,0x80,0,0,0x7E},
			{0x80,0x80,0x80,0x80,0,0,0x7E,0xFF},
			{0x80,0x80,0x80,0,0,0x7E,0xFF,0x81},
			{0x80,0x80,0,0,0x7E,0xFF,0x81,0x81},
			{0x80,0,0,0x7E,0xFF,0x81,0x81,0xFF},
			{0,0,0x7E,0xFF,0x81,0x81,0xFF,0x7E},
			{0,0x7E,0xFF,0x81,0x81,0xFF,0x7E,0},	// 0
			{0x7E,0xFF,0x81,0x81,0xFF,0x7E,0,0},
			{0xFF,0x81,0x81,0xFF,0x7E,0,0,0x46},
			{0x81,0x81,0xFF,0x7E,0,0,0x46,0x8F},
			{0x81,0xFF,0x7E,0,0,0x46,0x8F,0x89},
			{0xFF,0x7E,0,0,0x46,0x8F,0x89,0x89},
			{0x7E,0,0,0x46,0x8F,0x89,0x89,0xF9},
			{0,0,0x46,0x8F,0x89,0x89,0xF9,0x72},
			{0,0x46,0x8F,0x89,0x89,0xF9,0x72,0},	// S
			{0x46,0x8F,0x89,0x89,0xF9,0x72,0,0},
			{0x8F,0x89,0x89,0xF9,0x72,0,0,0xFF},
			{0x89,0x89,0xF9,0x72,0,0,0xFF,0xFF},
			{0x89,0xF9,0x72,0,0,0xFF,0xFF,0x89},
			{0xF9,0x72,0,0,0xFF,0xFF,0x89,0x89},
			{0x72,0,0,0xFF,0xFF,0x89,0x89,0x89},
			{0,0,0xFF,0xFF,0x89,0x89,0x89,0x81},
			{0,0xFF,0xFF,0x89,0x89,0x89,0x81,0},	// E
	};

	for (e=1; e<=8; e++)
		maxAll(e, 0);
	for(j=0; j<3; j++)
	{
		for (i=0; i<8; i++)
			maxAll(mangx[i], lose[i]);
		SysCtlDelay(SysCtlClockGet()/6);
		for (e=1; e<=8; e++)
			maxAll(e, 0);
		SysCtlDelay(SysCtlClockGet()/6);
	}

	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for(j=0; j<25; j++)
	{
		for (i=0; i<8; i++)
			maxAll(mangx[i], loseStart[j][i]);

		SysCtlDelay(SysCtlClockGet()/12);
	}

	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for (i=0; i<8; i++)
	{
		maxAll(mangx[i], loseEnd[i]);
		SysCtlDelay(SysCtlClockGet()/12);
	}

	ui8Length = 3;
	ui8Speed = 1;
}

/*
 * Chuong trinh con: Begin
 * Mo ta: Tao su chuan bi cho nguoi choi
 */
void Begin(void)
{
	begin = 0;
	int j =0;
	unsigned char begin[][8] = {
			{0,0x62,0xC3,0x89,0x89,0xFF,0x76,0},	// 3
			{0,0xC6,0xE7,0xB1,0x9F,0x8E,0x80,0},	// 2
			{0,0x84,0x82,0xFF,0xFF,0x80,0x80,0},	// 1
	};
	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for(j=0; j<3; j++)
	{
		for (i=0; i<8; i++)
			maxAll(mangx[i], begin[j][i]);
		SysCtlDelay(SysCtlClockGet()/3);
	}

	ui8Speed = ui8Speed * 2;
}

/*
 * Chuong trinh con: Wait
 * Mo ta: Cho doi trong luc nhan Start
 */
void Wait(void)
{
	int j = 0;
	unsigned char wait[][8] = {
			{0x06,0x29,0x66,0xA0,0xA0,0x66,0x29,0x06},
			{0x04,0x24,0x24,0x20,0x20,0x24,0x24,0x04}
	};

	for (e=1; e<=8; e++)
		maxAll(e, 0);

	for(j=0; j<2; j++)
	{
		for (i=0; i<8; i++)
			maxAll(mangx[i], wait[j][i]);
		SysCtlDelay(SysCtlClockGet()/6);
	}
}

int main(void)
{
	Init_System();
	Init_KeyPad();
	Init_Matrix();


	while(1)
	{
		// Doc gia tri nut nhan
		for (i=0; i<3; i++)
		{
			GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_4| GPIO_PIN_5| GPIO_PIN_6| GPIO_PIN_7, readArray[i]);
			switch(i)
			{
			case 0:
				if ((GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3)==0)&&(ui8Status!=down))
					ui8Status = up;
				break;
			case 1:
				if ((GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_2)==0)&&(ui8Status!=right))
					ui8Status = left;
				if ((GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_4)==0)&&(ui8Status!=left))
					ui8Status = right;
				if (GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3)==0)
					start = 1;
				break;
			case 2:
				if ((GPIOPinRead(GPIO_PORTA_BASE, GPIO_PIN_3)==0)&&(ui8Status!=up))
					ui8Status = down;
				break;
			}
		}

		for (e=1; e<=8; e++)
			maxAll(e, 0);

		if (start==1)
		{
			if(begin==1)
				Begin();

			Snake();
			Fruit();

			if (ui8Length == 15)
				Win();

			// Kiem tra thua
			for (i=1; i<ui8Length; i++)
			{
				if((snakex[i]==snakex[0])&&(snakey[i]==snakey[0]))
					flag = 1;
			}

			if(flag==1)
				Lose();
			SysCtlDelay(SysCtlClockGet()/6/ui8Speed);
		}
		else
			Wait();
	}
}


