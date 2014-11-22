//----------------------------------------------------------------------------

//Frequency Counter FC-510
//main module header file

//----------------------------------------------------------------------------

#ifndef MainH
#define MainH

//----------------------------------------------------------------------------

#include <iom8.h>
#include <intrinsics.h>
#include <stdbool.h>
#include <stdlib.h>

//------------------------------- Constants: ---------------------------------

//#define       F_CLK    5.000 //clock frequency, MHz
//#define	F_CLK   10.000 //clock frequency, MHz
//#define	F_CLK   12.000 //clock frequency, MHz
//#define	F_CLK   12.288 //clock frequency, MHz
#define	F_CLK   12.800 //clock frequency, MHz
//#define	F_CLK   13.000 //clock frequency, MHz
//#define	F_CLK   14.400 //clock frequency, MHz
//#define       F_CLK   15.000 //clock frequency, MHz
//#define       F_CLK   16.000 //clock frequency, MHz
//#define	F_CLK   16.368 //clock frequency, MHz
//#define	F_CLK   16.369 //clock frequency, MHz
//#define	F_CLK   16.384 //clock frequency, MHz
//#define	F_CLK   17.500 //clock frequency, MHz

#define T_SYS    500.0 //system tick, uS

//----------------------------------------------------------------------------
//-------------------------------- Ports: ------------------------------------
//----------------------------------------------------------------------------

//Legend:

//I - input
//O - outbut
//B - bidirectional

//L - low active level
//H - high active level
//X - undefined active level
//A - analog signal

//------------------------------- Port B: ------------------------------------

#define SCLOCK (1 << PB0) //OX - CPLD serial clock
#define SDATA  (1 << PB1) //IX - CPLD serial data
#define RETL   (1 << PB2) //IL - keyboard return line
#define LOAD   (1 << PB3) //OL - LCD load strobe
#define DATA   (1 << PB4) //OX - LCD serial data
#define SCLK   (1 << PB5) //OX - LCD serial clock
#define NC_PB6 (1 << PB6) //IL - not used
#define NC_PB7 (1 << PB7) //IL - not used

//Direction:
#define I_DDRB  (SCLOCK | LOAD | DATA | SCLK)
//Pull-ups (in) or initial state (out):
#define I_PORTB (SDATA | RETL | LOAD | DATA | SCLK | NC_PB6 | NC_PB7)
//Port control macros:
#define Port_SCLOCK_0 (PORTB &= ~SCLOCK)
#define Port_SCLOCK_1 (PORTB |= SCLOCK)
#define Pin_SDATA     (PINB & SDATA)
#define Pin_RETL      (PINB & RETL)
#define Port_LOAD_0   (PORTB &= ~LOAD)
#define Port_LOAD_1   (PORTB |= LOAD)
#define Port_DATA_0   (PORTB &= ~DATA)
#define Port_DATA_1   (PORTB |= DATA)
#define Port_SCLK_0   (PORTB &= ~SCLK)
#define Port_SCLK_1   (PORTB |= SCLK)

//------------------------------- Port C: ------------------------------------

#define FDIV   (1 << PC0) //IH - frequency divider sense
#define LED    (1 << PC1) //OH - gate LED
#define SND    (1 << PC2) //OL - sound generation
#define FSYNC  (1 << PC3) //OL - CPLD frame synchronization
#define RESET  (1 << PC4) //OL - CPLD reset
#define CALIB  (1 << PC5) //OH - CPLD calibrate
#define NC_PC6 (1 << PC6) //IL - not used

//Direction:
#define I_DDRC  (LED | SND | FSYNC | RESET | CALIB)
//Pull-ups (in) or initial state (out):
#define I_PORTC (FDIV | SND | FSYNC | RESET | NC_PC6)
//Port control macros:
#define Pin_FDIV      (PINC & FDIV)
#define Port_LED_0    (PORTC &= ~LED)
#define Port_LED_1    (PORTC |= LED)
#define Port_SND_0    (PORTC &= ~SND)
#define Port_SND_1    (PORTC |= SND)
#define Port_SND      (PORTC & SND)
#define Port_FSYNC_0  (PORTC &= ~FSYNC)
#define Port_FSYNC_1  (PORTC |= FSYNC)
#define Port_RESET_0  (PORTC &= ~RESET)
#define Port_RESET_1  (PORTC |= RESET)
#define Port_CALIB_0  (PORTC &= ~CALIB)
#define Port_CALIB_1  (PORTC |= CALIB)

//------------------------------- Port D: ------------------------------------

#define RXD    (1 << PD0) //IL - serial port RXD
#define TXD    (1 << PD1) //OL - serial port TXD
#define GATE   (1 << PD2) //OH - CPLD gate
#define MODE2  (1 << PD3) //IL - CPLD mode 2
#define QFREF  (1 << PD4) //IX - TO counter input
#define QFIN   (1 << PD5) //IX - T1 counter input
#define MODE0  (1 << PD6) //IL - CPLD mode 0
#define MODE1  (1 << PD7) //IL - CPLD mode 1

//Direction:
#define I_DDRD  (TXD | GATE | MODE2 | MODE0 | MODE1)
//Pull-ups (in) or initial state (out):
#define I_PORTD (RXD | TXD | QFREF | QFIN)
//Port control macros:
#define Port_GATE_0   (PORTD &= ~GATE)
#define Port_GATE_1   (PORTD |= GATE)
#define Port_MODE2_0  (PORTD &= ~MODE2)
#define Port_MODE2_1  (PORTD |= MODE2)
#define Port_MODE0_0  (PORTD &= ~MODE0)
#define Port_MODE0_1  (PORTD |= MODE0)
#define Port_MODE1_0  (PORTD &= ~MODE1)
#define Port_MODE1_1  (PORTD |= MODE1)

//------------------------------- Macros: ------------------------------------

#define HI(x) ((x >> 8) & 0xFF)
#define LO(x) (x & 0xFF)
#define Delay_us(x) __delay_cycles((int)(x * F_CLK + 0.5))
#define ms2sys(x) ((int)(1E3 * x / T_SYS))
#define ABS(x) ((x < 0)? (-x) : (x))

//----------------------------------------------------------------------------

#endif
