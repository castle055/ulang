BUILD_DIR := build

#CC = gcc
#OPT = -O3 -std=c11 -lfl
CC = g++
OPT = -O3 -std=c++2b -DUPARSER_GEN_EXEC
uparser: $(BUILD_DIR)/lexer.cpp $(BUILD_DIR)/parser.cpp parser_ctx.cpp
	$(CC) $(OPT) $^ -o $@

$(shell mkdir -pv $(BUILD_DIR))

$(BUILD_DIR)/lexer.hpp $(BUILD_DIR)/lexer.cpp: lexer.l
	flex --header-file=$(BUILD_DIR)/lexer.hpp -o $(BUILD_DIR)/lexer.cpp lexer.l

$(BUILD_DIR)/parser.hpp $(BUILD_DIR)/parser.cpp: parser.y
	bison -L C++ -d -v -o $(BUILD_DIR)/parser.cpp parser.y  # -Wcounterexamples

clean:
	rm -rvf $(BUILD_DIR)/*
	rmdir $(BUILD_DIR)

.PHONY: clean