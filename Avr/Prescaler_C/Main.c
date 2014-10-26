//----------------------------------------------------------------------------

//Project:         Prescaler (LMX2324)
//Version:         V1.0
//Compiler:        IAR EWAVR 5.30
//Microcontroller: ATtiny25
//E-mail:          wubblick@yahoo.com

//----------------------------------------------------------------------------

//Fuses: 0xD920
//SPI Enable          (SPIEN = 0)
//External clock      (SKSEL3 = 0, SKSEL2 = 0, SKSEL1 = 0, SKSEL0 = 0)  
//Startup 6CK + 65 ms (SUT0 = 0)
//BOD enabled, 4.0V   (BODLEVEL = 0, BODEN = 0)

//----------------------------------------------------------------------------

#include <iotiny25.h>
#include <intrinsics.h>

//----------------------------------------------------------------------------

#define N      256  //желаемый коэффициент деления

//----------------------------------------------------------------------------

#define PRE     32  //коэффициент деления встроенного прескалера
#define NB  (N / PRE)
#define NA  (N - NB * PRE)
#if (NA > NB) || (NB < 3) || (NB > 1023)
#error"Недопустимый коэффициент деления!"
#endif

#define BITS    18  //разрядность регистров

//биты регистра R:

#define TEST     1  //режим тестирования
#define RS       0  //зарезервированный бит
#define PD_POL   1  //вывод счетчика N
#define CP_TRI   1  //в режиме тестирования всегда =1
#define R_CNTR   2  //значение счетчика R (2..1023)
#define R_ADDR   1  //адрес регистра R

//биты регистра N:

#define NB_CNTR  NA //значение счетчика NA (0..31, NA <= NB)
#define NA_CNTR  NB //значение счетчика NB (3..1023)
#define CNT_RST  0  //режим сброса счетчиков
#define PWDN     0  //режим power down
#define N_ADDR   0  //адрес регистра N

//------------------------------- Порты : ------------------------------------

#define SDATA  (1 << PB0)
#define LE     (1 << PB1)
#define SCLK   (1 << PB2)
#define NC     (1 << PB3)
#define LED    (1 << PB4)

//Направление:
#define I_DDRB  (SDATA | LE | SCLK | LED)
//Начальное стостояние/pull-ups:
#define I_PORTB (SDATA | LE | SCLK | NC | LED)
//Макросы:
#define Port_SDATA_0 (PORTB &= ~SDATA)
#define Port_SDATA_1 (PORTB |= SDATA)
#define Port_LE_0    (PORTB &= ~LE)
#define Port_LE_1    (PORTB |= LE)
#define Port_SCLK_0  (PORTB &= ~SCLK)
#define Port_SCLK_1  (PORTB |= SCLK)
#define Port_LED_0   (PORTB &= ~SCLOCK)
#define Port_LED_1   (PORTB |= SCLOCK)

//-------------------------- Прототипы функций: ------------------------------

void main(void);
void SPI_Load(long n);

//------------------------- Основная программа: ------------------------------

void main(void)
{
  DDRB  = I_DDRB;
  PORTB = I_PORTB;
  //загрузка регистра N:
  SPI_Load((NB_CNTR <<  8) |
           (NA_CNTR <<  3) |
           (CNT_RST <<  2) |
              (PWDN <<  1) |
            (N_ADDR <<  0));
  //загрузка регистра R:
  SPI_Load(   (TEST << 14) |
                (RS << 13) |
            (PD_POL << 12) |
            (CP_TRI << 11) |
            (R_CNTR <<  1) |
            (R_ADDR <<  0));
  __sleep();
}

//------------------ Функция загрузка BITS битов по SPI: ---------------------

void SPI_Load(long n)
{
  Port_LE_0;
  for(char i = 0; i < BITS; i++)
  {
    Port_SCLK_0;
    if(n & (1L << (BITS - 1)))
      Port_SDATA_1;
        else Port_SDATA_0;
    n = n << 1;    
    Port_SCLK_1;
  }
  Port_SDATA_1;
  Port_LE_1;
}

//----------------------------------------------------------------------------
