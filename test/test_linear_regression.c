#include "test.h"

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
