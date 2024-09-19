#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "main.h"
#include "lcd.h"
#include "lcd_init.h"
#include "stdio.h"
#include "rtc.h"
#include "ina219.h"

#define longPressThreshold 1000 

typedef struct fan_info
{
    uint16_t cur_rpm;
    uint16_t tar_rpm;
    float cur_vol;
    float cur_cut;
    float tar_cut;
    float cur_pow;
    int8_t tar_duty;
    uint32_t time;
}fan_info;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern INA219_t ina219;

void ui_init(void);
void refresh_info(fan_info info);
void get_status(fan_info* info);
void pwm_out(float value);
void get_rtc_seconds(fan_info *info);
void show_duty(float duty);
float pid_ctrl(fan_info info);
void load_word(void);

void Timer_Event(uint32_t time, uint8_t onePulse, void (*func)(void));
void Timer_Event_Stop(void);

#endif
