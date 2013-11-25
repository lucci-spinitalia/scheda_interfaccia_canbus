# MPLAB IDE generated this makefile for use with GNU make.
# Project: test_velocità_comunicazione.mcp
# Date: Fri Nov 09 13:39:00 2012

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

test_velocità_comunicazione.cof : interrupt.o main.o uart_interface.o
	$(LD) /p18F8722 /l"C:\Program Files (x86)\Microchip\mplabc18\v3.43\lib" "system\linker\18f8722_g.lkr" "interrupt.o" "main.o" "uart_interface.o" /u_CRUNTIME /u_DEBUG /z__MPLAB_BUILD=1 /z__MPLAB_DEBUG=1 /o"test_velocità_comunicazione.cof" /M"test_velocità_comunicazione.map" /W

interrupt.o : system/interrupt/interrupt.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h system/interrupt/interrupt.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f8722.h user/global.h system/interrupt/interrupt.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h
	$(CC) -p=18F8722 "system\interrupt\interrupt.c" -fo="interrupt.o" -D__DEBUG -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

main.o : main.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h main.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f8722.h main.h system/io_cfg.h user/global.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h
	$(CC) -p=18F8722 "main.c" -fo="main.o" -D__DEBUG -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

uart_interface.o : user/uart_interface.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h user/uart_interface.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f8722.h system/io_cfg.h
	$(CC) -p=18F8722 "user\uart_interface.c" -fo="uart_interface.o" -D__DEBUG -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

clean : 
	$(RM) "interrupt.o" "main.o" "uart_interface.o" "test_velocità_comunicazione.cof" "test_velocità_comunicazione.hex"

