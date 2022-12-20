#include <motion_controller.h>
#include "main.h"

volatile uint8_t pulse_flag = 0;
volatile uint8_t integral_flag = 0;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;
volatile uint8_t motor_direction = 0;
static volatile int32_t current_step = 0;

#define debug 0

uint8_t get_pulse_flag() {
	return pulse_flag;
}

uint8_t getIntegralFlag() {
	return integral_flag;
}

void set_pulse_flag(uint8_t val) {
	pulse_flag = val;
}

void set_integral_flag(uint8_t val) {
	integral_flag = val;
}


void set_motor_direction(uint8_t dir) {
	motor_direction = dir;
	HAL_GPIO_WritePin(DIRX_PORT, DIRX_PIN, dir);
}

void set_motor_angle(float angle) {
	float angle_per_step = (float)STEP_TOTAL_360 / 360.0; // 200/360

	uint32_t steps = (uint32_t)(angle * angle_per_step);

	make_n_steps(steps);
}

/*
 * T_out = (PSC * Preload) / Clock
 */

void make_n_steps(uint32_t cnt) {
	uint32_t idx = 0;

	// cnt * 2
	while(idx < cnt*2) {
		if(get_pulse_flag()){
			toggle_half_step();
			set_pulse_flag(0);
			idx++;
		}
	}
}

uint16_t rpm_to_period(uint16_t velocity) {

	double micro_period = (1000.0/((double)velocity / 60.0))/STEP_TOTAL_360 * 1000.0 / 2;

	return (uint16_t)micro_period;
}

void update_step_count(uint8_t dir) {
	if(motor_direction == CLOCKWISE) {
		current_step++;
	}
	if(motor_direction == COUNTERCLOCK) {
		current_step--;
	}
}

void velocity_mode(uint16_t rpm) {
	int current_period = START_PERIOD;

	rpm = rpm > 1200 ? 1200: rpm;
	rpm = rpm < 600  ? 600 : rpm;

	int micro_second = rpm_to_period(rpm);
	int offset = micro_second >= 1000 ? 3 : 1;
	int aim = micro_second - offset;

	int minuend = 1;
	int running = 1;

	uint32_t steps_current = 0;
	uint32_t steps_last = 0;

	uint32_t velocity_rpm_current = 0;
	uint32_t velocity_rpm_last = 0;

	__HAL_TIM_SetCounter(&htim5, 0);
	uint32_t timestamp_current = __HAL_TIM_GetCounter(&htim5);
	uint32_t timestamp_last = 0;

	int32_t speed_current = 0;
	int32_t speed_last = 0;

	uint8_t count_flag = 1;
	uint8_t hit_max = 0;
	uint8_t is_max = 0;

	uint32_t integral_cnt = 1;
	uint32_t integral = 0;

	uint32_t keep_max = 0;
	while(running) {
		while(pulse_flag&(1<<0)) {
			toggle_half_step();
			//reset interrupt flag
			set_pulse_flag(0);
			//toggle step count flag -> 2 halfsteps = 1 step
			count_flag ^= 1UL << 0;

			//only do stuff when full step was executed
			if(count_flag&(1<<0))continue;

			if(integral_flag&(1<<0)) {

				//calculate
				if(!is_max) current_period = current_period - minuend <= aim ? aim : current_period - minuend;
				set_integral_flag(0);
				__HAL_TIM_SET_AUTORELOAD(&htim2, current_period);
				set_pulse_flag(0);

				//tweak acceleration depending on period and torque
				if(hit_max) {
					if(current_period > 400){
						minuend = -10;
					} else
						minuend = -1;
				} else {
					if(current_period > 400){
						minuend = 10;
					} else
						minuend = 1;
				}


				if(aim == current_period && !hit_max){
					__HAL_TIM_SetCounter(&htim5, 0);
					hit_max = 1;
					is_max = 1;
					keep_max = __HAL_TIM_GetCounter(&htim5);
					minuend = -1;	//running = 0;

				}

				if(__HAL_TIM_GetCounter(&htim5) - keep_max > SEC_TO_US_2) {
					is_max = 0;
				}

				//break from loop
				if(current_period >= START_PERIOD) running = 0;

				timestamp_current = __HAL_TIM_GetCounter(&htim5);
				steps_current = current_step;

				int32_t step_delta = steps_current - steps_last;
				int32_t periods_per_second = (1000/(INTEGRAL_PERIOD/1000));
				const uint8_t MINUTE_MULTI = 60;

				velocity_rpm_current = (step_delta * periods_per_second * MINUTE_MULTI / STEP_TOTAL_360) / 2; // /2 weil 2 impulse pro schritt
				speed_current = (velocity_rpm_current - velocity_rpm_last);

				integral += step_delta / (INTEGRAL_PERIOD/1000);

				int32_t val = integral / integral_cnt;

#if debug
				printf("velocity:%lu\n\rspeed:%ld\n\rperiod:%lu µs\n\r\n",
						velocity_rpm_current,
						speed_current,
						(timestamp_current - timestamp_last));

				printf("integral: %ld\n\r", val);
				printf("delta steps: %ld\n\r", steps_current - steps_last);
#endif
				velocity_rpm_last = velocity_rpm_current;
				steps_last = steps_current;
				timestamp_last = timestamp_current;
				speed_last = speed_current;
				integral_cnt++;
			}
		}
	}
}

