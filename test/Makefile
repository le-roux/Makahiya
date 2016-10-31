SRC_DIR := ../code/src
INC_DIR := ../code/includes
OUTPUT_DIR := ../code/out
CFLAGS := -Wall -Wextra -Werror -std=gnu99 -iquote $(INC_DIR)/
LDFLAGS := -lcriterion -lpthread
OBJ := test.o $(OUTPUT_DIR)/capacitive_sensor.o

test: OUT $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

OUT:
	-mkdir $(OUTPUT_DIR)

clean:
	rm -f *.o
	rm -f $(OUTPUT_DIR)/*
	-rmdir $(OUTPUT_DIR)
	rm -f test
