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
#include "usart_ATmega1284.h"
#include "keypad.h"
#include "lcd.h"

enum MASTERState {INIT,OFF,ON} master_state;

unsigned char toggle_right = 0x00;
unsigned char toggle_left = 0x00;
unsigned char toggle_magnet = 0x00;
unsigned char signal = 0x00;
unsigned char password[] = {'1','3','3','7'};
unsigned char password_count = 0;
unsigned char inputed_password[4] = {};
unsigned char password_correct_status = 0xFF;
unsigned char key;
unsigned char attempts = 0;
unsigned char steps = 0;
unsigned char steps_1_place = '0';
unsigned char steps_10_place = '0';
unsigned char steps_100_place = '0';
unsigned char timer = 0;
unsigned char steps_temp;

void MASTER_Init(){
	master_state = INIT;
}

void MASTER_Tick(){
	//Actions
	switch(master_state){
		case INIT:
			break;
		case OFF:
			signal = 0x00;
			if(password_count >= 4)
			{
				attempts++;
				password_count = 0;
				password_correct_status = 0xFF;
				LCD_DisplayString(1,"Incorrect! Try again: ");
			}
			key = GetKeypadKey();
			if(key != '\0')
			{
				if(attempts == 0)
				{
					LCD_DisplayString(1,"Enter Password:  ");
				}
				else
				{
					LCD_DisplayString(1,"Incorrect! Try again: ");
				}
				inputed_password[password_count] = key;
				if(key != password[password_count])
				{
					password_correct_status = 0x00;
				}
				password_count++;
				PORTB = key;
				for(int i = 0; i < password_count; i++)
				{
					LCD_WriteData(inputed_password[i]);
				}
			}
			break;
		case ON:
			signal &= 0xFE;
			if(~PIND&0x04)
			{
				steps+=2;
				steps_temp = steps;
				steps_1_place = steps%10;
				steps_temp /= 10;
				steps_10_place = steps_temp%10;
				steps_temp /= 10;
				steps_100_place = steps_temp%10;
				steps_1_place += '0';
				steps_10_place += '0';
				steps_100_place += '0';
				LCD_DisplayString(1,"PEDOMETER: ");
				LCD_WriteData(steps_100_place);
				LCD_WriteData(steps_10_place);
				LCD_WriteData(steps_1_place);
			}
			if(~PINB&0x04)
			{
				toggle_magnet = ~toggle_magnet;
			}
			if(~PINB&0x20)
			{
				toggle_right = ~toggle_right;
			}
			if(~PINB&0x10)
			{
				toggle_left = ~toggle_left;
			}
			if(toggle_right == 0xFF)
			{
				PORTD |= 0x10;
			}
			else if(toggle_right == 0x00)
			{
				PORTD &= 0xEF;
			}
			if(toggle_left)
			{
				PORTD |= 0x20;
			}
			else
			{
				PORTD &= 0xDF;
			}
			if(toggle_magnet)
			{
				signal |= 0x02;
			}
			else
			{
				signal &= 0xFD;
			}
			if(~PINB&0x40)
			{
				PORTD ^= 0x30;
			}
			break;
	}
	//Transitions
	switch(master_state){
		case INIT:
			master_state = OFF;
			break;
		case OFF:
			if(password_correct_status == 0xFF && password_count == 4)
			{
				signal |= 0x01;
				master_state = ON;
				LCD_DisplayString(1,"PEDOMETER: ");
				LCD_WriteData(steps);
			}
			break;
		case ON:
			if(~PINB&0x01 && ~PINB&0x02)
			{
				PORTD = 0x00;
				LCD_DisplayString(1,"Enter Password: ");
				master_state = OFF;
				attempts = 0;
				password_count = 0;
				signal = 0xFF;
				password_correct_status = 0xFF;
				toggle_magnet = 0x00;
				toggle_left = 0x00;
				toggle_right = 0x00;
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
		if(USART_IsSendReady(0))
		USART_Send(signal,0);
		vTaskDelay(30);
	}
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(MasterSecTask, (signed portCHAR *)"MasterSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xF0; PORTD = 0x0F;
	initUSART(0);
	LCD_init();
	LCD_DisplayString(1,"Enter Password: ");
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}
