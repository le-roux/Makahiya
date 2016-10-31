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
    for (int i = 0; i < BUFFER_SIZE; i++) {
        ret = scanf("%i\n", &data);
        add_value(SENSOR_1, data);
        id++;
    }
    update_default_value(SENSOR_1);
    printf("Default value updated: %i, min: %i, max: %i\n", default_value[SENSOR_1],
                    default_value[SENSOR_1] - MARGIN, default_value[SENSOR_1] + MARGIN);
    while(ret == 1) {
        id++;
        ret = scanf("%i\n", &data);
        if (add_value(SENSOR_1, data))
            printf("Touch detected (index: %i, value: %i)\n", id, average[SENSOR_1]);
        #if DEBUG
        else
            printf("Index: %i, in_touch: %i, Average: %i\n", id, in_touch, average[SENSOR_1]);
        #endif // DEBUG
    }
    printf("End (index: %i)\n", id);
    return 0;
}
