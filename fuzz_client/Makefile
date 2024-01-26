ifdef coverage
	CVG = -O0 -fprofile-arcs -ftest-coverage
else
	CVG = 
endif

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
	
ifdef parsejson
	TEST=-DPARSEJSON
else 
	TEST =
endif

ifdef updatecell
	TEST=-DUPDATECELL 
else
	TEST=
endif

ifdef updatemodel
	TEST=-DUPDATEMODEL
else
	TEST=
endif

ifdef move
	TEST=-DMOVE
else
	TEST=
endif

ifdef checkvalidation
	TEST=-DCHECK_VALIDATION 
else
	TEST=
endif

ifdef handlecmd
	TEST=-DHANDLE_CMD
else
	TEST=
endif

CC = gcc

LIBS = `pkg-config --libs --cflags gtk+-2.0`

cJSON_path = ../cJSON.c

client_path = ./test_client.c
%:%.c $(cJSON_path) $(client_path)
	$(CC) $(CVG) $^ $(THD) $(TT) $(LIBS) -o $@ $(TEST)

# client:test_client.c $(cJSON_path)
# 	$(CC) $(CVG) $^ -lpthread $(LIBS) -o client

clean:
	rm main
	rm test_client *.gcno *.gcda *.c.gcov
	rm main *.gcno *.gcda *.c.gcov
	