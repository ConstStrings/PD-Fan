#include "function.h"

volatile float Freq = 0.0;
volatile float Duty = 0.0;
volatile int capture_end_flag = 0;
volatile uint8_t capture_cnt = 1;
 
volatile uint32_t high_val = 0;
volatile uint32_t low_val = 0;

extern fan_info info;
uint8_t mode_flag = 1;

void (*Timer_Event_CallBack)(void);
uint8_t onePulse_flag = 0;


void ui_init()
{
    LCD_Init();
	
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_UPDATE);
	
    LCD_Fill(0,0,320,172,BLACK);

    LCD_DrawRoundRectangle_DMA(0, 0, 210, 100, 20, WHITE);
    LCD_DrawRoundRectangle_DMA(0, 110, 110, 172, 20, WHITE);
    LCD_DrawRoundRectangle_DMA(115, 110, 210, 172, 20, WHITE);
    LCD_DrawRoundRectangle_DMA(220, 0, 320, 172, 20, WHITE);

    LCD_DrawLine(30, 90, 180, 90, BLACK);
    LCD_DrawLine(30, 95, 180, 95, BLACK);
    LCD_DrawPoint(29, 90, BLACK);
    LCD_DrawPoint(28, 91, BLACK);
    LCD_DrawPoint(27, 92, BLACK);
    LCD_DrawPoint(27, 93, BLACK);
    LCD_DrawPoint(28, 94, BLACK);
    LCD_DrawPoint(29, 95, BLACK);
    LCD_DrawPoint(181, 90, BLACK);
    LCD_DrawPoint(182, 91, BLACK);
    LCD_DrawPoint(183, 92, BLACK);
    LCD_DrawPoint(183, 93, BLACK);
    LCD_DrawPoint(182, 94, BLACK);
    LCD_DrawPoint(181, 95, BLACK);
}

void load_word(void)
{
    LCD_ShowString(10, 5, (uint8_t*)"Speed", BLACK, WHITE, 24, 0);
    LCD_ShowString(150, 60, (uint8_t*)"rpm", BLACK, WHITE, 24, 0);

    if(mode_flag)
    {
        LCD_ShowString(20, 110, (uint8_t*)"Target", BLACK, WHITE, 24, 0);
        LCD_ShowString(80, 145, (uint8_t*)"rpm", BLACK, WHITE, 16, 0);
    }
    else
    {
        LCD_ShowString(30, 110, (uint8_t*)"Duty", BLACK, WHITE, 24, 0);
        LCD_ShowString(80, 140, (uint8_t*)"%", BLACK, WHITE, 24, 0);
    }
    


    LCD_ShowString(190, 140, (uint8_t*)"A", BLACK, WHITE, 24, 0);
    LCD_ShowString(135, 110, (uint8_t*)"Limit", BLACK, WHITE, 24, 0);

    LCD_ShowString(230, 5, (uint8_t*)"Voltage", BLACK, WHITE, 24, 0);
    LCD_ShowString(230, 65, (uint8_t*)"Current", BLACK, WHITE, 24, 0);
    LCD_ShowString(240, 118, (uint8_t*)"Power", BLACK, WHITE, 24, 0);
    LCD_ShowString(300, 35, (uint8_t*)"V", BLACK, WHITE, 24, 0);
    LCD_ShowString(300, 90, (uint8_t*)"A", BLACK, WHITE, 24, 0);
    LCD_ShowString(300, 144, (uint8_t*)"W", BLACK, WHITE, 24, 0);
}

void refresh_info(fan_info info)
{
    char buffer[30];
    sprintf(buffer, "%05d", info.cur_rpm);
    LCD_ShowString(60, 35, (uint8_t*)buffer, BLACK, WHITE, 32, 0);

    if(mode_flag)
    {
        sprintf(buffer, "%05d", info.tar_rpm);
        LCD_ShowString(15, 140, (uint8_t*)buffer, BLACK, WHITE, 24, 0);
    }
    else
    {
        sprintf(buffer, "%03d", info.tar_duty);
        LCD_ShowString(25, 135, (uint8_t*)buffer, BLACK, WHITE, 32, 0);
    }


    sprintf(buffer, "%02.2f", info.tar_cut);
    LCD_ShowString(125, 135, (uint8_t*)buffer, BLACK, WHITE, 32, 0);

    sprintf(buffer, "%02d.%02d", (int)info.cur_vol,(int)(info.cur_vol * 100) % 100);
    LCD_ShowString(232, 35, (uint8_t*)buffer, BLACK, WHITE, 24, 0);
    sprintf(buffer, "%01d.%03d", (int)info.cur_cut % 10,(int)(info.cur_cut * 1000) % 1000);
    LCD_ShowString(235, 90, (uint8_t*)buffer, BLACK, WHITE, 24, 0);
    //sprintf(buffer, "%02d:%02d", info.time / 60, info.time % 60);
    sprintf(buffer, "%02d.%02d", (int)info.cur_pow,(int)(info.cur_pow * 100) % 100);
    LCD_ShowString(236, 144, (uint8_t*)buffer, BLACK, WHITE, 24, 0);
}

