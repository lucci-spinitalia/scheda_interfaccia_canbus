# MPLAB IDE generated this makefile for use with GNU make.
# Project: interfaccia_canbus.mcp
# Date: Tue Feb 12 15:01:31 2013

AS = MPASMWIN.exe
CC = mcc18.exe
LD = mplink.exe
AR = mplib.exe
RM = rm

./output/interfaccia_canbus.cof : intermediary/main.o intermediary/interrupt.o intermediary/can_to_rs232_converter.o intermediary/uart_interface.o intermediary/can_interface.o intermediary/can.o
	$(LD) /p18F46K80 /l"C:\Program Files\Microchip\mplabc18\v3.42\lib" "system\linker\18f46k80_g.lkr" "intermediary\main.o" "intermediary\interrupt.o" "intermediary\can_to_rs232_converter.o" "intermediary\uart_interface.o" "intermediary\can_interface.o" "intermediary\can.o" /u_CRUNTIME /z__MPLAB_BUILD=1 /o"./output\interfaccia_canbus.cof" /M"./output\interfaccia_canbus.map" /W

intermediary/main.o : main.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h user/can_interface.h main.c main.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h system/io_cfg.h user/can_to_rs232_converter.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "main.c" -fo="./intermediary\main.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

intermediary/interrupt.o : system/interrupt/interrupt.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h user/can_interface.h system/interrupt/interrupt.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h system/interrupt/interrupt.h system/io_cfg.h user/can_to_rs232_converter.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "system\interrupt\interrupt.c" -fo="./intermediary\interrupt.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

intermediary/can_to_rs232_converter.o : user/can_to_rs232_converter.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h user/can_interface.h user/can_to_rs232_converter.c user/can_to_rs232_converter.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h system/io_cfg.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "user\can_to_rs232_converter.c" -fo="./intermediary\can_to_rs232_converter.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

intermediary/uart_interface.o : user/uart_interface.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stdio.h user/uart_interface.c ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h system/io_cfg.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stdarg.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stddef.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "user\uart_interface.c" -fo="./intermediary\uart_interface.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

intermediary/can_interface.o : user/can_interface.c user/can_interface.h user/can.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/string.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stdio.h user/can_interface.c system/io_cfg.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stddef.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/stdarg.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "user\can_interface.c" -fo="./intermediary\can_interface.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

intermediary/can.o : user/can.c user/can.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/usart.h user/uart_interface.h user/can.c system/io_cfg.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18cxxx.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/p18f46k80.h ../../../../../Program\ Files\ (x86)/Microchip/mplabc18/v3.43/h/pconfig.h
	$(CC) -p=18F46K80 /i".\user" -I".\system" -I"C:\Program Files\Microchip\mplabc18\v3.42\h" "user\can.c" -fo="./intermediary\can.o" -Ou- -Ot- -Ob- -Op- -Or- -Od- -Opa-

clean : 
	$(RM) "intermediary\main.o" "intermediary\interrupt.o" "intermediary\can_to_rs232_converter.o" "intermediary\uart_interface.o" "intermediary\can_interface.o" "intermediary\can.o" ".\output\interfaccia_canbus.cof" ".\output\interfaccia_canbus.hex"

