#ifndef OPENHAB_SITEMAP_TESTER_H
#define OPENHAB_SITEMAP_TESTER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <openhab/openhab.h>

int openhab_sitemap_tester_does_sitemap_exist        (openhab_client_t* c, const char* sitemap_name);
int openhab_sitemap_tester_does_sitemap_contain_item (openhab_client_t* c, const char* sitemap_name, const char* item_name);

#ifdef __cplusplus
}
#endif
#endif /* OPENHAB_SITEMAP_TESTER_H */
