/**
 * openhab_item_tester.c
 * Implementation of all item tester functions.
 * Uses cJSON for JSON parsing (bundled, header-only subset).
 */

#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_item_tester.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <time.h>

/* ── JSON helper: extract string field from flat JSON ───────────────────────
 * Minimal parser — finds "key":"value" and returns a malloc'd copy.
 * Returns NULL if not found.
 */
static char* json_str(const char* json, const char* key) {
    if (!json || !key) return NULL;
    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char* p = strstr(json, search);
    if (!p) return NULL;
    p += strlen(search);
    while (*p == ' ' || *p == ':' || *p == ' ') p++;
    if (*p != '"') return NULL;
    p++;
    const char* end = p;
    while (*end && *end != '"') {
        if (*end == '\\') end++;
        end++;
    }
    size_t len = (size_t)(end - p);
    char* result = malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, p, len);
    result[len] = '\0';
    return result;
}

/* Check if JSON array has any elements */
static int json_array_has_items(const char* json) {
    if (!json) return 0;
    const char* p = strchr(json, '[');
    if (!p) return 0;
    p++;
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    return *p != ']';
}

/* Extract "data" array length > 0 */
static int json_data_has_items(const char* json) {
    if (!json) return 0;
    const char* p = strstr(json, "\"data\"");
    if (!p) return 0;
    p = strchr(p, '[');
    if (!p) return 0;
    return json_array_has_items(p);
}

/* Find last element of "data" array and get "state" field */
static char* json_last_data_state(const char* json) {
    if (!json) return NULL;
    const char* p = strstr(json, "\"data\"");
    if (!p) return NULL;
    /* Find last { before ] */
    const char* arr_end = strchr(p, ']');
    if (!arr_end) return NULL;
    const char* last_obj = NULL;
    const char* scan = p;
    while (scan < arr_end) {
        if (*scan == '{') last_obj = scan;
        scan++;
    }
    if (!last_obj) return NULL;
    /* Copy the last object into a buffer and extract state */
    size_t len = (size_t)(arr_end - last_obj);
    char* buf  = malloc(len + 1);
    if (!buf) return NULL;
    memcpy(buf, last_obj, len);
    buf[len] = '\0';
    char* state = json_str(buf, "state");
    free(buf);
    return state;
}

/* Check if JSON array contains an object with "name" == item_name */
static int json_array_contains_name(const char* json, const char* item_name) {
    if (!json || !item_name) return 0;
    char needle[256];
    snprintf(needle, sizeof(needle), "\"name\":\"%s\"", item_name);
    return strstr(json, needle) != NULL;
}

/* to-upper in place copy */
static void str_upper(char* dst, const char* src, size_t n) {
    for (size_t i = 0; i < n-1 && src[i]; i++) dst[i] = (char)toupper((unsigned char)src[i]);
    dst[n-1] = '\0';
}

/* ── Static validators ───────────────────────────────────────────────────── */

