/**
  ******************************************************************************
  * @file    Project/Template/main.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    07/14/2010
  * @brief   Main program body
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDIN THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 
	
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "main.h"
#include "stm8l15x_it.h"
#include <stdlib.h>

/** @addtogroup Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define tf1ms Timeflags.f1ms
#define tf5ms Timeflags.f5ms
#define tf10ms Timeflags.f10ms
#define tf25ms Timeflags.f25ms
#define tf50ms Timeflags.f50ms
#define tf100ms Timeflags.f100ms
#define tf250ms Timeflags.f250ms
#define tf500ms Timeflags.f500ms
#define tf1s Timeflags.f1s
#define RESET_COLUMNS {GPIO_SetBits(GPIOC,GPIO_Pin_5); \
							GPIO_SetBits(GPIOC,GPIO_Pin_6); \
							GPIO_SetBits(GPIOA,GPIO_Pin_3); \
						GPIO_SetBits(GPIOD,GPIO_Pin_0); }
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

//volatile uint32_t freq_osci = 0x0;
volatile type_encoder Encoder;
volatile type_timeflags	Timeflags;
volatile type_display Display;
volatile type_device Device;
int16_t ContadorVidas[3];	//3 contadores de vidas
int16_t* puntero_contador, puntero_null;
//int16_t TimerSeconds;
volatile type_timerstruct Timer;
volatile type_dicestruct Dado;
const uint8_t To7SEG[10] = {0xFC,0x60,0xDA,0xF2,0x66,0xB6,0xBE,0xE0,0xFE,0xE6};	//conversor a 7 seg abcdefg/dp
const uint8_t Char_F = 0x8E;
const uint8_t Char_T = 0x1E;
const uint8_t Char_I = 0x0C;
const uint8_t Char_M = 0xEC;
const uint8_t Char_D = 0x7A;
const uint8_t Char_C = 0x9C;
const uint8_t Char_U = 0x7C;


static BitStatus startup = 1;
static uint8_t cnt_beeps = 0;
static uint8_t cnt_grupo_beeps = 0;
enum{
	wndBeep,
	wndToOff,
	wndTimer,
	wndDice,
	wndNoBeep,
	wndEfecto,
	wndLast
}enum_windows;

enum{
	wndTimercnt, //countdown del timer
	wndLast_1s
}enum_windows_1s;

volatile type_timewindow TimeWindow[wndLast+1];
volatile type_timewindow TimeWindow_1s[wndLast_1s+1];

/* Private function prototypes ---------------------------*/
void GPIO_Configuration(void);															// inicia puertos
u32 LSIMeasurement(void);																		// mide el oscilador para calibrar el beeper
void InitialiseSystemClock(void);														// inicia clocks de perifericos
void Timer4Init(void);																			// inicia el timer base de tiempos
void UpdateDisplay(void);																		// actualiza los pines del display en base a la estructura Display
void SysOp(void);																						// máquina de estados
void RellenaDisplay(int16_t valor);													// rellena los campos de display en función del valro pasado, hace la conversión a 7seg
void UpdateWindows(void);																		// recorre las ventanas de tiempo y descuenta 
void UpdateWindows_1s(void);																// recorre las ventanas de tiempo de 1s y descuenta 
void EvalWindows(void);																			// evalua si se han cumplido las ventanas
void EvalWindows_1s(void);																	// evalua si se han cumplido las ventanas
void BeepShot(uint8_t mode);																// realiza un bip de duración y tono determinado por el modo
void CheckOff(void);																				// comprueba si se ha pulsado para ir a off
void RellenaDisplayTiempo(int16_t TimerIn);									// muestra el valor del tiempo en la pantalla
void EndTimerBeep(void);																		// genera los beeps para indicar fin del timer

/* Private functions -------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
void main(void)
{
	InitialiseSystemClock();
	/* Configuration of the GPIO*/
	GPIO_Configuration();

	BEEP_LSICalibrationConfig(LSIMeasurement());	
	Timer4Init();
