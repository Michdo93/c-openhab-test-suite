#ifndef OPENHAB_PERSISTENCE_TESTER_H
#define OPENHAB_PERSISTENCE_TESTER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <openhab/openhab.h>

int openhab_persistence_tester_is_item_persisted       (openhab_client_t* c, const char* service_id, const char* item_name);
int openhab_persistence_tester_has_data_in_range       (openhab_client_t* c, const char* service_id, const char* item_name,
                                                        const char* start_time, const char* end_time);
int openhab_persistence_tester_check_last_persisted_state(openhab_client_t* c, const char* service_id,
                                                          const char* item_name, const char* expected_state);

#ifdef __cplusplus
}
#endif
#endif /* OPENHAB_PERSISTENCE_TESTER_H */
