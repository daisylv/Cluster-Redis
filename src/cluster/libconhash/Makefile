
# output path
BIN = ./bin

# debug stuff 
ifeq ($(CFLAG), DEBUG)
CFLAGS += -g
endif

AR  = ar -cqs
# itermidiate objects
OBJ = $(addprefix $(BIN)/, \
	  md5.o \
	  util_rbtree.o  \
	  conhash_inter.o \
	  conhash_util.o  \
	  conhash.o \
	  )

# include file path
INC = -I. -I.

	
$(BIN)/%.o : %.c
	gcc $(INC) $(CFLAGS) -c $< -o $@	

 
all : 
# prepare the bin dir	
#.PHONY : prepare	
#prepare : 
#		-mkdir $(BIN)
#	  
#.PHONY : clean
#clean  :
#		-rm -rf $(BIN)