int openhab_item_tester_is_valid_switch_value(const char* v) {
    if (!v) return 0;
    char u[8]; str_upper(u, v, sizeof(u));
    return strcmp(u,"ON")==0 || strcmp(u,"OFF")==0;
}
int openhab_item_tester_is_valid_contact_value(const char* v) {
    if (!v) return 0;
    char u[8]; str_upper(u, v, sizeof(u));
    return strcmp(u,"OPEN")==0 || strcmp(u,"CLOSED")==0;
}
int openhab_item_tester_is_valid_dimmer_value(const char* v) {
    if (!v) return 0;
    char u[16]; str_upper(u, v, sizeof(u));
    if (!strcmp(u,"ON")||!strcmp(u,"OFF")||!strcmp(u,"INCREASE")||!strcmp(u,"DECREASE")) return 1;
    char* end; double d = strtod(v, &end);
    return end != v && *end == '\0' && d >= 0.0 && d <= 100.0;
}
int openhab_item_tester_is_valid_rollershutter_value(const char* v) {
    if (!v) return 0;
    char u[16]; str_upper(u, v, sizeof(u));
    if (!strcmp(u,"UP")||!strcmp(u,"DOWN")||!strcmp(u,"STOP")||!strcmp(u,"MOVE")) return 1;
    char* end; double d = strtod(v, &end);
    return end != v && *end == '\0' && d >= 0.0 && d <= 100.0;
}
int openhab_item_tester_is_valid_color_value(const char* v) {
    if (!v) return 0;
    char u[16]; str_upper(u, v, sizeof(u));
    if (!strcmp(u,"ON")||!strcmp(u,"OFF")||!strcmp(u,"INCREASE")||!strcmp(u,"DECREASE")) return 1;
    /* HSB: "H,S,B" */
    double h, s, b; char extra;
    if (sscanf(v, "%lf,%lf,%lf%c", &h, &s, &b, &extra) == 3)
        return h>=0&&h<=360&&s>=0&&s<=100&&b>=0&&b<=100;
    return 0;
}
int openhab_item_tester_is_valid_player_value(const char* v) {
    if (!v) return 0;
    char u[16]; str_upper(u, v, sizeof(u));
    return !strcmp(u,"PLAY")||!strcmp(u,"PAUSE")||!strcmp(u,"NEXT")||
           !strcmp(u,"PREVIOUS")||!strcmp(u,"REWIND")||!strcmp(u,"FASTFORWARD");
}
int openhab_item_tester_is_valid_number_value(const char* v) {
    if (!v) return 0;
    regex_t re;
    int ok = regcomp(&re, "^-?[0-9]+(\\.[0-9]+)?([ \t][^ \t]+)?$", REG_EXTENDED|REG_NOSUB) == 0
          && regexec(&re, v, 0, NULL, 0) == 0;
    regfree(&re);
    return ok;
}
int openhab_item_tester_is_valid_datetime_value(const char* v) {
    if (!v) return 0;
    regex_t re;
    int ok = regcomp(&re,
        "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}(\\.[0-9]+)?(Z|[+-][0-9]{2}:?[0-9]{2})?$",
        REG_EXTENDED|REG_NOSUB) == 0
        && regexec(&re, v, 0, NULL, 0) == 0;
    regfree(&re);
    return ok;
}
int openhab_item_tester_is_valid_location_value(const char* v) {
    if (!v) return 0;
    double lat, lon, alt; char extra;
    int n = sscanf(v, "%lf,%lf,%lf%c", &lat, &lon, &alt, &extra);
    if (n < 2) { /* try without alt */ n = sscanf(v, "%lf,%lf%c", &lat, &lon, &extra); if (n != 2) return 0; }
    return lat>=-90&&lat<=90&&lon>=-180&&lon<=180;
}
int openhab_item_tester_is_valid_image_value(const char* v) {
    if (!v) return 0;
    return strncmp(v,"http://",7)==0 || strncmp(v,"https://",8)==0 ||
           strncmp(v,"data:image/",11)==0;
}

/* ── Existence / type / state ─────────────────────────────────────────────── */

int openhab_item_tester_does_item_exist(openhab_client_t* c, const char* item_name) {
    char* raw = openhab_items_getItem(c, item_name, NULL, 0, NULL);
    if (!raw) { fprintf(stderr,"Error: The item '%s' does not exist!\n", item_name); return 0; }
    char* name = json_str(raw, "name");
    int ok = name && strcmp(name, item_name) == 0;
    free(name); free(raw);
    if (!ok) fprintf(stderr,"Error: The item '%s' does not exist!\n", item_name);
    return ok;
}

int openhab_item_tester_check_item_is_type(openhab_client_t* c,
                                            const char* item_name,
                                            const char* item_type) {
    static const char* valid[] = {
        "Color","Contact","DateTime","Dimmer","Group","Image",
        "Location","Number","Player","Rollershutter","String","Switch", NULL
    };
    int found = 0;
    for (int i = 0; valid[i]; i++) if (strcmp(valid[i], item_type)==0) { found=1; break; }
    if (!found) { fprintf(stderr,"Error: '%s' is not a valid item type.\n", item_type); return 0; }

    char* raw = openhab_items_getItem(c, item_name, NULL, 0, NULL);
    if (!raw) { fprintf(stderr,"Error: item '%s' not found.\n", item_name); return 0; }
    char* actual = json_str(raw, "type");
    free(raw);
    if (!actual) return 0;
    /* Strip subtype suffix (e.g. "Number:Temperature" → "Number") */
    char* colon = strchr(actual, ':');
    if (colon) *colon = '\0';
    int ok = strcmp(actual, item_type) == 0;
    if (!ok) fprintf(stderr,"Error: '%s' is type '%s', expected '%s'.\n",
                     item_name, actual, item_type);
    free(actual);
    return ok;
}

