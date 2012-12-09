#ifndef MAIN_H
#define MAIN_H

/* Define some lights for trying things out. */
#ifdef __18F4550
	#define LED0 (PORTDbits.RD0)
	#define LED1 (PORTDbits.RD1)
	#define LED2 (PORTDbits.RD2)
	#define LED3 (PORTDbits.RD3)
	#define LED4 (PORTDbits.RD4)
	#define LED5 (PORTDbits.RD5)
	#define LED6 (PORTDbits.RD6)
	#define LED7 (PORTDbits.RD7)
#endif /* __18F4550. */


signed InitializeLEDs(void);


void Task_Test0(void);
void Task_Test1(void);
void Task_Test2(void);
void Task_Test3(void);


signed InitializeNESController(void);
unsigned char ReadNESController(void);
signed NESControllerManager(void);


/* Define the NES controller pins. */
#define NES_IsParallelLoad  PORTBbits.RB0
#define NES_Clock           PORTBbits.RB1
#define NES_DataStream      PORTBbits.RB2

#endif /* MAIN_H. */
