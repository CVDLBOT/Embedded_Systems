// TuningFork.c Lab 12
// Runs on LM4F120/TM4C123
// Use SysTick interrupts to create a squarewave at 440Hz.  
// There is a positive logic switch connected to PA3, PB3, or PE3.
// There is an output on PA2, PB2, or PE2. The output is 
//   connected to headphones through a 1k resistor.
// The volume-limiting resistor can be any value from 680 to 2000 ohms
// The tone is initially off, when the switch goes from
// not touched to touched, the tone toggles on/off.
//                   |---------|               |---------|     
// Switch   ---------|         |---------------|         |------
//
//                    |-| |-| |-| |-| |-| |-| |-|
// Tone     ----------| |-| |-| |-| |-| |-| |-| |---------------
//
// Daniel Valvano, Jonathan Valvano
// December 29, 2014

/* This example accompanies the book
   "Embedded Systems: Introduction to ARM Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2014
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include "TExaS.h"
#include "..//tm4c123gh6pm.h"

// Global defitions:

#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI0_R             (*((volatile unsigned long *)0xE000E400))  // IRQ 28 to 31 Priority Register
#define GPIO_PORTA_DATA_R       (*((volatile unsigned long *)0x400043FC))
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define NVIC_SYS_PRI3_R         (*((volatile unsigned long *)0xE000ED20))  // Sys. Handlers 12 to 15 Priority
#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))


// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void WaitForInterrupt(void);  // low power mode

// **************Sound_Init*********************
// Initialize SysTick periodic interrupts
// Input: none
// Output: none
void Sound_Init(void){ 

  NVIC_ST_CTRL_R = 0;         			// disable SysTick during setup
  NVIC_ST_RELOAD_R = 90909-1; 			// reload value
  NVIC_ST_CURRENT_R = 0;      			// any write to current clears it
  NVIC_SYS_PRI3_R =  NVIC_SYS_PRI3_R&0x00FFFFFF; //priority 0                          
  NVIC_ST_CTRL_R = 0x07;      			// enable SysTick with core clock and interrupts
	EnableInterrupts();               // enable after all initialization are done	
}

// **************SysTick_Handler*********************
// called at 880 Hz
// Input: none
// Output: none
//Global Variables:
unsigned long Counts;
unsigned long Counts1;
unsigned long count3 = 0;

void SysTick_Handler(void){
	unsigned long led;	
  Counts1 = GPIO_PORTA_DATA_R&0x08; // Read input from port A.
	
	if(Counts1 == 0x08 && count3 == 0){
			led = GPIO_PORTA_DATA_R;       // toggle PF2;
			led = led^0x04;
			GPIO_PORTA_DATA_R = led;
			Counts = 1;
	}else if(Counts1 == 0x00 && Counts == 1){
		led = GPIO_PORTA_DATA_R;       // toggle PF2;
		led = led^0x04;
		GPIO_PORTA_DATA_R = led;
		count3 = 1;
	}else if(Counts1 == 0x08 && count3 == 1){
		GPIO_PORTA_DATA_R = 0x00;       // toggle PF2;
		Counts = 0;
	}else if(Counts1 == 0x08 && Counts == 0){
		led = GPIO_PORTA_DATA_R;       // toggle PF2;
		led = led^0x04;
		GPIO_PORTA_DATA_R = led;
		count3 = 0;
	}
}

// **************Main*********************
// input from PA3, output from PA2, SysTick interrupts
int main(void){
	  volatile unsigned long delay;
	// activate grader and set system clock to 80 MHz
  TExaS_Init(SW_PIN_PA3, HEADPHONE_PIN_PA2,ScopeOn);
	SYSCTL_RCGC2_R |= 0x01; 					// activate port A clock.
	delay = SYSCTL_RCGC2_R; 					// Allow clock to stabilize.
  Counts = 0;             					// Intialize count to zero.
  GPIO_PORTA_DIR_R |= 0x04;         // make PA2 output
	GPIO_PORTA_DIR_R &= ~0x08;        //make PA3 input
  GPIO_PORTA_AFSEL_R &= ~0x0C;      // disable alt funct on PA2, PA3
  GPIO_PORTA_DEN_R |= 0x0C;         // enable digital I/O on PA2, PA3,configure as GPIO
  GPIO_PORTA_PCTL_R &= ~0x0000FF00; //regular function PA2, PA3
  GPIO_PORTA_AMSEL_R &= ~0x0C;      // disable analog functionality on PF	
  Sound_Init();      
	EnableInterrupts();
  while(1){
    // main program is free to perform other tasks
    // do not use WaitForInterrupt() here, it may cause the TExaS to crash
  }
}
