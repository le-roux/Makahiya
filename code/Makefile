SRC_DIR := ./src
INC_DIR := ./includes
OUTPUT_DIR := ./out
HEADERS := $(INC_DIR)/capacitive_sensor.h $(INC_DIR)/fdc_utils.h

CC := $(CCACHE) gcc
CFLAGS := -Wall -Wextra -Werror -std=gnu99 -iquote $(INC_DIR)

all: OUT $(OUTPUT_DIR)/capacitive_sensor.o

OUT:
	-mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f out/*.o
	rm -f out/main
	-rmdir out

clean_doc:
	rm -Rf doc/html
	rm -Rf doc/latex
