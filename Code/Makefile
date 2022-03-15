#---------------------------------------------------
# Target file to be compiled by default
#---------------------------------------------------

MAIN = Main

#----------------------------------------------------
# CC will be the compiler to use
#---------------------------------------------------

CC = gcc

#---------------------------------------------------
# CFLAGS will be the options passed to the compiler
#---------------------------------------------------

CFLAGS = -Wall -lpthread -lrt -lwiringPi -lm


#---------------------------------------------------
# Dependencies
#---------------------------------------------------

$(MAIN): $(MAIN).o ptask.o auxlib.o hwlib.o
	$(CC) -o $(MAIN) $(MAIN).o ptask.o auxlib.o hwlib.o `allegro-config --libs` $(CFLAGS)

$(MAIN).o: $(MAIN).c
	$(CC) -c $(MAIN).c

ptask.o: ptask.c
	$(CC) -c ptask.c
	
mylib.o: auxlib.c
	$(CC) -c auxlib.c

hwlib.o: hwlib.c
	$(CC) -c hwlib.c
