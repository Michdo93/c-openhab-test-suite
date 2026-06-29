#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_channel_tester.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int json_array_has_items(const char* j) {
    if (!j) return 0;
    const char* p=strchr(j,'['); if(!p) return 0; p++;
    while(*p==' '||*p=='\n'||*p=='\r'||*p=='\t') p++;
    return *p!=']';
}

static char* json_str_field(const char* json, const char* key) {
    if (!json||!key) return NULL;
    char search[128]; snprintf(search,sizeof(search),"\"%s\"",key);
    const char* p=strstr(json,search); if(!p) return NULL;
    p+=strlen(search);
    while(*p==' '||*p==':') p++;
    if(*p!='"') return NULL; p++;
    const char* end=p;
    while(*end&&*end!='"'){if(*end=='\\')end++;end++;}
    size_t len=(size_t)(end-p);
    char* r=malloc(len+1); if(!r) return NULL;
    memcpy(r,p,len); r[len]='\0'; return r;
}

int openhab_channel_tester_is_item_linked_to_channel(openhab_client_t* c,
                                                      const char* item_name,
                                                      const char* channel_uid) {
    char* raw = openhab_links_getLink(c, item_name, channel_uid);
    if (!raw) return 0;
    char* n = json_str_field(raw, "itemName");
    int ok = n && strcmp(n, item_name)==0;
    free(n); free(raw); return ok;
}

char* openhab_channel_tester_get_links_for_item(openhab_client_t* c, const char* item_name) {
    char* r = openhab_links_getLinks(c, item_name, NULL);
    return r ? r : strdup("[]");
}

int openhab_channel_tester_is_item_linked_to_any_channel(openhab_client_t* c, const char* item_name) {
    char* r = openhab_channel_tester_get_links_for_item(c, item_name);
    int ok = json_array_has_items(r);
    free(r); return ok;
}

int openhab_channel_tester_has_orphaned_links(openhab_client_t* c) {
    char* r = openhab_links_getOrphanLinks(c);
    if (!r) return 0;
    int ok = json_array_has_items(r);
    free(r); return ok;
}
