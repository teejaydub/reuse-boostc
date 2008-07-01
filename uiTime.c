// uiTime.c

#define IN_UITIME

#include <system.h>

#include "uiTime.h"


// Rolls over every "tick".
// A tick is 0.262 seconds; there are about 3.8 per second.
// Driven by Timer 0.
unsigned char tickScaler;


void ResetUITimer(void)
{
	tickScaler = 0;
	ticks = 0;
}

void InitUiTime_Timer0(void)
{
	option_reg.T0CS = 0;  // T0 transition on internal CLKOUT
	option_reg = (option_reg & 0xF0) | 0x01;  // 1:4 prescaler on Timer 0: rolls over with a period of 1.024 ms.
	intcon.T0IE = 1;
	intcon.PEIE = 1;
	tickScaler = 0;
	ticks = 0;
}

unsigned char UiTimeInterrupt(void)
{
	// Timer 0, rolling over with a 1.024 ms period.
	if (intcon.T0IF) {
		// Clear the interrupt.
		intcon.T0IF = 0;

		if (++tickScaler == 0)
			ticks++;

		return true;
	} else
		return false;
}

void InitUiTime_Timer1(void)
{
	pie1.TMR1IE = 1;
	intcon.PEIE = 1;
	t1con = 0x01;  // prescale 1:1 on 16-bit counter @ 1 MHz.
		// 1:4 would get you ~4 Hz, but we count those manually for more resolution.
	tickScaler = 0;
	ticks = 0;
}

void UiTimeInterrupt1(void)
{
	// Timer 1, rolling over with a 65.536 ms period.
	if (pir1.TMR1IF) {
		// Clear the interrupt.
		pir1.TMR1IF = 0;

		++tickScaler;
		if (tickScaler >= 4) {
			ticks++;
			tickScaler = 0;
		}
	} 
}

void InitUiTime_60Hz(void)
{
	ResetUITimer();
}

void UiTimeUpdate60(void)
{
	if (++tickScaler >= 15) {
		tickScaler = 0;
		ticks++;
	}
}
