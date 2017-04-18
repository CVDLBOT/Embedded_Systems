// ***** 0. Documentation Section *****
// TableTrafficLight.c for Lab 10
// Runs on LM4F120/TM4C123
// Index implementation of a Moore finite state machine to operate a traffic light.  
// Tony Joseph
// December 29, 2014

// east/west red light connected to PB5
// east/west yellow light connected to PB4
// east/west green light connected to PB3
// north/south facing red light connected to PB2
// north/south facing yellow light connected to PB1
// north/south facing green light connected to PB0
// pedestrian detector connected to PE2 (1=pedestrian present)
// north/south car detector connected to PE1 (1=car present)
// east/west car detector connected to PE0 (1=car present)
// "walk" light connected to PF3 (built-in green LED)
// "don't walk" light connected to PF1 (built-in red LED)

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"
#include "SysTick.h"

// ***** 2. Global Declarations Section *****
#define SYSCTL_RCGC2_R1          (*((volatile unsigned long *)0x400FE108))

//// Port A:
//#define GPIO_PORTA_DATA_R       (*((volatile unsigned long *)0x400043FC)) // bits 7-0
//#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
//#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
//#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
//#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
//#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
	
// Port F:
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))


// Port B
#define GPIO_PORTB_DATA_R       (*((volatile unsigned long *)0x400053FC)) // bits 7-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_DR8R_R       (*((volatile unsigned long *)0x40005508))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
	
// Port E
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC)) // bits 7-0
#define GPIO_PORTE_PUR_R        (*((volatile unsigned long *)0x40024510)) // bits 7-0
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortF_init(void);
void PortB_init(void);
void PortE_init(void);
void PortA_init(void);

// ***** 3. Subroutines Section *****
struct State {
	unsigned long Out;     // Traffic light Led Outputs
	unsigned long Out1;    // Pedestrain led ouput.
	unsigned long Time;    // Wait Time
	unsigned long Next[8]; // Next States.
};

typedef const struct State StateType;

#define goW    0  // Green west, red south : 0x02, 0x0C
#define waitW  1  // Yellow west, red south: 0x02, 0x14
#define goS    2  // Green south, red west : 0x02, 0x21
#define waitS  3  // Yellow West, red South: 0x02, 0x22
#define goPed  4  // Green Pedestrain, red South & West : 0x08, 0x24
#define StopPed  5 //  Red Pedestrain, red South & West : 0x02, 0x24
#define offPed   6 //  off Pedestrain, red South & West : 0x00, 0x24
#define FlashPed 7 //  Red Pedestrain, red South & West : 0x02, 0x24
#define finPed   8 //  off Pedestrain, red South & West : 0x00, 0x24

StateType Fsm[9] = {
 {0x0C, 0x02, 100, {goW, goW, waitW, waitW, waitW, waitW, waitW, waitW}}, 
 {0x14, 0x02, 50,  {waitW, waitW, goS, goS, goPed, goW, goS, goS}},
 {0x21, 0x02, 100, {goS, waitS, goS, waitS, waitS, waitS, waitS, waitS}},
 {0x22, 0x02, 50,  {waitS, goW, waitS, goW, goPed, goW, goS, goPed}},
 {0x24, 0x08, 100, {goPed, StopPed, StopPed,StopPed, goPed, StopPed, StopPed,StopPed}},
 {0x24, 0x02, 5,   {StopPed, offPed, offPed, offPed, offPed, offPed, offPed, offPed}},
 {0x24, 0x00, 5,   {offPed, FlashPed, FlashPed, FlashPed, FlashPed, FlashPed, FlashPed, FlashPed}},
 {0x24, 0x02, 5,   {FlashPed, finPed, finPed, finPed, finPed, finPed, finPed, finPed}},
 {0x24, 0x00, 5,   {finPed, goW, goS, goW, FlashPed, goW, goS, goW}}
};

unsigned long S; // Current State
unsigned long input; // Car senor and Pedestrian sensor
unsigned long out, out1; // Pedestrain 

int main(void){ 
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210); // activate grader and set system clock to 80 MHz
  EnableInterrupts();
	SysTick_Init(); 
	PortF_init();
	PortB_init();
	PortE_init();
	
	S = goS; // Intial State.
  while(1){
		//Set Lights:
		out  = Fsm[S].Out;
		out1 = Fsm[S].Out1;
		GPIO_PORTB_DATA_R = out;
		GPIO_PORTF_DATA_R = out1;
		SysTick_Wait10ms(Fsm[S].Time); // Delay.
		
		// Get inputs:
		input = GPIO_PORTE_DATA_R;
		S = Fsm[S].Next[input];
	}
}

void PortF_init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x00000020;      // 1) F clock
  delay = SYSCTL_RCGC2_R;            // delay to allow clock to stabilize 
	GPIO_PORTF_LOCK_R = 0x4C4F434B;    // Unlock GPIO port F
  GPIO_PORTF_CR_R |= 0x1F;	
  GPIO_PORTF_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL  
  GPIO_PORTF_DIR_R |= 0x0A;          // 4.2) PF3, PF1 output  
  GPIO_PORTF_AFSEL_R &= 0x00;        // 5) no alternate function
  GPIO_PORTF_PUR_R |= 0x00;          // 6) enable pullup resistor on PF4       
  GPIO_PORTF_DEN_R |= 0x0A;          // 7) enable digital pins PF4-PF1
}

void PortB_init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x02;          // 1) activate Port B
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PB7-0
  GPIO_PORTB_AMSEL_R &= ~0xFF;     // 3) disable analog functionality on PB7-0
  GPIO_PORTB_PCTL_R = 0x00000000;  // 4) configure PB7-0 as GPIO
  GPIO_PORTB_DIR_R |= 0x3F;        // 5) make PB5-0 out
  GPIO_PORTB_AFSEL_R &= ~0x3F;     // 6) disable alt funct on PB5-0
  GPIO_PORTB_DEN_R |= 0x3F;        // 7) enable digital I/O on PB5-0
}

//void PortA_init(void){
//	volatile unsigned long delay;
//	SYSCTL_RCGC2_R |= 0x01;          // 1) activate Port E
//  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
//                                   // 2) no need to unlock PE1-0
//  GPIO_PORTA_AMSEL_R &= ~0x1C;     // 3) disable analog function on PE1-0
//  GPIO_PORTA_PCTL_R &= ~0x000FFF00;// 4) configure PE1-0 as GPIO
//  GPIO_PORTA_DIR_R &= ~0x1C;       // 5) make PE1-0 input
//  GPIO_PORTA_AFSEL_R &= ~0x1C;     // 6) disable alt funct on PE1-0
//  GPIO_PORTA_DEN_R |= 0x1C;        // 7) enable digital I/O on PE1-0
//}

void PortE_init(void){
	volatile unsigned long delay;
	SYSCTL_RCGC2_R |= 0x10;          // 1) activate Port E
  delay = SYSCTL_RCGC2_R;          // allow time for clock to stabilize
                                   // 2) no need to unlock PE1-0
	GPIO_PORTE_PUR_R   &= ~0x07;     //    Disable Pull Up resistor.
  GPIO_PORTE_AMSEL_R &= ~0x07;     // 3) disable analog function on PE1-0
  GPIO_PORTE_PCTL_R &= ~0x00000FFF;// 4) configure PE1-0 as GPIO
  GPIO_PORTE_DIR_R &= ~0x07;       // 5) make PE1-0 input
  GPIO_PORTE_AFSEL_R &= ~0x07;     // 6) disable alt funct on PE1-0
  GPIO_PORTE_DEN_R |= 0x07;        // 7) enable digital I/O on PE1-0
}
