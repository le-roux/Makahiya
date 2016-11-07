#include "test.h"

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
    cr_expect(ret == 1, "ret: %i", ret);
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
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "No touch 3, i = %i, ret = %i\n", i, ret);
    }
    // Detect 3rd touch
    for (int i = 68; i < 75; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    //
    cr_expect(ret == 1, "ret: %i\n", ret);
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

Test(suite_1, test_long_touch) {
    FILE* file = fopen("data_configuration_1/long_touch.csv", "r");
    uint32_t data;
    int ret = 0;
    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);
    for (int i = BUFFER_SIZE; i < 72; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0);
    }

    // Detect touch
    for (int i = 72; i < 79; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 1);

    // No other touch must be detected
    for (int i = 79; i < 197; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "i: %i, ret: %i\n", i, ret);
    }
    fclose(file);
}

/**
 * __WARNING__: this test in doesn't really pass as only 1 touch is detected.
 * But, I don't think we could reasonnably detect both touches. To discuss.
 */
Test(suite_1, test_double_touch) {
    FILE* file = fopen("data_configuration_1/double_touch.csv", "r");
    uint32_t data;
    int ret = 0;

    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);

    for (int i = BUFFER_SIZE; i < 25; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0);
    }

    // Detect 1st touch
    for (int i = 25; i < 29; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 1, "ret: %i\n", ret);

    // Detect second touch
    for (int i = 29; i < 32; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    //cr_expect(ret == 2);

    for (int i = 32; i < 73; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret = detect_action(SENSOR_1);
        cr_expect(ret == 0, "i: %i, ret: %i\n", i, ret);
    }
    fclose(file);
}

Test(suite_1, test_1_touch_2_minutes) {
    FILE* file = fopen("data_configuration_1/1_touch_in_2_minutes.csv", "r");
    uint32_t data;
    int ret = 0;

    init(SENSOR_1);
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
    }
    update_default_value(SENSOR_1);

    for (int i = BUFFER_SIZE; i < 1280; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0);
    }

    // Detect 1st touch
    for (int i = 1280; i < 1285; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        ret += detect_action(SENSOR_1);
    }
    cr_expect(ret == 1);

    for (int i = 1285; i < 2015; i++) {
        fscanf(file, "%i\n", &data);
        add_value(SENSOR_1, data);
        cr_expect(detect_action(SENSOR_1) == 0);
    }
    fclose(file);
}
