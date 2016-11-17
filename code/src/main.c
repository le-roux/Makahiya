#include "capacitive_sensor.h"
#include "utils.h"
#include "stdio.h"

uint32_t data, id;
int ret;

#define SENSOR_1 0

int main(int argc, char* argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    id = 0;
    init(SENSOR_1);
    printf("Start\n");
    for (int i = 0; i < BUFFER_SIZE; i++)
        add_value(SENSOR_1, i);
    update_default_value(SENSOR_1);
    while(ret == 1) {
        id++;
        ret = scanf("%i\n", &data);
        add_value(SENSOR_1, data);
        if (detect_action(SENSOR_1) == 1)
            printf("Touch detected (index: %i, average %u)\n", id, *average);
    }
    update_default_value(SENSOR_1);
    linear_regression(SENSOR_1);
    return 0;
}
