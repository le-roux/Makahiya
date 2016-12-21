#include "criterion/criterion.h"
#include "utils.h"
#include "wifi.h"
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

Test(suite_2, test_parse_response_code) {
    wifi_response_header out;
    strcpy(response_code, "R000003\r\n");
    out = parse_response_code();
    cr_expect(out.error == 0);
    cr_expect(out.error_code == 0);
    cr_expect(out.length == 3);

    strcpy(response_code, "R100018\r\n");
    out = parse_response_code();
    cr_expect(out.error == 1);
    cr_expect(out.error_code == 1);
    cr_expect(out.length == 18);

    strcpy(response_code, "R30072\r\n");
    out = parse_response_code();
    cr_expect(out.error == 1);
    cr_expect(out.error_code == 3);
    cr_expect(out.length == 72);
}
