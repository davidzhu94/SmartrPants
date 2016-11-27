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
enum BeltState {INIT, Release, Connect} belt_state;

unsigned char temp;

void Belt_Init(){
	belt_state = INIT;
}

void Belt_Tick(){
	//Actions
	switch(belt_state){
		case INIT:
			break;
		case Release:
			//release magnet & lengthen belt
			break;
		case Connect:
			//charge magnets and wait 3 seconds, then tighten belt until load sensor reaches threshold 
			break;
	}
	//Transitions
	switch(belt_state){
		case INIT:
			belt_state = Release;
			break;
		case Release:
			if(USART_HasReceived(0))
			{
				temp = USART_HasReceived(0);
				if(temp == 0)
				belt_state = Connect;
				USART_Flush(0);
			}
			break;
		case Connect:
			if(USART_HasReceived(0))
			{
				temp = USART_HasReceived(0);
				if(temp == 1)
				belt_state = Release;
				USART_Flush(0);
			}
			break;
		
	}
}

void BeltSecTask()
{
	Belt_Init();
   for(;;) 
   { 	
	Belt_Tick();
	vTaskDelay(100); 
   } 
}

void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(BeltSecTask, (signed portCHAR *)"BeltSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}	
 
int main(void) 
{ 
   DDRB = 0xFF; PORTB=0x00;
   DDRD = 0x00; PORTD = 0xFF;
   initUSART(0);
   //Start Tasks  
   StartSecPulse(1);
    //RunSchedular 
   vTaskStartScheduler(); 
 
   return 0; 
}