SRC_DIR := ../code/src
INC_DIR := ../code/includes
OUTPUT_DIR := ../code/out
CFLAGS := -Wall -Wextra -Werror -std=gnu99 -iquote $(INC_DIR)/
LDFLAGS := -lcriterion -lpthread
OBJ := test_touch.o test_slide.o test_linear_regression.o $(OUTPUT_DIR)/capacitive_sensor.o

check: test
	./test

test: $(OUTPUT_DIR) $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@

$(OUTPUT_DIR):
	-mkdir -p $(OUTPUT_DIR)

clean:
	rm -f *.o
	rm -f $(OUTPUT_DIR)/*
	-rmdir $(OUTPUT_DIR)
	rm -f test
