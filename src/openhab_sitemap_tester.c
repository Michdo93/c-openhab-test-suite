#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_sitemap_tester.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int contains_sitemap_name(const char* json, const char* name) {
    if (!json||!name) return 0;
    char needle[256]; snprintf(needle,sizeof(needle),"\"name\":\"%s\"",name);
    return strstr(json,needle)!=NULL;
}

static int contains_item_name(const char* json, const char* item_name) {
    if (!json||!item_name) return 0;
    /* Search for item blocks: "item":{"name":"<item_name>" */
    char needle[256];
    snprintf(needle, sizeof(needle), "\"name\":\"%s\"", item_name);
    /* We rely on the fact that item name appears under an "item" key */
    const char* p = json;
    while ((p = strstr(p, "\"item\"")) != NULL) {
        const char* brace = strchr(p, '{');
        if (!brace) break;
        /* Check within 200 chars */
        char buf[256]; size_t n = strnlen(brace, 255);
        strncpy(buf, brace, n); buf[n]='\0';
        if (strstr(buf, needle)) return 1;
        p += 6;
    }
    return 0;
}

int openhab_sitemap_tester_does_sitemap_exist(openhab_client_t* c, const char* name) {
    char* raw = openhab_sitemaps_getSitemaps(c);
    if (!raw) return 0;
    int ok = contains_sitemap_name(raw, name);
    free(raw); return ok;
}

int openhab_sitemap_tester_does_sitemap_contain_item(openhab_client_t* c,
                                                      const char* sitemap_name,
                                                      const char* item_name) {
    char* raw = openhab_sitemaps_getSitemap(c, sitemap_name, NULL, 0, NULL);
    if (!raw) return 0;
    int ok = contains_item_name(raw, item_name);
    free(raw); return ok;
}
