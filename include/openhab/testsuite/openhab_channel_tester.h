#ifndef OPENHAB_CHANNEL_TESTER_H
#define OPENHAB_CHANNEL_TESTER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <openhab/openhab.h>

int   openhab_channel_tester_is_item_linked_to_channel    (openhab_client_t* c, const char* item_name, const char* channel_uid);
/**
 * Returns heap-allocated JSON array string of links for the item.
 * Caller must free(). Returns NULL on error.
 */
char* openhab_channel_tester_get_links_for_item            (openhab_client_t* c, const char* item_name);
int   openhab_channel_tester_is_item_linked_to_any_channel (openhab_client_t* c, const char* item_name);
int   openhab_channel_tester_has_orphaned_links            (openhab_client_t* c);

#ifdef __cplusplus
}
#endif
#endif /* OPENHAB_CHANNEL_TESTER_H */