//	BEEP_Init(BEEP_Frequency_1KHz);
//  BEEP_Cmd(ENABLE);
  enableInterrupts();
	Display.cnt_digito = 0;
	Device.status = 0;
	Device.status_prev = 0xff;
	ContadorVidas[0] = 20;
	ContadorVidas[1] = 20;
	ContadorVidas[2] = 20;
	Timer.timerSeconds = 60;	//tiempo por defecto
	BeepShot(0);
	startup = SET;
	Dado.TipoDado = Dado6;
  /* Infinite loop */
  while (1)
  {
		/*
		if(GPIOB->IDR & 0x02)
			Encoder.switch_status = SET;
		else
			Encoder.switch_status = RESET;
		*/

		if(tf1ms  == SET){
			//GPIO_ToggleBits(GPIOA,GPIO_Pin_2); //TX
			UpdateDisplay();
			tf1ms  = RESET;
		}
		if(tf5ms == SET){
			/*
			if(Encoder.switch_pushed ==SET){
				BEEP_Init(BEEP_Frequency_1KHz);
				BEEP_Cmd(ENABLE);
				Encoder.switch_pushed = RESET;
			}
			if(Encoder.switch_released == SET ){
				BEEP_Cmd(DISABLE);
				Encoder.switch_released = RESET;			
			}
			*/
			tf5ms = RESET;
		}
		
		if(tf10ms == SET){
			UpdateWindows();
			EvalWindows();
			tf10ms = RESET;
		}
		
		if(tf25ms == SET){
			tf25ms = RESET;
		}
		
		if(tf50ms == SET){
			SysOp();
			tf50ms = RESET;
		}
		
		if(tf100ms == SET){			
			
			tf100ms = RESET;
		}
		
		if(tf250ms == SET){
			
			tf250ms = RESET;
		}
		
		if(tf500ms == SET){
			/*
			GPIO_ToggleBits(GPIOC,GPIO_Pin_4);	//A/L1
			GPIO_ToggleBits(GPIOC,GPIO_Pin_1);	//B/L2
			GPIO_ToggleBits(GPIOC,GPIO_Pin_0);	//C/L3
			GPIO_ToggleBits(GPIOB,GPIO_Pin_7);	//D	
			GPIO_ToggleBits(GPIOB,GPIO_Pin_6);	//E
			GPIO_ToggleBits(GPIOB,GPIO_Pin_5);	//F	
			GPIO_ToggleBits(GPIOB,GPIO_Pin_4);	//G	
			GPIO_ToggleBits(GPIOB,GPIO_Pin_3);	//DP	
			*/
			tf500ms = RESET;
		}

		if(tf1s == SET){			
			UpdateWindows_1s();																		// MAAAAL, al arrancar la ventana no sabes cuanto s faltan para actualizar
			EvalWindows_1s();
			tf1s = RESET;
		}
  }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

void GPIO_Configuration(void){
	/* PORT A */
	//GPIO_Init(GPIOA,GPIO_Pin_0,GPIO_Mode_Out_PP_Low_Fast);	// beeper, alternate function
	GPIO_Init(GPIOA,GPIO_Pin_1,GPIO_Mode_Out_PP_High_Slow);		// Reset, On/off switch
	GPIO_Init(GPIOA,GPIO_Pin_2,GPIO_Mode_Out_PP_High_Fast);		// Tx
	GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_Out_PP_High_Fast);		// CA2
	GPIO_Init(GPIOD,GPIO_Pin_0,GPIO_Mode_Out_PP_High_Fast);		// CA3
	
	EXTI_SetPinSensitivity(EXTI_Pin_0,EXTI_Trigger_Rising_Falling);
	GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_In_FL_IT);						// ENC_A
	
	EXTI_SetPinSensitivity(EXTI_Pin_1,EXTI_Trigger_Rising_Falling);
	GPIO_Init(GPIOB,GPIO_Pin_1,GPIO_Mode_In_FL_IT);						// SWITCH_IN
	
	EXTI_SetPinSensitivity(EXTI_Pin_2,EXTI_Trigger_Rising_Falling);
	GPIO_Init(GPIOB,GPIO_Pin_2,GPIO_Mode_In_FL_IT);						// ENC_B
	
	GPIO_Init(GPIOB,GPIO_Pin_3,GPIO_Mode_Out_PP_High_Fast);		// DP
	GPIO_Init(GPIOB,GPIO_Pin_4,GPIO_Mode_Out_PP_High_Fast);		// G
	GPIO_Init(GPIOB,GPIO_Pin_5,GPIO_Mode_Out_PP_High_Fast);		// F
	GPIO_Init(GPIOB,GPIO_Pin_6,GPIO_Mode_Out_PP_High_Fast);		// E
	GPIO_Init(GPIOB,GPIO_Pin_7,GPIO_Mode_Out_PP_High_Fast);		// D
	GPIO_Init(GPIOC,GPIO_Pin_0,GPIO_Mode_Out_PP_High_Fast);		// C_L3
	GPIO_Init(GPIOC,GPIO_Pin_1,GPIO_Mode_Out_PP_High_Fast);	 	// B_L2
	GPIO_Init(GPIOC,GPIO_Pin_4,GPIO_Mode_Out_PP_High_Fast);		// A_L1
	GPIO_Init(GPIOC,GPIO_Pin_5,GPIO_Mode_Out_PP_High_Fast);		// CAL
	GPIO_Init(GPIOC,GPIO_Pin_6,GPIO_Mode_Out_PP_High_Fast);		// CA1
	
	RST_GPOutputEnable();
	GPIO_SetBits(GPIOA,GPIO_Pin_1);//Reset, On/off switch
	
}


