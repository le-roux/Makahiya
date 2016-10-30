SRC_DIR := ./src
INC_DIR := ./includes
OUTPUT_DIR := ./out
HEADERS := $(INC_DIR)/capacitive_sensor.h $(INC_DIR)/utils.h

CC := gcc
CFLAGS := -Wall -Wextra -Werror -iquote $(INC_DIR)

all: OUT main

main: $(OUTPUT_DIR)/main.o $(OUTPUT_DIR)/capacitive_sensor.o
	$(CC) $(CFLAGS) $^ -o $(OUTPUT_DIR)/$@

OUT:
	-mkdir $(OUTPUT_DIR)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f out/*.o
	rm -f out/main
	-rmdir out
