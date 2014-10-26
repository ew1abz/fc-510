;----------------------------------------------------------------------------

;Title	: FC-510 prescaler (LMX2324)
;Version: 1.00
;Target	: ATtiny12
;Author	: wubblick@yahoo.com

;AVR Assembler 2.0

;Fuse bits:

;Int RCosc, Startup 4.2ms + 6 CK
;No BOD function

;----------------------------------------------------------------------------

.include "tn12def.inc"

;----------------------------------------------------------------------------

;Константы:

.equ N = 256   ;желаемый коэффициент деления

.equ PRE = 32  ;коэффициент деления встроенного прескалера
.equ NR = 2    ;коэффициент деления Ref (не используется)
.equ NB = (N / PRE)
.equ NA = (N - NB * PRE)
.if (NA > NB) || (NB < 3) || (NB > 1023)
.error "Недопустимый коэффициент деления!"
.endif

;Биты регистра R:

.equ TEST    = (1 << 14) ;режим тестирования
.equ RS      = (1 << 13) ;зарезервированный бит
.equ PD_POL  = (1 << 12) ;вывод счетчика N
.equ CP_TRI  = (1 << 11) ;в режиме тестирования всегда =1
.equ R_CNTR  = (NR << 1) ;значение счетчика R (2..1023)
.equ R_ADDR  = (1 << 0)  ;адрес регистра R

;Биты регистра N:

.equ NB_CNTR = (NB << 8) ;значение счетчика NA (0..31, NA <= NB)
.equ NA_CNTR = (NA << 3) ;значение счетчика NB (3..1023)
.equ CNT_RST = (1 << 2)  ;режим сброса счетчиков
.equ PWDN    = (1 << 1)  ;режим power down
.equ N_ADDR  = (0 << 0)  ;адрес регистра N

;Код для загрузки в регистр R:

.equ REG_R = TEST | PD_POL | CP_TRI | R_CNTR | R_ADDR

;Код для загрузки в регистр N:

.equ REG_N = NB_CNTR | NA_CNTR | N_ADDR

;----------------------------------------------------------------------------

;Ports definition:

.equ SDATA  = PB0 ;сигнал SDATA
.equ LE     = PB1 ;сигнал LE
.equ SCLK   = PB2 ;сигнал SCLK
.equ NCPB3  = PB3
.equ NCPB4  = PB4

;Направление порта B:
.equ DIRB   = (1 << SDATA) | (1 << LE) | (1 << SCLK)
;Начальное состояние/пуллапы:
.equ PUPB   = 0xFF

;----------------------------------------------------------------------------

;Глобальные регистровые переменные:

.def tempL = r16
.def tempM = r17
.def tempH = r18
.def Cnt   = r19

;----------------------------------------------------------------------------

.CSEG
.org 0

;Инициализация:

	ldi	tempL,PUPB
	out	PORTB,tempL
	ldi	tempL,DIRB	
	out	DDRB,tempL

;Основная программа:

Main:

;Загрузка регистра N:

	ldi tempL,byte1(REG_N)
	ldi tempM,byte2(REG_N)
	ldi tempH,byte3(REG_N)
	rcall SPI_Load

;Загрузка регистра R:

	ldi tempL,byte1(REG_R)
	ldi tempM,byte2(REG_R)
	ldi tempH,byte3(REG_R)
	rcall SPI_Load

	sleep
	rjmp Main

;----------------------------------------------------------------------------

;Загрузка слова 18 бит из tempH:tempM:tempL по SPI:

SPI_Load:
	cbi	PORTB,LE    ;LE = 0
	ldi Cnt,18
Loop:
	cbi	PORTB,SCLK  ;SCLK = 0
	sbrc tempH,1
	rjmp data1
data0:
	cbi	PORTB,SDATA ;SDATA = 0 или
	rjmp dataX
data1:
	sbi	PORTB,SDATA ;SDATA = 1
dataX:
    lsl TempL 
	rol TempM
	rol TempH
	sbi	PORTB,SCLK  ;SCLK = 1
	dec	Cnt
	brne Loop

	sbi	PORTB,SDATA ;SDATA = 1
	sbi	PORTB,LE    ;LE = 1
	ret

;----------------------------------------------------------------------------
