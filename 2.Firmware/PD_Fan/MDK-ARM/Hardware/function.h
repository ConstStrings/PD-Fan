#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "main.h"
#include "lcd.h"
#include "lcd_init.h"
#include "stdio.h"
#include "rtc.h"
#include "ina219.h"

#define longPressThreshold 1000 

#define PARAM_ADD 0x08000000 + 0x0000F800

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

typedef struct fan_save
{
    uint16_t tar_rpm;
    float tar_cut;
    int8_t tar_duty;
    uint8_t checkByte;
}fan_save;

typedef struct Back_Color
{
    uint16_t Block1;
    uint16_t Block2;
    uint16_t Block3;
    uint16_t Block4;
}Back_Color;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern INA219_t ina219;

void system_init(void);
void load_ui(Back_Color color);
void refresh_info(fan_info info, Back_Color color);
void get_status(fan_info* info);
void pwm_out(float value);
void get_rtc_seconds(fan_info *info);
void show_duty(float duty);
float pid_ctrl(fan_info info);
void reload_word(Back_Color color);
float get_pid(void);

void Timer_Event(uint32_t time, uint8_t onePulse, void (*func)(void));
void Timer_Event_Stop(void);

void Load_Para(fan_info *info);
void Save_Para(fan_info info);

void check_current(fan_info info, Back_Color *color);

#endif
