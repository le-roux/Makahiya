SRC_DIR := ../code/src
INC_DIR := ../code/includes
CFLAGS := -Wall -Wextra -Werror -std=gnu99 -iquote $(INC_DIR)/
LDFLAGS := -lcriterion

test: test.c $(SRC_DIR)/capacitive_sensor.c

clean:
	rm -f *.o
	rm -f test
