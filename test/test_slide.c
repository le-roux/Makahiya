#include "test.h"

Test(suite_3, test_slide_1) {
    FILE* file = fopen("data_configuration_1/slide_from_end.csv", "r");
    init(SENSOR_1);
    uint32_t data;
    int ret = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    for (int i = BUFFER_SIZE; i < 45; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "nothing to detect");
    }

    // Detect slide
    for (int i = 45; i < 64; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 2, "ret : %i", ret);

    for (int i = 64; i < 161; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "i: %i, ret: %i", i, ret);
    }
    fclose(file);
}

#include <stdio.h>
Test(suite_3, test_slide_2) {
    FILE* file = fopen("data_configuration_3/slide_from_end.csv", "r");
    init(SENSOR_1);
    uint32_t data;
    int ret = 0;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    for (int i = BUFFER_SIZE; i < 48; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "nothing to detect");
    }

    // Detect slide
    for (int i = 48; i < 64; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 2, "ret : %i", ret);

    for (int i = 64; i < 141; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "i: %i, ret: %i", i, ret);
    }
    fclose(file);
}
