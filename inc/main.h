/**
  ******************************************************************************
  * @file    Project/Template/main.h
  ******************************************************************************
  */ 

typedef struct{
	BitStatus switch_pushed;	//set when is pushed, cleared by evaluation
	BitStatus switch_released;	//set when the switch is released
	BitStatus switch_status;	//actual status of the switch
	BitStatus	encA_down;
	BitStatus	encB_down;
	BitStatus	encA_up;
	BitStatus	encB_up;
}type_encoder;

typedef struct{
	BitStatus f1ms;
	BitStatus f5ms;
	BitStatus f10ms;
	BitStatus f25ms;
	BitStatus f50ms;
	BitStatus f100ms;
	BitStatus f250ms;
	BitStatus f500ms;
	BitStatus f1s;
	BitStatus fTimerDots;
}type_timeflags;
	
typedef struct{
	uint8_t digit[4];	//digitos 7-seg con DP, el 4º digito son los leds
	BitStatus L1;			//punto divisor superior
	BitStatus L2;			//punto divisor inferior
	BitStatus L3;			//punto decimal superior
	BitStatus CA1;		//columna digito 1
	BitStatus CA2;		//columna digito 2
	BitStatus CA3;		//columna digito 3
	BitStatus CAL;		//columna leds
	uint8_t cnt_digito;
}type_display;

typedef struct{
	uint8_t status;
	uint8_t status_prev;
}type_device;

typedef struct{
	BitStatus on;
	BitStatus end;
	int16_t	time;
}type_timewindow;

typedef struct{
	int16_t timerSeconds;
	int16_t timerSeconds_recall;
	BitStatus on;
	BitStatus paused;
	BitStatus alarm;
}type_timerstruct;
typedef enum{
	Dado4 = 0,
	Dado6,
	Dado8,
	Dado10,
	Dado12,
	Dado20,
	Dado30,	
	Dado100,
	DadoX	
}enum_dados;

typedef struct{
	int16_t TipoDado;
	int16_t TipoDadoPrevio;
	uint8_t ValorDado;
	uint8_t	ResultadoDado;
}type_dicestruct;

extern volatile type_encoder Encoder;
extern volatile type_timeflags	Timeflags;
extern volatile type_display Display;
extern int16_t* puntero_contador;
extern volatile type_device Device;
extern volatile type_timerstruct Timer;
void NewWindow(uint8_t windowNumber, int16_t time);
void NewWindow_1s(uint8_t windowNumber, int16_t time);

