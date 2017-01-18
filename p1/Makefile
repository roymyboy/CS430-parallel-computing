
CC=mpicc
CFLAGS= -Wall -O
#CFLAGS= -g -Wall 
LDFLAGS = -lprand
OBJS = monte_carlo_pi.o timing.o

pmcpi: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)


clean:
	/bin/rm -f *.o *.log pmcpi

