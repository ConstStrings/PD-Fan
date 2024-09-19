#include "lcd.h"
#include "lcd_init.h"
#include "lcdfont.h"
#include "main.h"


/******************************************************************************
      ����˵������ָ������������?
      ������ݣ�xsta,ysta   ��ʼ����
                xend,yend   ��ֹ����
								color       Ҫ������ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color)
{          
		
	u16 i,j; 
	LCD_Address_Set(xsta,ysta,xend-1,yend-1);//������ʾ��Χ
	
	uint16_t* buffer = (uint16_t*)malloc(xend - xsta);
	uint16_t* p = buffer;
												   	 	
	for(j=xsta;j<xend;j++)
	{
		*p = color >> 8 | color << 8;
		p++;
	}

	LCD_CS_Clr();	
	for(i=ysta;i<yend;i++)
	{
		HAL_SPI_Transmit(&hspi1, (uint8_t*)buffer, (xend - xsta) * sizeof(u16), 1000);
	}
	LCD_CS_Set();	
	free(buffer);	
}

/******************************************************************************
      ����˵������ָ��λ�û���
      ������ݣ�x,y ��������
                color ������?
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_Address_Set(x,y,x,y);//���ù��λ��?? 
	LCD_WR_DATA(color);
} 


/******************************************************************************
      ����˵��������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   �ߵ���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance;
	int incx,incy,uRow,uCol;
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1;
	uRow=x1;//�����������??
	uCol=y1;
	if(delta_x>0)incx=1; //���õ������� 
	else if (delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;}
	if(delta_y>0)incy=1;
	else if (delta_y==0)incy=0;//ˮƽ�� 
	else {incy=-1;delta_y=-delta_y;}
	if(delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y;
	for(t=0;t<distance+1;t++)
	{
		LCD_DrawPoint(uRow,uCol,color);//����
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}


/******************************************************************************
      ����˵����������
      ������ݣ�x1,y1   ��ʼ����
                x2,y2   ��ֹ����
                color   ���ε���ɫ
      ����ֵ��  ��
******************************************************************************/
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}




/******************************************************************************
      ����˵������Բ
      ������ݣ�x0,y0   Բ������
                r       �뾶
                color   Բ����ɫ
      ����ֵ��  ��
******************************************************************************/
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawPoint(x0-b,y0-a,color);             //3           
		LCD_DrawPoint(x0+b,y0-a,color);             //0           
		LCD_DrawPoint(x0-a,y0+b,color);             //1                
		LCD_DrawPoint(x0-a,y0-b,color);             //2             
		LCD_DrawPoint(x0+b,y0+a,color);             //4               
		LCD_DrawPoint(x0+a,y0-b,color);             //5
		LCD_DrawPoint(x0+a,y0+b,color);             //6 
		LCD_DrawPoint(x0-b,y0+a,color);             //7
		a++;
		if((a*a+b*b)>(r*r))//�ж�Ҫ���ĵ��Ƿ���?
		{
			b--;
		}
	}
}

void Draw_SolidCircle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	a=0;b=r;	  
	while(a<=b)
	{
		LCD_DrawLine(x0-b,y0-a,x0+b,y0-a,color);             //3           
		LCD_DrawLine(x0-b,y0+a,x0+b,y0+a,color);             //0           
		LCD_DrawLine(x0-a,y0+b,x0+a,y0+b,color);             //1                
		LCD_DrawLine(x0-a,y0-b,x0+a,y0-b,color);             //2             
		a++;
		if((a*a+b*b)>(r*r))
		{
			b--;
		}
	}
}


/******************************************************************************
      ����˵������ʾ�����ַ�
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ���ַ�
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	// u8* temp;
	// u8 sizex,t=0;
	// u16 i,TypefaceNum;//һ���ַ���ռ�ֽڴ�С
	// u16 x0=x;
	// sizex=sizey/2;
	// TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	// num=num-' ';    //�õ�ƫ�ƺ���?

	// u16* buffer = (u16*)malloc(sizey * sizex * sizeof(u16));
    // u16 *pBuffer = buffer;

	// LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //���ù��λ��??

	// for(i=0;i<sizey;i++)
	// { 
	// 	if(sizey==12)
	// 		temp=(uint8_t *)&ascii_1206[num][i];		       //����6x12����
	// 	else if(sizey==16)
	// 		temp=(uint8_t *)&ascii_1608[num][i];		 //����8x16����
	// 	else if(sizey==24)
	// 		temp=(uint8_t *)&ascii_2412[num][i*2];		 //����12x24����
	// 	else if(sizey==32)
	// 		temp=(uint8_t *)&ascii_3216[num][i*2];		 //����16x32����
	// 	else return;
	// 	for(t=0;t<sizex;t++)	//�޸Ĺ������ܵ��µ���ģʽ�쳣
	// 	{
	// 		if (!mode) // �ǵ���ģʽ
    //         {
    //             if ((temp[t / 8]) & (0x01 << (t % 8)))
    //                 *pBuffer = fc >> 8 | fc << 8;
    //             else
    //                 *pBuffer = bc >> 8 | bc << 8;

    //             pBuffer++;
    //         }
	// 		else//����ģʽ
	// 		{
	// 			if(*temp&(0x01<<t))
	// 				LCD_DrawPoint(x,y,fc);//��һ����
	// 			x++;
	// 			if((x-x0)==sizex)
	// 			{
	// 				x=x0;
	// 				y++;
	// 				break;
	// 			}
	// 		}
	// 	}
	// }   	 	

	// LCD_CS_Clr();
	// HAL_SPI_Transmit(&hspi1, (uint8_t*)buffer, sizey * sizex * sizeof(u16), 1000);
	// LCD_CS_Set();
	// free(buffer);

	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	LCD_Address_Set(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))LCD_WR_DATA(fc);
				else LCD_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))LCD_DrawPoint(x,y,fc);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 
}


/******************************************************************************
      ����˵������ʾ�ַ���
      ������ݣ�x,y��ʾ����
                *p Ҫ��ʾ���ַ���
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
                mode:  0�ǵ���ģʽ  1����ģʽ
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}


/******************************************************************************
      ����˵������ʾ����
      ������ݣ�m������nָ��
      ����ֵ��  ��
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      ����˵������ʾ��������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾ��������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 


/******************************************************************************
      ����˵������ʾ��λС������
      ������ݣ�x,y��ʾ����
                num Ҫ��ʾС������
                len Ҫ��ʾ��λ��
                fc �ֵ���ɫ
                bc �ֵı���ɫ
                sizey �ֺ�
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}


/******************************************************************************
      ����˵������ʾͼƬ
      ������ݣ�x,y�������??
                length ͼƬ����
                width  ͼƬ����
                pic[]  ͼƬ����    
      ����ֵ��  ��
******************************************************************************/
void LCD_ShowPicture_DMA(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	LCD_CS_Clr();
	HAL_SPI_Transmit_DMA(&hspi1, (u8*)pic, length*width*2);
}

