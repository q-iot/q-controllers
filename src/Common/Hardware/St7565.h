#ifndef LCD_ST7565_H
#define LCD_ST7565_H

void LCD_init(void);
void LCD_Reset(void) ;
 
void LCD_DrawRegion(u16 X,u16 Y,u16 W,u16 H,const u8 *pData);
void LCD_Fill(u16 X,u16 Y,u16 W,u16 H,u8 Data);
void LCD_DrawRegion2(u16 X,u16 Y,u16 W,u16 H,const u8 *pData);
void LCD_Fill2(u16 X,u16 Y,u16 W,u16 H,u8 Data);
void LCD_Slide(bool ToRight);







#endif

