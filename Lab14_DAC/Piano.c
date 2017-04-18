// Piano.c
// Runs on LM4F120 or TM4C123, 
// edX lab 13 
// There are four keys in the piano
// Daniel Valvano
// December 29, 2014

// Port E bits 3-0 have 4 piano keys

#include "Piano.h"
#include "..//tm4c123gh6pm.h"

// **************Piano_Init*********************
// Initialize piano key inputs
// Input: none
// Output: none
void Piano_Init(void){ 
  volatile unsigned long  delay;
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOE;          // 1) activate Port E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PE1-0
	GPIO_PORTE_PUR_R   &= ~0x0F;     //    Disable Pull Up resistor.
  GPIO_PORTE_AMSEL_R &= ~0x0F;     // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x0000FFFF;// 4) configure PE3-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x0F;       // 5) make PE3-0 input
  GPIO_PORTE_AFSEL_R &= ~0x0F;     // 6) disable alt funct on PE3-0
  GPIO_PORTE_DEN_R |= 0x0F;        // 7) enable digital I/O on PE3-0
}

// **************Piano_In*********************
// Input from piano key inputs
// Input: none 
// Output: 0 to 15 depending on keys
// 0x01 is key 0 pressed, 0x02 is key 1 pressed,
// 0x04 is key 2 pressed, 0x08 is key 3 pressed
// Piano key 3: G (783.991 Hz)
// Piano key 2: E (659.255 Hz)
// Piano key 1: D (587.330 Hz)
// Piano key 0: C (523.251 Hz)

unsigned long Piano_In(void){
	return (GPIO_PORTE_DATA_R&0x0F);
}
	
