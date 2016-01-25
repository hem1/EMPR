#include "lpc_types.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pwm.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_systick.h"
#include "lpc_types.h"
#include "lpc17xx_pinsel.h"

#include "string.h"
#include "stdio.h"
#include "Serial.h"
#include "i2cIO.h"
#include "sysTick.h"
//#include "ultraSonicSensor.h"

//p28, P0.10 as trigger for sensor
#define TRIGGER (1<<10)


static uint8_t ECHO_LEVEL = 1; // flag to check state of echo ( )
						//odd for low, even for high .
uint32_t LOW_TO_HIGH_COUNT = 0;		// timer count when echo at low
uint32_t HIGH_TO_LOW_COUNT = 0;		// timer count when echo at high

void PWM_init()
{
	PWM_TIMERCFG_Type PWMCfgDat;
	PWM_MATCHCFG_Type PWMMatchCfgDat;
	PINSEL_CFG_Type PinCfg;

	//p24 as PWM1.3 output. P2.2 in embed

	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	
	PinCfg.Funcnum = 1;
	PinCfg.Portnum = 2;
	PinCfg.Pinnum = 2;
	PINSEL_ConfigPin(&PinCfg);

	// initialize PWM timer mode
	PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_USVAL;
	PWMCfgDat.PrescaleValue = 1;
	PWM_Init(LPC_PWM1, PWM_MODE_TIMER, &PWMCfgDat);
	
	/* Set match value for PWM match channel 0 = 5000000uS, total period */
	PWM_MatchUpdate(LPC_PWM1, 0, 60000, PWM_MATCH_UPDATE_NOW);
	/* PWM Timer/Counter will be reset when channel 0 matching
	* no interrupt when match
	 * no stop when match 

	*  PWM Pulse Width is set by the PWMMRO */
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 0;
	PWMMatchCfgDat.ResetOnMatch = ENABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	
	//PWM channel edge option
	PWM_ChannelConfig(LPC_PWM1, 3, PWM_CHANNEL_SINGLE_EDGE);
	
	//Configure match value for match registers
	PWM_MatchUpdate(LPC_PWM1, 3, 10, PWM_MATCH_UPDATE_NOW);
	PWMMatchCfgDat.IntOnMatch = DISABLE;
	PWMMatchCfgDat.MatchChannel = 3;
	PWMMatchCfgDat.ResetOnMatch = DISABLE;
	PWMMatchCfgDat.StopOnMatch = DISABLE;
	PWM_ConfigMatch(LPC_PWM1, &PWMMatchCfgDat);
	
	PWM_ChannelCmd(LPC_PWM1, 3, ENABLE);
	
	PWM_ResetCounter(LPC_PWM1);
	PWM_CounterCmd(LPC_PWM1, ENABLE);
	
	 /* Start PWM now */
	PWM_Cmd(LPC_PWM1, ENABLE);
	
}



void TIMER3_IRQHandler(void)
 {
	
	
	int data, PULSE_WIDTH;
	if (TIM_GetIntCaptureStatus(LPC_TIM3,0))
	{
		TIM_ClearIntCapturePending(LPC_TIM3,0);
		data = TIM_GetCaptureValue(LPC_TIM3,0);

		ECHO_LEVEL += 1;
		if ( ECHO_LEVEL %= 2)
		{
			HIGH_TO_LOW_COUNT = data;
			
			PULSE_WIDTH = (HIGH_TO_LOW_COUNT - LOW_TO_HIGH_COUNT);
			PULSE_WIDTH = PULSE_WIDTH ;
			PULSE_WIDTH = PULSE_WIDTH /58;
			//serialPrint("Pulse Width is : %d\n\r", PULSE_WIDTH);
			serialPrint("HIGH to low %d\r\n",HIGH_TO_LOW_COUNT);

		}
		else
		{
			LOW_TO_HIGH_COUNT = data;
			serialPrint("LOw to High %d\r\n", LOW_TO_HIGH_COUNT);
		}
		//serialPrint("\n\rTimer Value is : %d v\n\r", data);
		
		
		TIM_ResetCounter(LPC_TIM3);
		TIM_Cmd(LPC_TIM3, ENABLE);
	}
}

void SETUP_ULTRA_SOUND()
{


	PINSEL_CFG_Type PinCfg;
	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
	//p15 CAP3.0
        PinCfg.OpenDrain = 0;
        PinCfg.Pinmode = 0;
	PinCfg.Funcnum = 3;
        PinCfg.Portnum = 0;
        PinCfg.Pinnum = 23;
        PINSEL_ConfigPin(&PinCfg);
 
        // Initialize timer 0, prescale count time of 1000000uS = 1S
        TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
        TIM_ConfigStruct.PrescaleValue  = 0;
 	

        // use channel 0, CAPn.0
        TIM_CaptureConfigStruct.CaptureChannel = 0;
        // Enable capture on CAPn.0 rising edge
        TIM_CaptureConfigStruct.RisingEdge = ENABLE;
        // Enable capture on CAPn.0 falling edge
        TIM_CaptureConfigStruct.FallingEdge = ENABLE;
        // Generate capture interrupt
        TIM_CaptureConfigStruct.IntOnCaption = ENABLE;

         // Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM3,TIM_TIMER_MODE,&TIM_ConfigStruct);
        TIM_ConfigCapture(LPC_TIM3, &TIM_CaptureConfigStruct);
        TIM_ResetCounter(LPC_TIM3);
 
 
        /* preemption = 1, sub-priority = 1 */
        NVIC_SetPriority(TIMER3_IRQn, ((0x01<<3)|0x01));
        /* Enable interrupt for timer 3 */
        NVIC_EnableIRQ(TIMER3_IRQn);
        // To start timer 3
        TIM_Cmd(LPC_TIM3,ENABLE);

}



void fire()
{
	
	//LEDdebug(0b1000);
	PWM_init();
	
	SETUP_ULTRA_SOUND();
	
	
	
	
}
