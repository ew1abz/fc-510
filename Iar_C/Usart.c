//----------------------------------------------------------------------------

//USART support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Keyboard.h"
#include "Usart.h"

//----------------------------- Constants: -----------------------------------

#define BAUD 19200         //UART baud rate
#define TX_CR_OFFSET 0 //LCD_SIZE     //CR symbol index
#define TX_LF_OFFSET 1 //LCD_SIZE + 1 //LF symbol index

#define UBRRV (int)((F_CLK * 1E6)/(16.0 * BAUD) - 0.5)

//------------------------------ Variables: ----------------------------------

static char TxPtr;         //TX buffer pointer
static char *buffer;
static char bufSize;

//------------------------- Function prototypes: -----------------------------

#pragma vector = USART_RXC_vect
__interrupt void Rx_Int(void); //RX complete interrupt

//----------------------------- Display init: --------------------------------

void USART_Init(void)
{
  UBRRL = LO(UBRRV);   //set up baud rate
  UBRRH = HI(UBRRV);
  UCSRB = (1 << RXCIE) | (1 << RXEN) | (1 << TXEN); //RX, TX enable
  TxPtr = bufSize + TX_LF_OFFSET + 1;   //do not TX
}

//----------------------- TX display copy via UART: --------------------------

void USART_Exe(bool t)
{
  if(t)
  {
    if(!TxPtr) UDR = buffer[TxPtr++];
    else if(TxPtr <= bufSize + TX_LF_OFFSET)
    {
      if(UCSRA & (1 << TXC))
      {
        UCSRA = 1 << TXC; //reset TXC flag
        if(TxPtr == bufSize + TX_CR_OFFSET) { UDR = '\r'; TxPtr++; }
        else if(TxPtr == bufSize + TX_LF_OFFSET) { UDR = '\n'; TxPtr++; }
        else UDR = buffer[TxPtr++];
      }
    }
  }
}

//----------------------- TX LCD content via UART: ---------------------------

void USART_Update(char * buf, char size)
{
  buffer = buf;
  TxPtr = 0; //request to TX
  bufSize = size;
}

//-------------------------- RX byte via UART: -------------------------------

#pragma vector = USART_RXC_vect
__interrupt void Rx_Int(void)
{
  char data = UDR;
  char code = KEY_NO;
  switch(data)
  {
  case 'M': code = KEY_MN; break; //"MENU" code
  case 'D': code = KEY_DN; break; //"DOWN" code
  case 'U': code = KEY_UP; break; //"UP" code
  case 'A': code = KEY_UD; break; //"DOWN" + "UP" ("Auto Scale") code
  case 'K': code = KEY_OK; break; //"OK" code
  }
  Keyboard_SetCode(code);
}

