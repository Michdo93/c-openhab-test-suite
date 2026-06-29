#ifndef OPENHAB_THING_TESTER_H
#define OPENHAB_THING_TESTER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <openhab/openhab.h>

/**
 * Returns a heap-allocated string with the Thing's status
 * (e.g. "ONLINE", "OFFLINE"). Caller must free(). Returns NULL on error.
 */
char* openhab_thing_tester_get_thing_status     (openhab_client_t* c, const char* thing_uid);

int   openhab_thing_tester_is_thing_status      (openhab_client_t* c, const char* thing_uid, const char* status);
int   openhab_thing_tester_is_thing_online      (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_is_thing_offline     (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_is_thing_pending     (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_is_thing_unknown     (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_is_thing_uninitialized(openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_is_thing_error       (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_enable_thing         (openhab_client_t* c, const char* thing_uid);
int   openhab_thing_tester_disable_thing        (openhab_client_t* c, const char* thing_uid);

#ifdef __cplusplus
}
#endif
#endif /* OPENHAB_THING_TESTER_H */
