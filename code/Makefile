SRC_DIR := ./src
INC_DIR := ./includes
OUTPUT_DIR := ./out

CC := gcc
CFLAGS := -Wall -Wextra -Werror -iquote $(INC_DIR)

all: main

main: OUT $(OUTPUT_DIR)/main.o

OUT:
	-mkdir $(OUTPUT_DIR)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f out/*.o
	rm -f out/main
	-rmdir out
