SRC_DIR := ./src
INC_DIR := ./includes
OUTPUT_DIR := ./out

CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu99 -iquote $(INC_DIR)

all: OUT main

main: $(OUTPUT_DIR)/main.o $(OUTPUT_DIR)/capacitive_sensor.o
	$(CC) $(CFLAGS) $^ -o $@

OUT:
	-mkdir $(OUTPUT_DIR)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f out/*.o
	rm -f out/main
	-rmdir out
