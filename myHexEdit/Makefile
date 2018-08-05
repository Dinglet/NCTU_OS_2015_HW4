
CC = gcc

CFLAGS = -lm -I../include/ -DPARTITION_START=2048*512 -Wall

# gdb 
CFLAGS += -g

OUTPUT = myHexEdit
OBJ	=	common \
		myHexEdit

$(OUTPUT): $(addsuffix .o, $(OBJ))
	mkdir -p ../out/
	$(CC) -o ../out/$@ $(addprefix ../obj/, $^) $(CFLAGS)

%.o: ../%.c
	mkdir -p ../obj/
	$(CC) -c -o ../obj/$@ $< $(CFLAGS)
	
clean:
	rm -r ../obj/
	rm -r ../out/

cleanobj:
	rm -r ../obj/
