#include "criterion/criterion.h"
#include "capacitive_sensor.h"
#include <stdio.h>

Test(suite_1, test_default_value) {
    init();
    for (int i = 0; i < BUFFER_SIZE; i++)
        add_value(1);
    update_default_value();
    cr_expect(default_value == 1, "Test 1");
}

Test(suite_1, test_no_touch_detection) {
    FILE* file = fopen("./data2.csv", "r");
    uint32_t data;
    init();
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(data);
    }
    update_default_value();
    while (fscanf(file, "%i\n", &data) == 1)
        cr_expect(add_value(data) == 0, "no touch");
    fclose(file);
}

/**
 * Test that the 3 touches are properly detected in data3.csv.
 */
Test(suite_1, test_touch_detection) {
    FILE* file = fopen("./data3.csv", "r");
    int ret = 0;
    uint32_t data;
    init();
    for (int i = 0; i < BUFFER_SIZE; i++) {
        fscanf(file, "%i\n", &data);
        add_value(data);
    }
    update_default_value();
    //
    for (int i = BUFFER_SIZE; i < 31; i++) {
        fscanf(file, "%i\n", &data);
        cr_expect(add_value(data) == 0, "No touch 1");
    }
    // Detect 1st touch
    for (int i = 31; i < 37; i++) {
        fscanf(file, "%i\n", &data);
        ret += add_value(data);
    }
    cr_expect(ret == 1, "Touch 1");
    //
    for (int i = 37; i < 50; i++) {
        fscanf(file, "%i\n", &data);
        cr_expect(add_value(data) == 0, "No touch 2");
    }
    // Detect 2nd touch
    for (int i = 50; i < 56; i++) {
        fscanf(file, "%i\n", &data);
        ret += add_value(data);
    }
    cr_expect(ret == 2, "Touch 2");
    //
    for (int i = 56; i < 68; i++) {
        fscanf(file, "%i\n", &data);
        cr_expect(add_value(data) == 0, "No touch 3");
    }
    // Detect 3rd touch
    for (int i = 68; i < 75; i++) {
        fscanf(file, "%i\n", &data);
        ret += add_value(data);
    }
    //
    cr_expect(ret == 3, "Touch 3");
    for (int i = 75; i < 116; i++) {
        fscanf(file, "%i\n", &data);
        cr_expect(add_value(data) == 0, "No touch 4");
    }
    fclose(file);
}
