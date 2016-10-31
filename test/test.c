#include "criterion/criterion.h"
#include "capacitive_sensor.h"
#include <stdio.h>

#define SENSOR_1 0

Test(suite_1, test_default_value) {
    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++)
        add_value(SENSOR_1, 1);
    update_default_value(SENSOR_1);
    cr_expect(default_value[SENSOR_1] == BUFFER_SIZE, "Test 1");
}

Test(suite_1, test_no_touch_detection) {
    FILE* file = fopen("./data2.csv", "r");
    uint32_t data;
    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    while (fscanf(file, "%i\n", &data) == 1) {
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "no touch");
    }
    fclose(file);
}

/**
 * Test that the 3 touches are properly detected in data3.csv.
 */
Test(suite_1, test_touch_detection) {
    FILE* file = fopen("./data3.csv", "r");
    int ret = 0;
    uint32_t data;
    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    //
    for (int i = BUFFER_SIZE; i < 31; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "No touch i: %i, ret: %i", i, ret);
    }
    ret = 0;
    // Detect 1st touch
    for (int i = 31; i < 37; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 1, "Touch 1");
    //
    for (int i = 37; i < 50; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "No touch 2");
    }
    // Detect 2nd touch
    for (int i = 50; i < 56; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 2, "ret: %i", ret);
    //
    for (int i = 56; i < 68; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "No touch 3");
    }
    // Detect 3rd touch
    for (int i = 68; i < 75; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    //
    cr_expect(ret == 3, "Touch 3");
    for (int i = 75; i < 116; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0, "No touch 4");
    }
    fclose(file);
}

/**
 * Test that 1 touch is detected in data_configuration_1/single_touch.csv.
 */
Test(suite_1, test_one_touch) {
    FILE* file = fopen("data_configuration_1/single_touch.csv" ,"r");
    uint32_t data;
    int ret = 0;
    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    for (int i = BUFFER_SIZE; i < 70; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "no touch i:%i, ret: %i", i, ret);
    }
    ret = 0;
    for (int i = 70; i < 75; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 1, "1 touch");
    for (int i = 75; i < 119; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "no touch i: %i, ret: %i", i, ret);
    }
    fclose(file);
}

Test(suite_2, test_linear_regression_1) {
    init(SENSOR_1);
    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, i);
    cr_expect(linear_regression(SENSOR_1) == 1);
}

Test(suite_2, test_linear_regression_2) {
    init(SENSOR_1);
    for (int i = 0; i < 3 * REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 2 * i);
    cr_expect(linear_regression(SENSOR_1) == 2, "coeff found : %i", linear_regression(SENSOR_1));
}

Test(suite_2, test_linear_regression_3) {
    init(SENSOR_1);
    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 2 * i);
    cr_expect(linear_regression(SENSOR_1) == 2);

    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 3 * i);
    cr_expect(linear_regression(SENSOR_1) == 3, "coeff found: %i", linear_regression(SENSOR_1));

    for (int i = 0; i < 3 * REGRESSION_SIZE; i++)
        add_value(SENSOR_1, i);
    cr_expect(linear_regression(SENSOR_1) == 1);
}

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
        //printf("i: %i, ret: %i, coeff: %i, dist: %i\n", i, ret, linear_regression(SENSOR_1), current_distance(SENSOR_1));
        cr_expect(ret == 0, "i: %i, ret: %i", i, ret);
    }
    fclose(file);
}
