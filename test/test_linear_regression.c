#include "test.h"

Test(suite_2, test_linear_regression_1) {
    init(SENSOR_1);
    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, i);
    reg_t ret = linear_regression(SENSOR_1);
    cr_expect(ret.slope == 1);
    cr_expect(ret.var_y == 4, "var_y: %lu\n", ret.var_y);
    cr_expect(ret.corr == 1, "corr: %f\n", ret.corr);
}

Test(suite_2, test_linear_regression_2) {
    init(SENSOR_1);
    for (int i = 0; i < 3 * REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 2 * i);
    reg_t ret = linear_regression(SENSOR_1);
    cr_expect(ret.slope == 2, "coeff found : %i", ret.slope);
}

Test(suite_2, test_linear_regression_3) {
    init(SENSOR_1);
    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 2 * i);
    reg_t ret = linear_regression(SENSOR_1);
    cr_expect(ret.slope == 2);

    for (int i = 0; i < REGRESSION_SIZE; i++)
        add_value(SENSOR_1, 3 * i);
    ret = linear_regression(SENSOR_1);
    cr_expect(ret.slope == 3, "coeff found: %i", ret.slope);

    for (int i = 0; i < 3 * REGRESSION_SIZE; i++)
        add_value(SENSOR_1, i);
    ret = linear_regression(SENSOR_1);
    cr_expect(ret.slope == 1);
}
