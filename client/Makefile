DIR	:= ./debug
SRC := ./src
INC := ./include
BIN := ./bin
ELF	:= $(BIN)/client
CC		:= gcc 
LIBS	:= crypt pthread
SRCS	:= $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/*.c) $(wildcard $(SRC)/*.cc)
OCPP	:= $(patsubst $(SRC)/%.cpp, $(DIR)/%.o, $(wildcard $(SRC)/*.cpp))
OC		:= $(patsubst $(SRC)/%.c, $(DIR)/%.co, $(wildcard $(SRC)/*.c))
OCC		:= $(patsubst $(SRC)/%.cc, $(DIR)/%.cco, $(wildcard $(SRC)/*.cc))
OBJS	:= $(OC) $(OCC) $(OCPP)
RM		:= rm -rf
CXXFLAGS	:= -Wall -std=gnu99 -I$(INC)
start	: mkdebug $(ELF) 
mkdebug :
	@if [ ! -d $(DIR) ]; then mkdir $(DIR); fi;
$(ELF) 	: $(OBJS)
	$(CC) -o $@ $(OBJS) $(addprefix -l,$(LIBS))
$(DIR)/%.o : $(SRC)/%.cpp
	$(CC) -c $(CXXFLAGS) $< -o $@
$(DIR)/%.co : $(SRC)/%.c
	$(CC) -c $(CXXFLAGS) $< -o $@
$(DIR)/%.cco : $(SRC)/%.cc
	$(CC) -c $(CXXFLAGS) $< -o $@
.PHONY 	: clean rebuild
clean	:
	@$(RM) $(BIN)/* $(DIR)/*.o $(DIR)/*.co $(DIR)/*.cco
rebuild: clean start
