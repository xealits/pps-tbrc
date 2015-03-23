BIN = ppsRun

INCLUDE_DIR = include
SRC_DIR = src
OBJ_DIR = obj

CC = g++
CFLAGS = -O3 -Wall
LD = g++
LDFLAGS = -g

vpath %.h $(INCLUDE_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.o $(OBJ_DIR)

all: $(BIN)

$(BIN): $(OBJ_DIR)/main.o
	@echo "Linking everything together..."
	@$(LD) $(LDFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp %.cpp
	@echo "Compiling" $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@echo "Compiling" $<
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*
	rm -f $(BIN)
