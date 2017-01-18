NCC = /usr/local/cuda/bin/nvcc
CC = ${NCC} 
EXEC =cudaMandy 
FLAGS = --compiler-bindir /home/faculty/cathie/gcc-4.7/bin -O3 -I/usr/local/cuda/include

all: $(EXEC)

cudaMandy: cudaMandy.cu 
	$(CC) $(FLAGS) -o $@ cudaMandy.cu $(OBJS)



clean: 
	rm -f *.o $(OBJS) $(EXEC) *.log *.pgm 

