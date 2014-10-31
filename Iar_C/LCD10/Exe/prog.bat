@echo off
set prog=e:\programs\avrdude\avrdude
set port=COM9
set uc=m8
set flash="FC-510.hex"
rem set efuse=0x01
rem set hfuse=0xdf
rem set lfuse=0xe2
rem set lock=0x3c


:begin
%prog% -p %uc% -c stk500v2 -P %port% -B 2.1  -U flash:w:%flash%:i
rem -U hfuse:w:%hfuse%:m -U lfuse:w:%lfuse%:m 
IF ERRORLEVEL 1 GOTO error
rem %prog% -p %uc% -c stk500v2 -P %port% -B 2.1 -q -q -U lock:w:%lock%:m
rem IF ERRORLEVEL 1 GOTO error

:ok
cls
color a0
echo -              OK                -
pause
exit 0

:error
color c0
echo -             ERROR              -

:onexit
pause
exit 1

