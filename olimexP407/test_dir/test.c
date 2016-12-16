#include "criterion/criterion.h"
#include "utils.h"
#include <string.h>

Test(suite_1, test_int_to_char) {
    char tmp[6];
    int_to_char(tmp, 2);
    cr_expect(strcmp(tmp, "2") == 0, "tmp = %s", tmp);

    int_to_char(tmp, 12);
    cr_expect(strcmp(tmp, "12") == 0, "tmp = %s", tmp);

    int_to_char(tmp, 854);
    cr_expect(strcmp(tmp, "854") == 0, "tmp = %s", tmp);

    int_to_char(tmp, 98653);
    cr_expect(strcmp(tmp, "98653") == 0, "tmp = %s", tmp);
}
