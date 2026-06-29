#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_thing_tester.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* json_str_field(const char* json, const char* key) {
    if (!json || !key) return NULL;
    char search[128]; snprintf(search, sizeof(search), "\"%s\"", key);
    const char* p = strstr(json, search);
    if (!p) return NULL;
    p += strlen(search);
    while (*p==' '||*p==':') p++;
    if (*p!='"') return NULL; p++;
    const char* end = p;
    while (*end && *end!='"') { if (*end=='\\') end++; end++; }
    size_t len = (size_t)(end-p);
    char* r = malloc(len+1); if (!r) return NULL;
    memcpy(r, p, len); r[len]='\0'; return r;
}

static char* get_status_from_statusinfo(const char* thing_json) {
    if (!thing_json) return NULL;
    const char* p = strstr(thing_json, "\"statusInfo\"");
    if (!p) return NULL;
    /* Find "status" within statusInfo object */
    const char* brace = strchr(p, '{');
    if (!brace) return NULL;
    /* Extract sub-object */
    const char* end = brace+1; int depth=1;
    while (*end && depth>0) {
        if (*end=='{') depth++;
        else if (*end=='}') depth--;
        end++;
    }
    size_t len = (size_t)(end-brace);
    char* sub = malloc(len+1); if (!sub) return NULL;
    memcpy(sub, brace, len); sub[len]='\0';
    char* status = json_str_field(sub, "status");
    free(sub); return status;
}

char* openhab_thing_tester_get_thing_status(openhab_client_t* c, const char* uid) {
    char* raw = openhab_things_getThing(c, uid, NULL);
    if (!raw) {
        fprintf(stderr,"Error reading status of '%s': %s\n", uid, openhab_last_error(c));
        return strdup("UNKNOWN");
    }
    char* status = get_status_from_statusinfo(raw);
    free(raw);
    return status ? status : strdup("UNKNOWN");
}

int openhab_thing_tester_is_thing_status(openhab_client_t* c, const char* uid, const char* status) {
    char* s = openhab_thing_tester_get_thing_status(c, uid);
    int ok = s && strcmp(s, status)==0;
    free(s); return ok;
}
int openhab_thing_tester_is_thing_online      (openhab_client_t* c, const char* u) { return openhab_thing_tester_is_thing_status(c,u,"ONLINE"); }
int openhab_thing_tester_is_thing_offline     (openhab_client_t* c, const char* u) { return openhab_thing_tester_is_thing_status(c,u,"OFFLINE"); }
int openhab_thing_tester_is_thing_pending     (openhab_client_t* c, const char* u) { return openhab_thing_tester_is_thing_status(c,u,"PENDING"); }
int openhab_thing_tester_is_thing_unknown     (openhab_client_t* c, const char* u) { return openhab_thing_tester_is_thing_status(c,u,"UNKNOWN"); }
int openhab_thing_tester_is_thing_uninitialized(openhab_client_t* c,const char* u) { return openhab_thing_tester_is_thing_status(c,u,"UNINITIALIZED"); }
int openhab_thing_tester_is_thing_error       (openhab_client_t* c, const char* u) { return openhab_thing_tester_is_thing_status(c,u,"ERROR"); }

int openhab_thing_tester_enable_thing(openhab_client_t* c, const char* uid) {
    char* r = openhab_things_setThingStatus(c, uid, 1, NULL);
    if (!r) { fprintf(stderr,"Error enabling '%s': %s\n", uid, openhab_last_error(c)); return 0; }
    printf("Thing '%s' enabled successfully.\n", uid);
    free(r); return 1;
}

int openhab_thing_tester_disable_thing(openhab_client_t* c, const char* uid) {
    char* r = openhab_things_setThingStatus(c, uid, 0, NULL);
    if (!r) { fprintf(stderr,"Error disabling '%s': %s\n", uid, openhab_last_error(c)); return 0; }
    printf("Thing '%s' disabled successfully.\n", uid);
    free(r); return 1;
}