int openhab_item_tester_check_item_has_state(openhab_client_t* c,
                                              const char* item_name,
                                              const char* expected) {
    char* raw = openhab_items_getItemState(c, item_name);
    if (!raw) return 0;
    /* State endpoint returns plain text or JSON string */
    char* state = raw;
    /* Strip surrounding quotes if JSON string */
    if (raw[0] == '"') {
        size_t len = strlen(raw);
        if (raw[len-1] == '"') { raw[len-1] = '\0'; state = raw+1; }
    }
    int ok = strcmp(state, expected) == 0;
    free(raw);
    return ok;
}

int openhab_item_tester_is_group_item(openhab_client_t* c, const char* item_name) {
    return openhab_item_tester_check_item_is_type(c, item_name, "Group");
}

char* openhab_item_tester_get_group_members(openhab_client_t* c, const char* group_name) {
    char* raw = openhab_items_getItem(c, group_name, NULL, 1, NULL);
    if (!raw) return NULL;
    const char* p = strstr(raw, "\"members\"");
    if (!p) { free(raw); return strdup("[]"); }
    p = strchr(p, '[');
    if (!p) { free(raw); return strdup("[]"); }
    int depth = 0;
    const char* end = p;
    do {
        if (*end == '[') depth++;
        else if (*end == ']') depth--;
        end++;
    } while (*end && depth > 0);
    size_t len = (size_t)(end - p);
    char* result = malloc(len + 1);
    memcpy(result, p, len);
    result[len] = '\0';
    free(raw);
    return result;
}

int openhab_item_tester_does_group_contain_member(openhab_client_t* c,
                                                   const char* group_name,
                                                   const char* member_name) {
    char* members = openhab_item_tester_get_group_members(c, group_name);
    if (!members) return 0;
    int ok = json_array_contains_name(members, member_name);
    free(members);
    return ok;
}

int openhab_item_tester_check_group_member_state(openhab_client_t* c,
                                                  const char* group_name,
                                                  const char* member_name,
                                                  const char* expected_state) {
    char* members = openhab_item_tester_get_group_members(c, group_name);
    if (!members) return 0;
    /* Find the member object */
    char needle[256];
    snprintf(needle, sizeof(needle), "\"name\":\"%s\"", member_name);
    const char* p = strstr(members, needle);
    int ok = 0;
    if (p) {
        /* Get surrounding object */
        while (p > members && *p != '{') p--;
        const char* end = p;
        int depth = 0;
        do {
            if (*end == '{') depth++;
            else if (*end == '}') depth--;
            end++;
        } while (*end && depth > 0);
        size_t len = (size_t)(end - p);
        char* obj = malloc(len + 1);
        memcpy(obj, p, len);
        obj[len] = '\0';
        char* state = json_str(obj, "state");
        ok = state && strcmp(state, expected_state) == 0;
        free(state); free(obj);
    }
    free(members);
    return ok;
}

/* ── SSE context for item test ───────────────────────────────────────────── */

typedef struct {
    const char* expected;
    int*        found;
    time_t      deadline;
} sse_item_ctx_t;

static int sse_item_cb(const char* data, void* userdata) {
    sse_item_ctx_t* ctx = (sse_item_ctx_t*)userdata;
    if (time(NULL) >= ctx->deadline) return 0;
    if (!data) return 1;
    /* Look for "value":"<expected>" in the payload */
    char needle[512];
    snprintf(needle, sizeof(needle), "\"value\":\"%s\"", ctx->expected);
    if (strstr(data, needle)) {
        printf("OK: item reached state '%s'.\n", ctx->expected);
        *ctx->found = 1;
        return 0;  /* stop */
    }
    return 1;  /* continue */
}

/* ── Core test runner ─────────────────────────────────────────────────────── */

static const char* CONTACT_TYPES[] = { "Contact", "Location", NULL };
static int is_update_only(const char* t) {
    for (int i = 0; CONTACT_TYPES[i]; i++)
        if (strcmp(CONTACT_TYPES[i], t)==0) return 1;
    return 0;
}

