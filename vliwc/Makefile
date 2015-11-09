CC=gcc
LD=gcc
#CFLAGS=-Wall

OBJ = loader.o basic_block.o decode.o deps.o \
      utils.o graph.o main.o

all: vliwc

vliwc : $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	-rm *.o

