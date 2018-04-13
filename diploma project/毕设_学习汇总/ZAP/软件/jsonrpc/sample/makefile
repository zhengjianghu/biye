INCLUDE_FLAG:=-I../../../../inc
INCLUDE_FLAG+=-I../../../../inc/WINDOWS
CC_FLAG:=
CC:=gcc

all : main.o
	$(CC) $^ -o sample.exe -ljsonrpcwinx86gnud -lringbuf16winx86gnud -lcjsonwinx86gnud -lloggerwinx86gnud -lws2_32 -L../../../../lib/X86_WINDOWS 

%.o: %.c
	$(CC) $(INCLUDE_FLAG) $(CC_FLAG) -c -MMD -MP -MF"$(@:%.o=%.d)" -o"$@" "$<" 
	
.PHONY: clean
clean :
	-rm -rf *.exe *.o *.d
