#Variables of the Makefile
CC =     mpicc
CFLAGS = -g 
LDFLAGS = -lm 
OBJ_DIR = obj/
SRC_DIR = src/
BIN_DIR = bin/
INC_DIR = include/
EXE_NAME = parapoxvirus

# Object files
OBJ =   ${SRC:${SRC_DIR}%.c=${OBJ_DIR}%.o}
# Source files 	
SRC = 	${wildcard  ${SRC_DIR}*.c}
# Header files	
INC = 	${wildcard  ${INC_DIR}*.h}
# Executable file
EXE :=  ${BIN_DIR}${EXE_NAME}