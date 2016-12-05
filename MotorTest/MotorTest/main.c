#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "usart_ATmega1284.h"
#include <util/delay.h>
enum MotorState {INIT,Wait, Pull, Reverse} motor_state;

unsigned short pullTimer = 0;
unsigned short reverseTimer = 0;
unsigned char temp;
unsigned char password[] = {'1','3','3','7'};
unsigned char password_count = 0;

void Motor_Init(){
	motor_state = INIT;
}

void Motor_Tick(){
	//Actions
	switch(motor_state){
		case INIT:
			break;
		case Wait:
			break;
		case Pull:
			PORTA = 0x01;
			_delay_ms(1);
			PORTA = 0x00;
			_delay_ms(1);
			pullTimer++;
			break;
		case Reverse:
			PORTA = 0x03;
			_delay_ms(1);
			PORTA = 0x02;
			_delay_ms(1);
			reverseTimer++;
		break;
	}
	//Transitions
	switch(motor_state){
		case INIT:
		motor_state = Wait;
		break;
		case Wait:
		if(USART_HasReceived(0))
		{
			temp = USART_Receive(0);
			if(temp == 0x01)
			{
				PORTB = 0x01;
				temp = USART_HasReceived(0);
				if(temp == 1)
					motor_state = Pull;
				if(temp == 2)
					motor_state = Reverse;
				USART_Flush(0);
			}
			PORTB = 0x00;
			break;
		case Pull:
			if(pullTimer == 5000)
				motor_state = Pull;
				PORTB = 0x01;
			}
			if(temp == 0x02)
			{
				motor_state = Reverse;
				PORTB = 0x02;
			}
			USART_Flush(0);
		}
		break;
		case Pull:
		if(pullTimer == 1500)
		{
			pullTimer = 0;
			PORTB = 0x00;
			motor_state = Wait;
		}
		break;
		case Reverse:
		if(reverseTimer == 1500)
		{
			reverseTimer = 0;
			PORTB = 0x00;
			motor_state = Wait;
		}
		break;
	}
}

void MotorSecTask()
{
	Motor_Init();
	for(;;)
	{
		Motor_Tick();
		vTaskDelay(1);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(MotorSecTask, (signed portCHAR *)"MotorSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	DDRA = 0xFF; PORTA=0x00;
	DDRB = 0xFF; PORTB=0x00;
	DDRD = 0x00; PORTD = 0xFF;
	DDRA = 0xFF; PORTB = 0x00;
	initUSART(0);
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}
