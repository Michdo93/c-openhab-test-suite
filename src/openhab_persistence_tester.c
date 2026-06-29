#define OPENHAB_STATIC
#include <openhab/openhab.h>
#include <openhab/testsuite/openhab_persistence_tester.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int array_contains_name(const char* json, const char* name) {
    if (!json||!name) return 0;
    char needle[256]; snprintf(needle,sizeof(needle),"\"name\":\"%s\"",name);
    return strstr(json,needle)!=NULL;
}
static int has_data_entries(const char* json) {
    if (!json) return 0;
    const char* p=strstr(json,"\"data\""); if(!p) return 0;
    const char* q=strchr(p,'['); if(!q) return 0; q++;
    while(*q==' '||*q=='\t'||*q=='\n') q++;
    return *q!=']';
}
static char* last_data_state(const char* json) {
    if (!json) return NULL;
    const char* p=strstr(json,"\"data\""); if(!p) return NULL;
    const char* end_arr=strchr(p,']'); if(!end_arr) return NULL;
    const char* last=NULL;
    const char* s=p;
    while(s<end_arr){if(*s=='{')last=s;s++;}
    if(!last) return NULL;
    size_t len=(size_t)(end_arr-last);
    char* obj=malloc(len+1); if(!obj) return NULL;
    memcpy(obj,last,len); obj[len]='\0';
    /* extract "state" */
    const char* sp=strstr(obj,"\"state\""); char* result=NULL;
    if(sp){
        const char* vp=strchr(sp,':'); if(vp){vp++;
        while(*vp==' ') vp++;
        if(*vp=='"'){vp++;const char* ve=vp;while(*ve&&*ve!='"')ve++;
        size_t l=(size_t)(ve-vp); result=malloc(l+1);
        if(result){memcpy(result,vp,l);result[l]='\0';}}}
    }
    free(obj); return result;
}

int openhab_persistence_tester_is_item_persisted(openhab_client_t* c,
                                                  const char* service_id,
                                                  const char* item_name) {
    char* raw = openhab_persistence_getItemsFromService(c, service_id);
    if (!raw) return 0;
    int ok = array_contains_name(raw, item_name);
    free(raw); return ok;
}

int openhab_persistence_tester_has_data_in_range(openhab_client_t* c,
                                                  const char* service_id,
                                                  const char* item_name,
                                                  const char* start_time,
                                                  const char* end_time) {
    char* raw = openhab_persistence_getItemPersistenceData(
        c, item_name, service_id, start_time, end_time, 1, 50, 0);
    if (!raw) return 0;
    int ok = has_data_entries(raw);
    free(raw); return ok;
}

int openhab_persistence_tester_check_last_persisted_state(openhab_client_t* c,
                                                           const char* service_id,
                                                           const char* item_name,
                                                           const char* expected_state) {
    char* raw = openhab_persistence_getItemPersistenceData(
        c, item_name, service_id, NULL, NULL, 1, 50, 0);
    if (!raw) return 0;
    char* s = last_data_state(raw);
    free(raw);
    int ok = s && strcmp(s, expected_state)==0;
    free(s); return ok;
}
