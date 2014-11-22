//----------------------------------------------------------------------------

//Keyboard support module: header file

//----------------------------------------------------------------------------

#ifndef KeyboardH
#define KeyboardH

//----------------------------- Constants: -----------------------------------

#define KEY_NO  0x00  //no key pressed
#define KEY_MN  0x01  //key "MENU"
#define KEY_DN  0x02  //key "DOWN"
#define KEY_UP  0x03  //key "UP"
#define KEY_UD  0x04  //key "UP" + "DOWN"
#define KEY_OK  0x05  //key "OK"

#define REP_S   0x40  //slow autorepeat
#define REP_F   0x80  //fast autorepeat

//-------------------------- Прототипы функций: ------------------------------

void Keyboard_Init(void);      //keyboard module init
void Keyboard_Exe(bool t);     //scan keyboard
void Keyboard_SetCode(char c); //set key code
char Keyboard_GetCode(void);   //get key code

//----------------------------------------------------------------------------

#endif