static int run_item_test(openhab_client_t* c,
                         const char* item_name,
                         const char* item_type,
                         const char* command_or_update,
                         const char* expected_state,
                         int timeout_sec) {
    char* initial_state = NULL;
    int   result        = 0;

    /* Save initial state */
    if (command_or_update && command_or_update[0]) {
        initial_state = openhab_items_getItemState(c, item_name);
        if (initial_state) {
            /* Strip quotes */
            if (initial_state[0]=='"') {
                size_t len = strlen(initial_state);
                if (initial_state[len-1]=='"') { initial_state[len-1]='\0'; memmove(initial_state,initial_state+1,len-1); }
            }
        }
    }

    /* Send command / update */
    if (command_or_update && command_or_update[0]) {
        char* r;
        if (is_update_only(item_type))
            r = openhab_items_postUpdate(c, item_name, command_or_update);
        else
            r = openhab_items_sendCommand(c, item_name, command_or_update);
        if (!r) {
            fprintf(stderr,"Error sending command to '%s': %s\n",
                    item_name, openhab_last_error(c) ? openhab_last_error(c) : "unknown");
            free(initial_state);
            return 0;
        }
        free(r);
    }

    if (!expected_state || !expected_state[0]) {
        result = 1;
    } else {
        /* SSE observation */
        int found = 0;
        sse_item_ctx_t ctx = { expected_state, &found, time(NULL) + timeout_sec };

        /* Build SSE URL for ItemStateChangedEvent */
        char url[512];
        snprintf(url, sizeof(url), "%s/rest/events?topics=openhab/items/%s/statechanged",
                 openhab_client_base_url(c), item_name);
        openhab_sse_listen(c, url, sse_item_cb, &ctx);

        result = found;
        if (!result) {
            /* Fallback: direct read */
            result = openhab_item_tester_check_item_has_state(c, item_name, expected_state);
            if (!result)
                fprintf(stderr,"Error: state of '%s' is not '%s' after %ds.\n",
                        item_name, expected_state, timeout_sec);
        }
    }

    /* Reset */
    if (initial_state && initial_state[0]) {
        char* r;
        if (is_update_only(item_type))
            r = openhab_items_postUpdate(c, item_name, initial_state);
        else
            r = openhab_items_sendCommand(c, item_name, initial_state);
        if (r) free(r);
    }
    free(initial_state);
    return result;
}

/* ── Per-type test functions ─────────────────────────────────────────────── */

#define DEFTEST(fn, type, validator, msg) \
int openhab_item_tester_##fn(openhab_client_t* c, const char* item, \
                              const char* cmd, const char* exp, int t) { \
    if (!openhab_item_tester_check_item_is_type(c, item, type)) return 0; \
    if (cmd && cmd[0] && !openhab_item_tester_##validator(cmd)) { \
        fprintf(stderr, "Invalid " msg ": '%s'\n", cmd); return 0; } \
    return run_item_test(c, item, type, cmd, exp, t); }

DEFTEST(test_switch,        "Switch",       is_valid_switch_value,        "Switch command")
DEFTEST(test_contact,       "Contact",      is_valid_contact_value,       "Contact update")
DEFTEST(test_color,         "Color",        is_valid_color_value,         "Color command")
DEFTEST(test_dimmer,        "Dimmer",       is_valid_dimmer_value,        "Dimmer command")
DEFTEST(test_rollershutter, "Rollershutter",is_valid_rollershutter_value, "Rollershutter command")
DEFTEST(test_number,        "Number",       is_valid_number_value,        "Number command")
DEFTEST(test_player,        "Player",       is_valid_player_value,        "Player command")
DEFTEST(test_datetime,      "DateTime",     is_valid_datetime_value,      "DateTime command")
DEFTEST(test_location,      "Location",     is_valid_location_value,      "Location update")
DEFTEST(test_image,         "Image",        is_valid_image_value,         "Image command")

int openhab_item_tester_test_string(openhab_client_t* c, const char* item,
                                     const char* cmd, const char* exp, int t) {
    if (!openhab_item_tester_check_item_is_type(c, item, "String")) return 0;
    return run_item_test(c, item, "String", cmd, exp, t);
}
