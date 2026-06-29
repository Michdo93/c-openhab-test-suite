#ifndef OPENHAB_RULE_TESTER_H
#define OPENHAB_RULE_TESTER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <openhab/openhab.h>

/**
 * Returns a heap-allocated JSON object string with rule status fields.
 * Caller must free(). Returns NULL on error.
 */
char* openhab_rule_tester_get_rule_status    (openhab_client_t* c, const char* rule_uid);

int   openhab_rule_tester_is_rule_active     (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_is_rule_disabled   (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_is_rule_running    (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_is_rule_idle       (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_enable_rule        (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_disable_rule       (openhab_client_t* c, const char* rule_uid);
int   openhab_rule_tester_run_rule           (openhab_client_t* c, const char* rule_uid, const char* context_json);
int   openhab_rule_tester_test_rule_execution(openhab_client_t* c, const char* rule_uid,
                                              const char* expected_item,
                                              const char* expected_value);

#ifdef __cplusplus
}
#endif
#endif /* OPENHAB_RULE_TESTER_H */