void get_status(fan_info* info)
{
    info->cur_vol = INA219_ReadBusVoltage(&ina219) / 1000.0f;
	capture_end_flag = 0;
    info->cur_rpm = Freq * 60;
    info->cur_cut = INA219_ReadCurrent_mA(&ina219) / 1000.0f;
    info->cur_pow = info->cur_vol * info->cur_cut ;
}

void pwm_out(float value)
{
    if(value > 1.0)
        value = 1.0;
    else if(value < 0.0)
        value = 0.0;
    uint16_t cmp = (int)(value * 2880);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, cmp);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void get_rtc_seconds(fan_info* info)
{
    RTC_TimeTypeDef sTime;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    info->time = sTime.Seconds + sTime.Minutes * 60 + sTime.Hours * 3600;
}

void show_duty(float duty)
{
    uint16_t len = (int)(duty * 154 + 0.5);
    LCD_DrawLine(28, 92, 28 + len, 92, BLACK);
    LCD_DrawLine(28, 93, 28 + len, 93, BLACK);
    if(len != 154)
    {
        LCD_DrawLine(28 + len + 1, 92, 182, 92, WHITE);
        LCD_DrawLine(28 + len + 1, 93, 182, 93, WHITE);
    }
    if(len > 0)
    {
        if(len < 154)
        {
            LCD_DrawLine(29, 91, 29 + 1 + len - 2, 91, BLACK);
            LCD_DrawLine(29, 94, 29 + 1 + len - 2, 94, BLACK);
            LCD_DrawLine(29 + len, 91, 181, 91, WHITE);
            LCD_DrawLine(29 + len, 94, 181, 94, WHITE);
        }
        else
        {
            LCD_DrawLine(29, 91, 181, 91, BLACK);
            LCD_DrawLine(29, 94, 181, 94, BLACK);
        }    
    }
    else
    {
        LCD_DrawLine(29, 91, 181, 91, WHITE);
        LCD_DrawLine(29, 94, 181, 94, WHITE);
    }
}

float pid_ctrl(fan_info info)
{
    float error = (info.tar_rpm - info.cur_rpm) / 1000;
    float p = 0.10;
    float i = 0.01;
    float d = 0.30;
    static float last_error = 0;
    static float sum_error = 0;
    sum_error += error;
    float output = p * error + i * sum_error + d * (error - last_error);
    last_error = error;
    if(output > 1.0)
        output = 1.0;
    else if(output < 0.0)
        output = 0.0;
    return output;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{        
    if(htim->Instance == TIM1)       
    {    
        if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)    
        {    
            if(capture_end_flag == 0)
            {
                if(capture_cnt == 1)        //第一个上升沿
                {
                    capture_cnt = 2;
                    __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING); //设置成下降沿触发
                    __HAL_TIM_SetCounter(htim, 0);    //清空定时器计数值
                    high_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);    //由第一个上升沿设为起始位置
                    //__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
                    
                }else if(capture_cnt == 2)    //第一个下降沿
                {
                    capture_cnt = 3;
                    low_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);    //低电平起始位置
                    __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1,TIM_INPUTCHANNELPOLARITY_RISING); //设置成上升沿触发
                    
                    
                }else if(capture_cnt == 3)    //第二个上升沿
                {
                    capture_cnt = 1;
                    high_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                      
                    //计算频率
                    Freq = (float)72000000 / 720 / (high_val+1);
                    //计算占空比
                    Duty = (float)(low_val+1) / (high_val+1);
                    capture_end_flag = 1;
                    //__HAL_TIM_DISABLE_IT(htim, TIM_IT_UPDATE);
                }
            }
        }
    }
}

void Timer_Event(uint32_t time,uint8_t onePulse,void (*func)(void))
{
    __HAL_TIM_SET_AUTORELOAD(&htim2, time * 10 - 1);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    Timer_Event_CallBack = func;
    onePulse_flag = onePulse;
    HAL_TIM_Base_Start_IT(&htim2);
}

void Timer_Event_Stop(void)
{
    HAL_TIM_Base_Stop_IT(&htim2);
}

uint8_t First_Event_flag = 1; //第一次会直接触发回调，原因未知
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        if(First_Event_flag)
        {
            First_Event_flag = 0;
        }
        else
        {
            if(onePulse_flag)
                HAL_TIM_Base_Stop_IT(&htim2);
            Timer_Event_CallBack();
        }
    }
    if(htim->Instance == TIM1)
    {
        Freq = 0;
        Duty = 0;
        capture_cnt = 1;   // 重置捕获状态
        capture_end_flag = 1;  // 标志捕获结束
    }
}



