/**
 * Integration test for c-openhab-test-suite.
 * Build and run:
 *   cmake -B build && cmake --build build
 *   OPENHAB_URL=http://127.0.0.1:8080 ./build/openhab_suite_test
 */
#define OPENHAB_STATIC
#include <openhab/openhab_testsuite.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed=0, failed=0;
static void check(const char* name, int ok) {
    printf("  %s %s\n", ok ? "✓" : "✗", name);
    ok ? passed++ : failed++;
}

int main(void) {
    const char* url  = getenv("OPENHAB_URL")  ? getenv("OPENHAB_URL")  : "http://127.0.0.1:8080";
    const char* user = getenv("OPENHAB_USER") ? getenv("OPENHAB_USER") : "openhab";
    const char* pass = getenv("OPENHAB_PASS") ? getenv("OPENHAB_PASS") : "habopen";

    printf("\n── Static validators ──\n");
    check("isValidSwitchValue ON",  openhab_item_tester_is_valid_switch_value("ON"));
    check("isValidSwitchValue bad", !openhab_item_tester_is_valid_switch_value("TOGGLE"));
    check("isValidDimmerValue 50",  openhab_item_tester_is_valid_dimmer_value("50"));
    check("isValidDimmerValue 101", !openhab_item_tester_is_valid_dimmer_value("101"));
    check("isValidColorValue HSB",  openhab_item_tester_is_valid_color_value("240,100,100"));
    check("isValidColorValue bad",  !openhab_item_tester_is_valid_color_value("red"));
    check("isValidNumberValue 42",  openhab_item_tester_is_valid_number_value("42"));
    check("isValidLocation",        openhab_item_tester_is_valid_location_value("48.7758,9.1829"));
    check("isValidDateTime",        openhab_item_tester_is_valid_datetime_value("2024-01-15T08:30:00+0000"));

    printf("\n── Connecting to %s ──\n", url);
    openhab_client_t* c = openhab_client_create(url, user, pass, NULL);
    if (!c || !openhab_client_is_logged_in(c)) {
        printf("  ✗ login\nConnection failed. Skipping integration tests.\n");
        openhab_client_destroy(c);
        return 1;
    }
    printf("  ✓ login (isCloud=%d)\n", openhab_client_is_cloud(c));

    printf("\n── ItemTester ──\n");
    check("doesItemExist testSwitch",   openhab_item_tester_does_item_exist(c,"testSwitch"));
    check("doesItemExist fake",         !openhab_item_tester_does_item_exist(c,"itemDoesNotExist99"));
    check("checkItemIsType Switch",     openhab_item_tester_check_item_is_type(c,"testSwitch","Switch"));
    check("checkItemIsType wrong",      !openhab_item_tester_check_item_is_type(c,"testSwitch","Dimmer"));
    check("testSwitch ON",  openhab_item_tester_test_switch(c,"testSwitch","ON","ON",10));
    check("testSwitch OFF", openhab_item_tester_test_switch(c,"testSwitch","OFF","OFF",10));

    printf("\n── ThingTester ──\n");
    char* status = openhab_thing_tester_get_thing_status(c,"astro:sun:local");
    printf("  ℹ astro:sun:local status = %s\n", status ? status : "NULL");
    check("getThingStatus not null", status != NULL);
    free(status);

    printf("\n── RuleTester ──\n");
    const char* ruid = "test_color-1";
    char* rs = openhab_rule_tester_get_rule_status(c, ruid);
    check("getRuleStatus not null", rs != NULL); free(rs);
    check("enableRule",   openhab_rule_tester_enable_rule(c, ruid));
    check("runRule",      openhab_rule_tester_run_rule(c, ruid, NULL));
    check("disableRule",  openhab_rule_tester_disable_rule(c, ruid));
    check("isRuleDisabled", openhab_rule_tester_is_rule_disabled(c, ruid));
    openhab_rule_tester_enable_rule(c, ruid);

    printf("\n── ChannelTester ──\n");
    check("hasOrphanedLinks runs",
          openhab_channel_tester_has_orphaned_links(c)==0 ||
          openhab_channel_tester_has_orphaned_links(c)==1);

    printf("\n── SitemapTester ──\n");
    check("doesSitemapExist default", openhab_sitemap_tester_does_sitemap_exist(c,"default"));
    check("doesSitemapExist fake",    !openhab_sitemap_tester_does_sitemap_exist(c,"fakeNonExistent99"));

    openhab_client_destroy(c);
    printf("\n%s\nResults: %d passed, %d failed\n","────────────────────────────────────────",passed,failed);
    return failed > 0 ? 1 : 0;
}