/**
  * @brief Measure the LSI frequency using timer IC1 and update the calibration registers.
  * @par Parameters:
  * None
  * @retval 
	* None
  * @par Required preconditions:
  * It is recommanded to use a timer clock frequency of at least 10MHz in order 
	* to obtain a better in the LSI frequency measurement.
  */
u32 LSIMeasurement(void)
{

  u32 lsi_freq_hz = 0x0;
  u32 fmaster = 0x0;
	uint16_t ICValue1 = 0x0;
	uint16_t ICValue2 = 0x0;	
	
  BEEP_Cmd(DISABLE);
	CLK_BEEPClockConfig(CLK_BEEPCLKSource_LSI);

  /* Get master frequency */
  fmaster = CLK_GetClockFreq();

  /* Enable the LSI measurement: LSI clock connected to timer Input Capture 1 */
	BEEP_LSClockToTIMConnectCmd(ENABLE);											// Measure the LSI clock frequency	//AWU->CSR |= AWU_CSR_MSR;

	/* Measure the LSI frequency with TIMER Input Capture 1 */

  /* Capture only every 8 events!!! */
  /* Enable capture of TI1 */
  TIM2_ICInit(TIM2_Channel_1, TIM2_ICPolarity_Rising, TIM2_ICSelection_DirectTI, TIM2_ICPSC_DIV8, 0);

  /* Enable TIM2 */
  TIM2_Cmd(ENABLE);
	/* wait a capture on cc1 */
  while ((TIM2->SR1 & TIM2_FLAG_CC1) != TIM2_FLAG_CC1);
	/* Get CCR1 value*/
  ICValue1 = TIM2_GetCapture1();
  TIM2_ClearFlag(TIM2_FLAG_CC1);

  /* wait a capture on cc1 */
  while ((TIM2->SR1 & TIM2_FLAG_CC1) != TIM2_FLAG_CC1);
    /* Get CCR1 value*/
  ICValue2 = TIM2_GetCapture1();
	TIM2_ClearFlag(TIM2_FLAG_CC1);

  /* Disable IC1 input capture */
  TIM2->CCER1 &= (u8)(~TIM_CCER1_CC1E);
  /* Disable timer3 */
  TIM2_Cmd(DISABLE);

  /* Compute LSI clock frequency */
  lsi_freq_hz = (8 * fmaster) / (ICValue2 - ICValue1);
  
  /* Disable the LSI measurement: LSI clock disconnected from timer Input Capture 1 */
 BEEP_LSClockToTIMConnectCmd(DISABLE);											//Measure the LSI clock frequency	//AWU->CSR &= (u8)(~AWU_CSR_MSR);

 return (lsi_freq_hz);
}

//--------------------------------------------------------------------------------
//
//  Setup the system clock to run at 16MHz using the internal oscillator.
//
void InitialiseSystemClock()
{
	
	CLK_HSICmd(ENABLE);
	CLK->ECKCR = 0;                       										//  Disable the external clock.
	while (CLK_GetFlagStatus(CLK_FLAG_HSIRDY) == 0);       		//  Wait for the HSI to be ready for use.
	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);

