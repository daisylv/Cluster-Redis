BIN=./cluster/clustermodule

ifeq ($(CFLAG), DEBUG)
CFLAGS += -g
endif

AR  = ar -cqs
# itermidiate objects
OBJ = $(addprefix $(BIN)/, \
	  cluster.o \
	  )
# include file path
INC = -I. -I.

$(BIN)/%.o:%.c ../libconhash/conhash.h
	gcc $(INC) $(CFLAGS) -c $< -o $@
all: