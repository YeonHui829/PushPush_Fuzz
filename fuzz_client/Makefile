# ifdef coverage
# 	CVG = -O0 -static -fprofile-arcs -ftest-coverage
# else
# 	CVG = 
# endif

ifdef thread
	THD = -lpthread
else
	THD = 
endif

ifdef test
	TT = -DMAIN
else
	TT = 
endif

CC = gcc

LIBS = `pkg-config --libs --cflags gtk+-2.0`

cJSON_path = ../cJSON.c

%:%.c $(cJSON_path)
	$(CC) -O0 -static -fprofile-arcs -ftest-coverage -o test $^ $(THD) $(TT) $(LIBS)

clean:
	rm test *.gcno *.gcda *.gcov