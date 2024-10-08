#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define USE_HORIZONTAL 0 
#define LCD_W 240
#define LCD_H 240

typedef unsigned int u32;
typedef  unsigned short u16;
typedef  unsigned char u8;

void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);
void LCD_Init(void);
void closeLCD();

#endif