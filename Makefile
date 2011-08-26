# Simple Makefile for Arduino directory

include ../../arduino.mk

LIB := libOpenLCB.a

include ../../standard.mk

INCLUDE_OPTIONS := -I${PWD} -I${PWD}/../CAN

lib:
	${ARDUINO_ROOT}hardware/tools/avr/bin/avr-g++ -c -g  ${CC_OPTIONS_ARDUINO} ${INCLUDE_ARDUINO} ${INCLUDE_OPTIONS} *.cpp 