void debug_mode(uint16_t rpm) {
	int current_period = START_PERIOD;

	rpm = rpm > 1200 ? 1200: rpm;

	int micro_second = rpm_to_period(rpm);
	int offset = micro_second >= 1000 ? 3 : 1;
	int aim = micro_second - offset;

	int minuend = 1;
	int running = 1;

	uint32_t steps_current = 0;
	uint32_t steps_last = 0;

	uint32_t velocity_rpm_current = 0;
	uint32_t velocity_rpm_last = 0;

	__HAL_TIM_SetCounter(&htim5, 0);
	uint32_t timestamp_current = __HAL_TIM_GetCounter(&htim5);
	uint32_t timestamp_last = 0;

	int32_t speed_current = 0;
	int32_t speed_last = 0;

	uint8_t count_flag = 1;
	uint8_t hit_max = 0;
	uint8_t is_max = 0;

	uint32_t integral_cnt = 1;
	uint32_t integral = 0;

	uint32_t keep_max = 0;
	while(running) {
		while(pulse_flag&(1<<0)) {
			toggle_half_step();
			//reset interrupt flag
			set_pulse_flag(0);
			//toggle step count flag -> 2 halfsteps = 1 step
			count_flag ^= 1UL << 0;

			//only do stuff when full step was executed
			if(count_flag&(1<<0))continue;

			if(integral_flag&(1<<0)) {

				//calculate
				if(!is_max) current_period = current_period - minuend <= aim ? aim : current_period - minuend;
				set_integral_flag(0);
				__HAL_TIM_SET_AUTORELOAD(&htim2, current_period);
				set_pulse_flag(0);

				//tweak acceleration depending on period and torque
				if(hit_max) {
					if(current_period > 400){
						minuend = -10;
					} else
						minuend = -1;
				} else {
					if(current_period > 400){
						minuend = 10;
					} else
						minuend = 1;
				}


				if(aim == current_period && !hit_max){
					__HAL_TIM_SetCounter(&htim5, 0);
					hit_max = 1;
					is_max = 1;
					keep_max = __HAL_TIM_GetCounter(&htim5);

				}


				//break from loop
				if(current_period >= START_PERIOD) running = 0;

				timestamp_current = __HAL_TIM_GetCounter(&htim5);
				steps_current = current_step;

				int32_t step_delta = steps_current - steps_last;
				int32_t periods_per_second = (1000/(INTEGRAL_PERIOD/1000));
				const uint8_t MINUTE_MULTI = 60;

				velocity_rpm_current = (step_delta * periods_per_second * MINUTE_MULTI / STEP_TOTAL_360) / 2; // /2 weil 2 impulse pro schritt
				speed_current = (velocity_rpm_current - velocity_rpm_last);

				integral += step_delta / (INTEGRAL_PERIOD/1000);

				int32_t val = integral / integral_cnt;

#if debug
				printf("velocity:%lu\n\rspeed:%ld\n\rperiod:%lu µs\n\r\n",
						velocity_rpm_current,
						speed_current,
						(timestamp_current - timestamp_last));

				printf("integral: %ld\n\r", val);
				printf("delta steps: %ld\n\r", steps_current - steps_last);
#endif
				velocity_rpm_last = velocity_rpm_current;
				steps_last = steps_current;
				timestamp_last = timestamp_current;
				speed_last = speed_current;
				integral_cnt++;
			}
		}
	}
}


void TIM_StartInterruptTimer()
{
	HAL_TIM_Base_Start_IT(&htim2);
}

void TIM_StartIntegralInterruptTimer()
{
	HAL_TIM_Base_Start_IT(&htim3);
}

void TIM_StartTrackInterruptTimer()
{
	HAL_TIM_Base_Start_IT(&htim5);
}


void TIM_StopInterruptTimer()
{
	HAL_TIM_Base_Start_IT(&htim2);
}


void toggle_half_step() {
	 HAL_GPIO_TogglePin(STEPX_PORT, STEPX_PIN);
	 update_step_count(motor_direction);
}

void HAL_TIM_STEP_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	pulse_flag |= (1<<0);
}

void HAL_TIM_INT_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
	integral_flag |= (1<<0);
}


void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 79;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = START_PERIOD;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}


void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 79;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = INTEGRAL_PERIOD;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_TIM5_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 79;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295; //max val
  	  	  	  	  	//191231209
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