/*	CLK->PCKENR1 = 0xff;                 										//  Enable all peripheral clocks.
	CLK->PCKENR2 = 0xff;                 											//  Ditto.
*/
	CLK_PeripheralClockConfig(CLK_Peripheral_BEEP,ENABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM2,ENABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4,ENABLE);
	
	CLK->CCOR = 0;                       											//  Turn off CCO.
	CLK->HSITRIMR = 0;                   											//  Turn off any HSIU trimming.
	//CLK->SWIMCCR = 0;                    										//  Set SWIM to run at clock / 2.
	CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
	//CLK_SWR = 0xe1;                     										//  Use HSI as the clock source.
	CLK->SWCR = 0;                       											//  Reset the clock switch control register.
	CLK->SWCR |= CLK_SWCR_SWEN;                  							//  Enable switching.
	while ((CLK->SWCR & CLK_SWCR_SWBSY) != 0);        				//  Pause while the clock switch is busy.
	
	/* Get master frequency */
 // freq_osci= CLK_GetClockFreq();
}
void Timer4Init(void){
	CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
	TIM4_TimeBaseInit(TIM4_Prescaler_64,0xFA);								//a6mh/64 -> 4useg ->250 es 1mseg
	TIM4_ITConfig(TIM4_IT_Update,ENABLE);
	TIM4_Cmd(ENABLE);
}

void UpdateDisplay(void){
	RESET_COLUMNS
	GPIOC->ODR |= 0x13; 																			//borro bits de 7seg
	GPIOB->ODR |= 0xF8; 
	switch(Display.cnt_digito){
		case 0:																									//centenas
			GPIO_ResetBits(GPIOC,GPIO_Pin_6);
			break;
		case 1:																									//decenas
			GPIO_ResetBits(GPIOA,GPIO_Pin_3); 
			break;
		case 2:																									//unidades
			GPIO_ResetBits(GPIOD,GPIO_Pin_0); 
			break;
		case 3:																									//leds
			GPIO_ResetBits(GPIOC,GPIO_Pin_5); 
			break;
	}
			GPIOC->ODR &= ~((Display.digit[Display.cnt_digito] & 0x80) >> 3);	//	A/L1 
			GPIOC->ODR &= ~((Display.digit[Display.cnt_digito] & 0x40) >> 5);	//	B/L2
			GPIOC->ODR &= ~((Display.digit[Display.cnt_digito] & 0x20) >> 5);	//	C/L3
			GPIOB->ODR &= ~((Display.digit[Display.cnt_digito] & 0x10) << 3);	//	D/DL2
			
			GPIOB->ODR &= ~((Display.digit[Display.cnt_digito] & 0x08) << 3);	//	E/DL3
			GPIOB->ODR &= ~((Display.digit[Display.cnt_digito] & 0x04) << 3);	//	F/DL4
			GPIOB->ODR &= ~((Display.digit[Display.cnt_digito] & 0x02) << 3);	//	G/DL5
			GPIOB->ODR &= ~((Display.digit[Display.cnt_digito] & 0x01) << 3);	//	DP/DL6
			
			if(Display.cnt_digito < 3)
				Display.cnt_digito++;
			else 
				Display.cnt_digito = 0;

}

