#include "main.h"
#include <stdio.h>

#define STEPX_PORT GPIOA
#define STEPX_PIN GPIO_PIN_10
#define DIRX_PIN GPIO_PIN_4
#define DIRX_PORT GPIOB
#define START_PERIOD 1000
#define STEP_TOTAL_360 200
#define INTEGRAL_PERIOD 50000 	//max 65535
#define SEC_TO_US_2 2000000


enum { COUNTERCLOCK, CLOCKWISE } ;


TIM_HandleTypeDef getTimer(int i);

void setFlag(uint8_t val);

void set_integral_flag(uint8_t);

void MX_TIM2_Init();

void MX_TIM3_Init();

void MX_TIM5_Init();

void TIM_StartTrackInterruptTimer(void);

void TIM_StopInterruptTimer();

void TIM_StartInterruptTimer();

void toggle_half_step();

void make_n_steps(uint32_t cnt);

void set_motor_direction(uint8_t dir);

void set_motor_angle(float angle);

void velocity_mode(uint16_t rpm);

//run forever
void debug_mode(uint16_t rpm);

void TIM_StartIntegralInterruptTimer();