void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	LCD_Address_Set(x,y,x+length-1,y+width-1);
	LCD_CS_Clr();
	HAL_SPI_Transmit(&hspi1, (u8*)pic, length*width*2,1000);
	LCD_CS_Set();	
}


void LCD_DrawRoundRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 r,u16 color)
{
	//LCD_Fill(x1, y1 + r, x2, y2 - r, color);
	//LCD_Fill(x1 + r, y1, x2 - r, y2, color);
	//Draw_SolidCircle(x1 + r, y1 + r, r, color);
	//Draw_SolidCircle(x2 - r, y1 + r, r, color);
	//Draw_SolidCircle(x1 + r, y2 - r, r, color);
	//Draw_SolidCircle(x2 - r, y2 - r, r, color);

	u16 i;

    // 1. 绘制中间的矩形部分（去掉上、下圆角区域的矩形）
    for (i = y1 + r; i <= y2 - r; i++) {
        LCD_DrawLine(x1, i, x2, i, color);  // 每一行画一条横线
    }

    // 2. 绘制顶部和底部的矩形区域（去掉左右圆角的部分）
    for (i = y1; i < y1 + r; i++) {
        LCD_DrawLine(x1 + r, i, x2 - r, i, color);  // 顶部直线
    }
    for (i = y2 - r + 1; i <= y2; i++) {
        LCD_DrawLine(x1 + r, i, x2 - r, i, color);  // 底部直线
    }

    // 3. 绘制四个圆角
    Draw_SolidCircle(x1 + r, y1 + r, r, color);  // 左上角圆
    Draw_SolidCircle(x2 - r, y1 + r, r, color);  // 右上角圆
    Draw_SolidCircle(x1 + r, y2 - r, r, color);  // 左下角圆
    Draw_SolidCircle(x2 - r, y2 - r, r, color);  // 右下角圆

}

void LCD_DrawRoundRectangle_DMA(u16 x1, u16 y1, u16 x2, u16 y2, u16 r1,u16 color)
{
	static uint16_t buffer[320];
	volatile u16 r = r1;
	for(int i = y1; i < y1 + r; i++)
	{
		uint16_t b_num = (int)(r - sqrt(r*r - (r - i + y1)*(r - i + y1)));
		
		for(int j = 0; j < b_num; j++)
		{ 
			buffer[j] = BLACK;
		}
		for(int j = b_num; j < x2 - x1 - b_num; j++)
		{
			buffer[j] = color << 8 | color >> 8;
		}
		for(int j = x2 - x1 - b_num; j < x2 - x1; j++)
		{
			buffer[j] = BLACK;
		}
		LCD_ShowPicture(x1, i, x2 - x1, 1, (u8*)buffer);
	}

	for(int i = y1 + r; i <= y2 - r; i++)
	{
		for(int j = 0; j < x2 - x1; j++)
		{
			buffer[j] = color << 8 | color >> 8;
		}
		LCD_ShowPicture(x1, i, x2 - x1, 1, (u8*)buffer);
	}

	for(int i = y2 - r + 1; i <= y2; i++)
	{
		uint16_t b_num = (int)(r - sqrt(r*r - (i - y2 + r )*(i - y2 + r)));
		for(int j = 0; j < b_num; j++)
		{
			buffer[j] = BLACK;
		}
		for(int j = b_num; j < x2 - x1 - b_num; j++)
		{
			buffer[j] = color << 8 | color >> 8;
		}
		for(int j = x2 - x1 - b_num; j < x2 - x1; j++)
		{
			buffer[j] = BLACK;
		}
		LCD_ShowPicture(x1, i, x2 - x1, 1, (u8*)buffer);
	}
}
