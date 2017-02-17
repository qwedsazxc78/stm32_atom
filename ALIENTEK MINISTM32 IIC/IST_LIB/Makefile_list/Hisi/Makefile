CC = arm-hisiv400-linux-gcc
AR = arm-hisiv400-linux-ar
INC = -I ../inc
OBJ = ist_calibration.o IST_LIB.o ist_math.o
LIB_NAME = IST_LIB.a

$(LIB_NAME): ${OBJ}
	${AR} -r $@ ${OBJ}
%.o: %.c
	${CC} $< ${INC} -c 

.PHONY: clean
clean:
	rm -f $(LIB_NAME) ${OBJ}