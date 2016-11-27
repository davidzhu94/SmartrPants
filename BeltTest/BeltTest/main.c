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
enum BeltState {INIT,Off, On} belt_state;

void Belt_Init(){
	belt_state = INIT;
}

void Belt_Tick(){
	//Actions
	switch(belt_state){
		case INIT:
			break;
		case Off:
			PORTB = 0x00;
			break;
		case On:
			PORTB = 0x01;
			break;
	}
	//Transitions
	switch(belt_state){
		case INIT:
			belt_state = Off;
			break;
		case Off:
			if((~PIND & 0x01) == 0x01)
				belt_state = On;
			break;
		case On:
			if((~PIND & 0x01) == 0x00)
				belt_state = Off;
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
   //Start Tasks  
   StartSecPulse(1);
    //RunSchedular 
   vTaskStartScheduler(); 
 
   return 0; 
}