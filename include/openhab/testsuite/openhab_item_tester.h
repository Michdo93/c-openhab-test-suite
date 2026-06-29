#ifndef OPENHAB_ITEM_TESTER_H
#define OPENHAB_ITEM_TESTER_H
/**
 * openhab_item_tester — C openHAB Test Suite
 *
 * Tests openHAB item behaviour: type checks, command/update validation,
 * SSE-based state observation, and automatic state reset after each test.
 * Mirrors the Python ItemTester from openhab-test-suite.
 *
 * Naming convention: openhab_item_tester_<method>(client, ...)
 * All test functions return 1 (true) or 0 (false).
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <openhab/openhab.h>

/* ── Static validators ───────────────────────────────────────────────────────
 * These do not need a client — they only inspect the value string.
 * Return 1 if valid, 0 if not.
 */
int openhab_item_tester_is_valid_switch_value      (const char* v);
int openhab_item_tester_is_valid_contact_value     (const char* v);
int openhab_item_tester_is_valid_dimmer_value      (const char* v);
int openhab_item_tester_is_valid_rollershutter_value(const char* v);
int openhab_item_tester_is_valid_color_value       (const char* v);
int openhab_item_tester_is_valid_player_value      (const char* v);
int openhab_item_tester_is_valid_number_value      (const char* v);
int openhab_item_tester_is_valid_datetime_value    (const char* v);
int openhab_item_tester_is_valid_location_value    (const char* v);
int openhab_item_tester_is_valid_image_value       (const char* v);

/* ── Existence / type / state ─────────────────────────────────────────────── */
int openhab_item_tester_does_item_exist    (openhab_client_t* c, const char* item_name);
int openhab_item_tester_check_item_is_type (openhab_client_t* c, const char* item_name, const char* item_type);
int openhab_item_tester_check_item_has_state(openhab_client_t* c, const char* item_name, const char* expected);
int openhab_item_tester_is_group_item      (openhab_client_t* c, const char* item_name);

/* ── Group helpers ────────────────────────────────────────────────────────── */
/**
 * Returns a heap-allocated JSON string of the group's members array.
 * Caller must free().  Returns NULL on error.
 */
char* openhab_item_tester_get_group_members(openhab_client_t* c, const char* group_name);
int   openhab_item_tester_does_group_contain_member  (openhab_client_t* c, const char* group_name, const char* member_name);
int   openhab_item_tester_check_group_member_state   (openhab_client_t* c, const char* group_name, const char* member_name, const char* expected_state);

/* ── Per-type test functions ──────────────────────────────────────────────────
 * Each sends a command/update, waits up to timeout_sec seconds for an SSE
 * ItemStateChangedEvent confirming expected_state, then resets the item.
 * Pass "" for expected_state to skip SSE verification.
 */
int openhab_item_tester_test_switch       (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_contact      (openhab_client_t* c, const char* item, const char* update,        const char* expected, int timeout_sec);
int openhab_item_tester_test_color        (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_dimmer       (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_rollershutter(openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_number       (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_player       (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_datetime     (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_location     (openhab_client_t* c, const char* item, const char* update,        const char* expected, int timeout_sec);
int openhab_item_tester_test_image        (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);
int openhab_item_tester_test_string       (openhab_client_t* c, const char* item, const char* command,       const char* expected, int timeout_sec);

#ifdef __cplusplus
}
#endif

#endif /* OPENHAB_ITEM_TESTER_H */
