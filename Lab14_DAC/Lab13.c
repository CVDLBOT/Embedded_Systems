// Tony Joseph
// Port B bits 3-0 have the 4-bit DAC
// Port E bits 3-0 have 4 piano keys

#include "..//tm4c123gh6pm.h"
#include "Sound.h"
#include "Piano.h"
#include "TExaS.h"

// basic functions defined at end of startup.s
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void delay(unsigned long msec);


int main(void){ 
	unsigned long input, previous;
	// Real Lab13 
	// for the real board grader to work 
	// you must connect PD3 to your DAC output
  TExaS_Init(SW_PIN_PE3210, DAC_PIN_PB3210,ScopeOn); // activate grader and set system clock to 80 MHz
// PortE used for piano keys, PortB used for DAC  
  Piano_Init(); // intialize inputs.	
  Sound_Init(); // initialize SysTick timer and DAC
  EnableInterrupts();  // enable after all initialization are done
	// 0x01 is key 0 pressed, 0x02 is key 1 pressed,
  // 0x04 is key 2 pressed, 0x08 is key 3 pressed
	previous = Piano_In()&0x0F;// Dyanmic Sensing
  while(1){			
		// Piano key 3: G (783.991 Hz)
		// Piano key 2: E (659.255 Hz)
		// Piano key 1: D (587.330 Hz)
		// Piano key 0: C (523.251 Hz)
					
		input = Piano_In()&0x0F;// input from keys to select tone
		
		if(input&&(previous==0x00)){ // key 1just pressed     
      EnableInterrupts();
      Sound_Tone(9556); // (523.25 Hz)
    }
    if(previous&&(input==0x00)){ // key 1 just released     
      DisableInterrupts();    // stop sound
    }
		//**********************************************
		
		if(input&&(previous==0x01)){ // just pressed     
      EnableInterrupts();
      Sound_Tone(9556); // (523.25 Hz)
    }
    if(previous&&(input==0)){ // just released     
      DisableInterrupts();    // stop sound
    }		
	  //**********************************************

		if(input&&(previous==0x02)){ // just pressed     
      EnableInterrupts();
      Sound_Tone(8513); // (659.255 Hz)
    }
    if(previous&&(input==0x02)){ // just released     
      DisableInterrupts();    // stop sound
    }
		//**********************************************

		if(input&&(previous==0x03)){ // just pressed     
      EnableInterrupts();
      Sound_Tone(7584); // (783.991 Hz)
    }
    if(previous&&(input==0x03)){ // just released     
      DisableInterrupts();    // stop sound
    }
		//**********************************************

		previous = input; 
    delay(10);  // remove switch bounce
  }         
}

// Inputs: Number of msec to delay
// Outputs: None
void delay(unsigned long msec){ 
  unsigned long count;
  while(msec > 0 ) {  // repeat while there are still delay
    count = 16000;    // about 1ms
    while (count > 0) { 
      count--;
    } // This while loop takes approximately 3 cycles
    msec--;
  }
}
