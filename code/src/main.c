#include "capacitive_sensor.h"
#include "utils.h"
#include "stdio.h"

uint32_t data, id;
int ret;

int main(int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    id = 0;
    init();
    printf("Start\n");
    for (int i = 0; i < 10; i++) {
        ret = scanf("%i\n", &data);
        add_value(data);
        id++;
    }
    update_default_value();
    printf("Default value updated: average %i, min = %i, max = %i\n", default_value, default_value - MARGIN, default_value + MARGIN);
    while(ret == 1) {
        id++;
        ret = scanf("%i\n", &data);
        if (add_value(data))
            printf("Touch detected (index: %i, average %i)\n", id, average);
    }
    printf("End (index: %i)\n", id);
    return 0;
}
