ifdef coverage
	CVG = -O0 -fprofile-arcs -ftest-coverage
endif

ifdef thread
	THD = -lpthread
endif

ifdef main
	TT = -DMAIN
endif

ifdef disconnected
	TT = -Ddisconnected
endif

ifdef handleclnt
	TT = -Dhandleclnt
endif

ifdef loadjson
	TT = -Dloadjson
endif

ifdef readbyte
	TT = -Dreadbyte
endif

ifdef sendmsgall
	TT = -Dsendmsgall
endif

CC = gcc

LIBS = `pkg-config --libs --cflags gtk+-2.0`

%:%.c ../cJSON.c ./server.c
	$(CC) $(CVG) -o $@ $^ $(THD) $(TT) $(LIBS)

clean:
	rm test testServer result *.gcno *.gcda *.gcov