void SysOp(void){																						//modos de funcionamiento del sispositivo
	switch(Device.status){
		case 0:
		if(Device.status_prev != 0){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x10;															//DL2
			puntero_contador = &ContadorVidas[0];
			TimeWindow[wndToOff].on = RESET;
		}
		if(startup == SET && Encoder.switch_released == SET){		//se comde el primer released para que no cambia de estado al arrancar con el switch
			Encoder.switch_released = RESET;
			startup = RESET;
		}		
		RellenaDisplay(ContadorVidas[0]);
		CheckOff();
		
		if(startup == RESET && Encoder.switch_released == SET){
			Device.status++;
			puntero_contador = &puntero_null;
		}	
		Device.status_prev = 0;
		break;

		case 1:
		if(Device.status_prev != 1){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x08;															//DL3
			puntero_contador = &ContadorVidas[1];
			TimeWindow[wndToOff].on = RESET;			
		}
		
		RellenaDisplay(ContadorVidas[1]);
		CheckOff();

		if(Encoder.switch_released == SET){
			Device.status++;
			puntero_contador = &puntero_null;
		}
		Device.status_prev = 1;
		break;
		
		case 2:
		if(Device.status_prev != 2){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x04;															//DL4
			puntero_contador = &ContadorVidas[2];		
			TimeWindow[wndToOff].on = RESET;
		}
		
		RellenaDisplay(ContadorVidas[2]);
		CheckOff();
		
		if(Encoder.switch_released == SET){
			Device.status++;
			puntero_contador = &puntero_null;
		}
		Device.status_prev = 2;
		break;
		
		/* TIMER */
		case 3:																									// Timer
		if(Device.status_prev != 3){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x02;															// DL5
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].time = 0;
			NewWindow(wndTimer,100);															// 1 seg mostrando TIM
			puntero_contador = &Timer.timerSeconds;
		}
		if(TimeWindow[wndTimer].on == SET){  										// estoy mostrando TIM
			Display.digit[0] = Char_T;
			Display.digit[1] = Char_I;
			Display.digit[2] = Char_M;			
		}else{ 																									// ya he entrado en modo TIM, muestro otra cosa
			if(TimeWindow[wndToOff].end == RESET && 
			(TimeWindow[wndToOff].on == RESET || 
			TimeWindow[wndToOff].time > 150 )){
				RellenaDisplayTiempo(Timer.timerSeconds);						// muestro tiempo 
				
			}else{																								// muestro out
				Display.digit[0] = To7SEG[0];	
				Display.digit[1] = Char_U;
				Display.digit[2] = Char_T;
				Display.digit[3] &= 0x3F;														// borro puntos de timer				
			}
			
		}
		CheckOff();
		if (TimeWindow[wndToOff].on == SET && 
		TimeWindow[wndToOff].time <= 150 && 
		Encoder.switch_released == SET){												// por debajo de 150 hago un out
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].end = RESET;
			Device.status++;
			//startup = SET;																			// voy a estado dice sin haber soltado, para que no interprete el primer release
		}else	if(TimeWindow[wndTimer].on == RESET && 
		TimeWindow[wndToOff].time > 150 && 
		Encoder.switch_released == SET && 
		TimeWindow[wndToOff].on == SET){ 												// se ha soltado antes de cumplir la ventana, pulsación normal para iniciar o parar timer
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].time = 0;
			if(Timer.timerSeconds > 0){ 													// arranco o paro el temporizador
			 if(Timer.on == RESET){																// activo por primera vez
					Timer.paused = RESET;
					Timer.on = SET;
					Timer.timerSeconds_recall = Timer.timerSeconds;
					NewWindow_1s(wndTimercnt,Timer.timerSeconds);			// 1 seg para restar
					system_timer1s = 0;																// reseteo contador de la interrupción
				}else{																							// ya activado, a pausar o despausar
					if(Timer.paused == RESET){
						TimeWindow_1s[wndTimercnt].on = RESET;					// pauso la ventana
						Timer.paused = SET;
					}else{
						if(Timer.timerSeconds != TimeWindow_1s[wndTimercnt].time){	// se ha modificado el tiempo
							TimeWindow_1s[wndTimercnt].time = Timer.timerSeconds;
							system_timer1s = 0;															// reseteo contador de la interrupción
						}
						TimeWindow_1s[wndTimercnt].on = SET;						// retomo la ventana
						Timer.paused = RESET;
					}
						
				}											
			}else{
					Timer.timerSeconds = Timer.timerSeconds_recall;		//recargo el tiempo original
			}
		}		

		if(TimeWindow[wndTimer].on == SET){
			if(Encoder.switch_released == SET){
				Device.status++;
				TimeWindow[wndToOff].end = RESET;
				TimeWindow[wndToOff].on = RESET;
				TimeWindow[wndTimer].on = RESET;
				puntero_contador = &puntero_null;
			}
		}else{
			if(TimeWindow[wndTimer].end == SET){
				Encoder.switch_pushed = RESET;
				TimeWindow[wndTimer].end = RESET;
			}				
		}
		Device.status_prev = 3;
		break;
		
		/* DICE */
		case 4:																									//Dice
		if(Device.status_prev != 4){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x01;															//DL6
			TimeWindow[wndToOff].on = RESET;
			NewWindow(wndDice,100);																//1 seg mostrando dic		
			Dado.TipoDado = Dado6;
			Dado.ValorDado = 6;
			Dado.ResultadoDado = Dado.ValorDado ;
			puntero_contador = &Dado.TipoDado;
		}

		if(TimeWindow[wndDice].on == SET){
			Display.digit[0] = Char_D;
			Display.digit[1] = Char_I;
			Display.digit[2] = Char_C;			
		}else{
		
			if(TimeWindow[wndToOff].end == RESET && 
			(TimeWindow[wndToOff].on == RESET || 
			TimeWindow[wndToOff].time > 150 )){										// me quedo en modo dado					
				switch(Dado.TipoDado){
					case Dado4:
						if(Dado.TipoDadoPrevio != Dado4){
							srand(system_timer);
							Dado.ResultadoDado = 4;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado4;
					break;
					case Dado6:
						if(Dado.TipoDadoPrevio != Dado6){
							srand(system_timer);
							Dado.ResultadoDado = 6;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado6;
					break;
					case Dado8:
						if(Dado.TipoDadoPrevio != Dado8){
							srand(system_timer);
							Dado.ResultadoDado = 8;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado8;
					break;
					case Dado10:
						if(Dado.TipoDadoPrevio != Dado10){
							srand(system_timer);
							Dado.ResultadoDado = 10;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado10;
					break;
					case Dado12:
						if(Dado.TipoDadoPrevio != Dado12){
							srand(system_timer);
							Dado.ResultadoDado = 12;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado12;
					break;
					case Dado20:
						if(Dado.TipoDadoPrevio != Dado20){
							srand(system_timer);
							Dado.ResultadoDado = 20;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado20;
					break;
					case Dado30:
						if(Dado.TipoDadoPrevio != Dado30){
							srand(system_timer);
							Dado.ResultadoDado = 30;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado30;
					break;
					case Dado100:
						if(Dado.TipoDadoPrevio != Dado100){						
							srand(system_timer);
							Dado.ResultadoDado = 100;
							Dado.ValorDado = Dado.ResultadoDado;
						}
						Dado.TipoDadoPrevio = Dado100;
					break;
					default:
						Dado.TipoDado = Dado4;
						Dado.TipoDadoPrevio = 0xFF;
					break;
				}
				if(TimeWindow[wndEfecto].on == SET){
					Dado.ResultadoDado = ((rand()%Dado.ValorDado) + 1);					
					BeepShot(2);
				}
				RellenaDisplay(Dado.ResultadoDado);						
				
			}else{																								//muestro out
				Display.digit[0] = To7SEG[0];	
				Display.digit[1] = Char_U;
				Display.digit[2] = Char_T;
			}	
		}
		/*
		if(startup == SET && Encoder.switch_released == SET){		//se comde el primer released para que no cambia de estado al arrancar con el switch
			Encoder.switch_released = RESET;
			startup = RESET;
		}	*/
		
		CheckOff();
		
		if (TimeWindow[wndToOff].on == SET && 
		TimeWindow[wndToOff].time <= 150 && 
		Encoder.switch_released == SET)
		{
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].end = RESET;
			Device.status = 0;
			//startup = SET;			//voy a estado dice sin haber soltado, para que no interprete el primer release
		}else	if(TimeWindow[wndDice].on == RESET && TimeWindow[wndToOff].time > 150 && Encoder.switch_released == SET && TimeWindow[	wndToOff].on == SET){ //se ha soltado antes de cumplir la ventana			
			if(Encoder.switch_released == SET){
				NewWindow(wndEfecto,50);
				//Dado.ResultadoDado = ((rand()%Dado.ValorDado) + 1);
			}
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].time = 0;
		}		
		
		/*
		if(startup == RESET && TimeWindow[wndDice].on == RESET && Encoder.switch_released == SET && TimeWindow[wndToOff].on == SET){ //se ha soltado antes de cumplir la ventana
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;
			TimeWindow[wndToOff].on = RESET;
		}

		if(startup == RESET && TimeWindow[wndDice].on == RESET && Encoder.switch_pushed == SET && TimeWindow[wndToOff].on == RESET && TimeWindow[wndToOff].end == RESET){		
			NewWindow(wndToOff, 50);	//espero 50mseg para salir
		}else if (TimeWindow[wndToOff].end == SET){
			TimeWindow[wndToOff].on = RESET;
			TimeWindow[wndToOff].end = RESET;	
			startup = SET;			//voy a estado 0 sin haber soltado, para que no interprete el primer release
			Device.status=0;
		}		
		*/
		if(startup == RESET && Encoder.switch_released == SET){
			Device.status=0;
			Encoder.switch_released = RESET;
			puntero_contador = &puntero_null;			
		}
		Device.status_prev = 4;
		break;
		
		case 5:	//off
		/*
		if(Device.status_prev != 5){
			Encoder.switch_released = RESET;
			Encoder.switch_pushed = RESET;			
			Display.digit[3] = 0x01;	//DL6
		}*/
		Display.digit[0] = To7SEG[0];
		Display.digit[1] = Char_F;
		Display.digit[2] = Char_F;
		Display.digit[3] &= 0x3F;	//borro puntos de timer
		
		if(Encoder.switch_released == SET){
			GPIO_ResetBits(GPIOA,GPIO_Pin_1);//Reset, On/off switch
			//Device.status=0;
		}
		Device.status_prev = 5;
		break;
	}
}

void RellenaDisplay(int16_t valor){
	uint16_t centenas, decenas, unidades;
	//if(TimeWindow[wndEfecto].on == RESET){
		if(valor >= 0){
			centenas = valor / 100;
			decenas = (valor % 100) / 10;
			unidades = valor % 10;
			if(centenas>0)
				Display.digit[0] = To7SEG[centenas];
			else
				Display.digit[0] = 0;
			if(valor>9)	
				Display.digit[1] = To7SEG[decenas];
			else{
				Display.digit[1] = 0;
				Display.digit[0] = 0;
			}
		}else{
			valor = -valor;
			centenas = valor / 100;
			decenas = (valor % 100) / 10;
			unidades = valor % 10;
			if(valor>9){	
				Display.digit[1] = To7SEG[decenas];
				Display.digit[0] = 0x02;		//menos
			}else{
				Display.digit[0] = 0;
				Display.digit[1] = 0x02;		//menos
			}
		}
//	}
	if(Timeflags.fTimerDots == 0 && Timer.on == SET && Timer.paused == RESET )	
		Display.digit[2] = To7SEG[unidades] | 0x01;	//le añado el punto		
	else
		Display.digit[2] = To7SEG[unidades] & 0xFE;	//le quito el punto			
		
}

void RellenaDisplayTiempo(int16_t TimerIn){
	uint16_t minutos,decenas, unidades;
	minutos = TimerIn / 60;
	decenas = ((TimerIn % 60) / 10);
	if(minutos <= 9) {
		Display.digit[0] = To7SEG[minutos];
		
		unidades = ((TimerIn % 60) % 10);
		Display.digit[1] = To7SEG[decenas];
		Display.digit[2] = To7SEG[unidades];
		if(Timeflags.fTimerDots == 0 || Timer.paused == SET || Timer.on == RESET)
			Display.digit[3] |= 0xC0;		
		else
			Display.digit[3] &= ~0xC0;	//borro los dos puntos de tiempo
	}else{
		Display.digit[0] = To7SEG[minutos/10];
		if(Timeflags.fTimerDots == 0 || Timer.paused == SET || Timer.on == RESET)
			Display.digit[1] = To7SEG[minutos%10] | 0x01;	//le añado el punto
		else
			Display.digit[1] = To7SEG[minutos%10] & 0xFE;	//le quito el punto
		
		Display.digit[2] = To7SEG[decenas];		
		Display.digit[3] &= ~0xC0;	//borro los dos puntos de tiempo
	}
	
/*
if(valor >= 0){
		centenas = valor / 100;
		decenas = (valor % 100) / 10;
		unidades = valor % 10;
		if(centenas>0)
			Display.digit[0] = To7SEG[centenas];
		else
			Display.digit[0] = 0;
		if(valor>9)	
			Display.digit[1] = To7SEG[decenas];
		else{
			Display.digit[1] = 0;
			Display.digit[0] = 0;
		}
		Display.digit[2] = To7SEG[unidades];		
	}else{
		valor = -valor;
		centenas = valor / 100;
		decenas = (valor % 100) / 10;
		unidades = valor % 10;
		if(valor>9){	
			Display.digit[1] = To7SEG[decenas];
			Display.digit[0] = 0x02;		//menos
		}else{
			Display.digit[0] = 0;
			Display.digit[1] = 0x02;		//menos
		}
		Display.digit[2] = To7SEG[unidades];		
		
	}*/
}
void NewWindow(uint8_t windowNumber, int16_t time){
	TimeWindow[windowNumber].on = SET;
	TimeWindow[windowNumber].end = RESET;
	TimeWindow[windowNumber].time = time;	
}
void NewWindow_1s(uint8_t windowNumber, int16_t time){
	TimeWindow_1s[windowNumber].on = SET;
	TimeWindow_1s[windowNumber].end = RESET;
	TimeWindow_1s[windowNumber].time = time;	
}
void UpdateWindows(void){
	uint8_t cnt_w=0;
	
	for(cnt_w=0; cnt_w < wndLast; cnt_w++){
		if(TimeWindow[cnt_w].on == SET){
			TimeWindow[cnt_w].time--;
			if(TimeWindow[cnt_w].time == 0){
				TimeWindow[cnt_w].on = RESET;
				TimeWindow[cnt_w].end = SET;
			}
		}
	}
}
void UpdateWindows_1s(void){
	uint8_t cnt_w=0;
	
	for(cnt_w=0; cnt_w < wndLast_1s; cnt_w++){
		if(TimeWindow_1s[cnt_w].on == SET){
			TimeWindow_1s[cnt_w].time--;
			if(TimeWindow_1s[cnt_w].time == 0){
				TimeWindow_1s[cnt_w].on = RESET;
				TimeWindow_1s[cnt_w].end = SET;
			}
		}
	}
}
void EvalWindows(void){
	if(TimeWindow[wndBeep].end == SET){
		TimeWindow[wndBeep].end = RESET;
		BEEP_Cmd(DISABLE);
		if(cnt_beeps>0){
			NewWindow(wndNoBeep,10); 
			cnt_beeps--;
		}
	}
	if(TimeWindow[wndNoBeep].end == SET){
		TimeWindow[wndNoBeep].end = RESET;
		if(cnt_beeps>0){
			BeepShot(3);		
		}else if(cnt_grupo_beeps>0){
			cnt_beeps = 3;
			NewWindow(wndNoBeep,50); 	
			cnt_grupo_beeps--;
		}
	}
}
void EvalWindows_1s(void){
	if(Timer.on == SET && Timer.paused == RESET){
		if(TimeWindow_1s[wndTimercnt].end == RESET){
				Timer.timerSeconds = TimeWindow_1s[wndTimercnt].time;
		}else{			
			Timer.timerSeconds = 0;
			TimeWindow_1s[wndTimercnt].end = RESET;
			Timer.alarm = SET;
			Timer.on = RESET;
			EndTimerBeep();
		}
	}
}

void BeepShot(uint8_t mode){
	int16_t time = 0;
	BEEP_Frequency_TypeDef BEEP_Frequency;
	switch(mode){
		case 0:
			time = 2;
			BEEP_Frequency = BEEP_Frequency_1KHz;
		case 1:
			time = 2;
			BEEP_Frequency = BEEP_Frequency_2KHz;
		case 2:
			time = 2;
			BEEP_Frequency = BEEP_Frequency_4KHz;
		break;
		case 3:
			time = 10;
			BEEP_Frequency = BEEP_Frequency_1KHz;
		break;		
	}
	BEEP_Init(BEEP_Frequency);
	BEEP_Cmd(ENABLE);
	NewWindow(wndBeep,time);
}
void EndTimerBeep(void){
	cnt_grupo_beeps = 2; 
	cnt_beeps = 3;
	BeepShot(3);
}
void CheckOff(void){
	if(Encoder.switch_pushed == SET && TimeWindow[wndToOff].on == RESET && TimeWindow[wndToOff].end == RESET){
		NewWindow(wndToOff, 200);
	}else if (TimeWindow[wndToOff].end == SET){
		Device.status = 5;
	}		
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/