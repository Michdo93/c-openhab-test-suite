#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_rule_tester.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

static char* extract_status_block(const char* json) {
    /* Extract content of "status":{...} block */
    if (!json) return NULL;
    const char* p = strstr(json, "\"status\"");
    if (!p) return NULL;
    const char* b = strchr(p+8, '{');
    if (!b) return NULL;
    const char* e = b+1; int d=1;
    while (*e && d>0) { if(*e=='{')d++; else if(*e=='}')d--; e++; }
    size_t len=(size_t)(e-b);
    char* r=malloc(len+1); if(!r) return NULL;
    memcpy(r,b,len); r[len]='\0'; return r;
}

char* openhab_rule_tester_get_rule_status(openhab_client_t* c, const char* uid) {
    char* raw = openhab_rules_getRule(c, uid);
    if (!raw) { fprintf(stderr,"Error reading status of rule '%s': %s\n",uid,openhab_last_error(c)); return NULL; }
    char* sblock = extract_status_block(raw);
    char* status  = sblock ? json_str_field(sblock,"status")       : NULL;
    char* detail  = sblock ? json_str_field(sblock,"statusDetail") : NULL;
    free(sblock); free(raw);
    char* result  = malloc(256);
    snprintf(result,256,"{\"status\":\"%s\",\"statusDetail\":\"%s\"}",
             status  ? status  : "UNKNOWN",
             detail  ? detail  : "UNKNOWN");
    free(status); free(detail);
    return result;
}

static char* rule_status_value(openhab_client_t* c, const char* uid) {
    char* sr = openhab_rule_tester_get_rule_status(c, uid);
    if (!sr) return strdup("UNKNOWN");
    char* s = json_str_field(sr,"status");
    free(sr);
    return s ? s : strdup("UNKNOWN");
}
static char* rule_detail_value(openhab_client_t* c, const char* uid) {
    char* sr = openhab_rule_tester_get_rule_status(c, uid);
    if (!sr) return strdup("NONE");
    char* d = json_str_field(sr,"statusDetail");
    free(sr);
    return d ? d : strdup("NONE");
}

int openhab_rule_tester_is_rule_active(openhab_client_t* c, const char* uid) {
    char* s=rule_status_value(c,uid); int ok=strcmp(s,"UNINITIALIZED")!=0; free(s); return ok;
}
int openhab_rule_tester_is_rule_disabled(openhab_client_t* c, const char* uid) {
    char* s=rule_status_value(c,uid); char* d=rule_detail_value(c,uid);
    int ok=strcmp(s,"UNINITIALIZED")==0 && strcmp(d,"DISABLED")==0;
    free(s); free(d); return ok;
}
int openhab_rule_tester_is_rule_running(openhab_client_t* c, const char* uid) {
    char* s=rule_status_value(c,uid); int ok=strcmp(s,"RUNNING")==0; free(s); return ok;
}
int openhab_rule_tester_is_rule_idle(openhab_client_t* c, const char* uid) {
    char* s=rule_status_value(c,uid); int ok=strcmp(s,"IDLE")==0; free(s); return ok;
}

int openhab_rule_tester_enable_rule(openhab_client_t* c, const char* uid) {
    char* r = openhab_rules_setRuleState(c, uid, 1);
    if (!r) { fprintf(stderr,"Error enabling rule '%s': %s\n",uid,openhab_last_error(c)); return 0; }
    printf("Rule '%s' enabled successfully.\n", uid);
    free(r); return 1;
}
int openhab_rule_tester_disable_rule(openhab_client_t* c, const char* uid) {
    char* r = openhab_rules_setRuleState(c, uid, 0);
    if (!r) { fprintf(stderr,"Error disabling rule '%s': %s\n",uid,openhab_last_error(c)); return 0; }
    printf("Rule '%s' disabled successfully.\n", uid);
    free(r); return 1;
}
int openhab_rule_tester_run_rule(openhab_client_t* c, const char* uid, const char* ctx) {
    if (openhab_rule_tester_is_rule_disabled(c,uid)) {
        fprintf(stderr,"Error: Rule '%s' is disabled.\n",uid); return 0;
    }
    char* r = openhab_rules_runNow(c, uid, ctx);
    if (!r) { fprintf(stderr,"Error executing rule '%s': %s\n",uid,openhab_last_error(c)); return 0; }
    printf("Rule '%s' executed successfully.\n", uid);
    free(r); return 1;
}
int openhab_rule_tester_test_rule_execution(openhab_client_t* c, const char* uid,
                                             const char* expected_item,
                                             const char* expected_value) {
    if (!openhab_rule_tester_run_rule(c,uid,NULL)) return 0;
    sleep(2);
    char* state = openhab_items_getItemState(c, expected_item);
    if (!state) { fprintf(stderr,"Error reading state of '%s'\n",expected_item); return 0; }
    /* Strip quotes */
    char* s = state;
    if (s[0]=='"') { size_t l=strlen(s); if(s[l-1]=='"'){s[l-1]='\0';s++;} }
    int ok = strcmp(s, expected_value)==0;
    if (ok) printf("OK: item '%s' = '%s'.\n", expected_item, s);
    else fprintf(stderr,"Error: '%s' expected '%s', found '%s'.\n",expected_item,expected_value,s);
    free(state); return ok;
}
