#include "criterion/criterion.h"
#include "capacitive_sensor.h"

Test(suite_1, test_1) {
    init();
    for (int i = 0; i < BUFFER_SIZE; i++)
        add_value(1);
    update_default_value();
    cr_expect(default_value == 2, "Test default_value");
}
