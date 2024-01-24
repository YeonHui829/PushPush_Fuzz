ifdef coverage
	CVG = -O0 -static -fprofile-arcs -ftest-coverage
else
	CVG = 
endif

ifdef thread
	THR = -lpthread
else
	THR = 
endif

CC = gcc

%: %.c
	$(CC) $(CVG) -o test $^ $(THR)

clean:
	rm test *.gcda *.gcno *.gcov