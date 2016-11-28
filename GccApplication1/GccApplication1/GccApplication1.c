#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
enum MASTERState {INIT,OFF,ON} master_state;
unsigned char counter = 0;
unsigned char toggle_right = 0x00;
unsigned char toggle_left = 0x00;
void MASTER_Init(){
	master_state = INIT;
}

void MASTER_Tick(){
	//Actions
	switch(master_state){
		case INIT:
			PORTD = 0;
			break;
		case OFF:
			PORTD = 1;
			break;
		case ON:
			counter++;
			if(counter >= 20)
			{
				counter = 0;
				PORTA &= 0xFD;
			}
			if(~PINC&0x04)
			{
				toggle_right = ~toggle_right;
			}
			if(~PINC&0x08)
			{
				toggle_left = ~toggle_left;
			}			
			if(toggle_right)
			{
				PORTA |= 0x04;
			}
			else
			{
				PORTA &= 0xFB;
			}
			if(toggle_left)
			{
				PORTA |= 0x08;
			}
			else
			{
				PORTA &= 0xF7;
			}
			break;
	}
	//Transitions
	switch(master_state){
		case INIT:
			master_state = OFF;
			break;
		case OFF:
			if(~PINC&0x10)
			{
				PORTA |= 0x03;
				master_state = ON;
			}
			break;
		case ON:
			if(~PINC&0x01 && ~PINC&0x02)
			{
				PORTA = 0x00;
				master_state = OFF;
			}
			break;
	}
}

void MasterSecTask()
{
	MASTER_Init();
	for(;;)
	{
		MASTER_Tick();
		vTaskDelay(50);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(MasterSecTask, (signed portCHAR *)"MasterSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	DDRC = 0x00; PORTC = 0xFF;
	DDRA = 0xFF; PORTA = 0x00;
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}