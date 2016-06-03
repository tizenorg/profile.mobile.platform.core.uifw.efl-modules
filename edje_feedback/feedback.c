#include <eina_module.h>
#include <Edje.h>
#include <feedback.h>

#define _FEEDBACK_PATTERN_PREFIX_LEN 17
#define _FEEDBACK_PATTERN_PREFIX     "FEEDBACK_PATTERN_"

struct _key_pair_s {
  char *key_str;
  int num;
};

static struct _key_pair_s _feedback_types[] = {
   {"FEEDBACK_TYPE_NONE", FEEDBACK_TYPE_NONE},
   {"FEEDBACK_TYPE_SOUND", FEEDBACK_TYPE_SOUND},
   {"FEEDBACK_TYPE_VIBRATION", FEEDBACK_TYPE_VIBRATION}
};

static struct _key_pair_s _feedback_patterns[] = {
   {"TAP", FEEDBACK_PATTERN_TAP},
   {"SIP", FEEDBACK_PATTERN_SIP},
   {"KEY0", FEEDBACK_PATTERN_KEY0},
   {"KEY1", FEEDBACK_PATTERN_KEY1},
   {"KEY2", FEEDBACK_PATTERN_KEY2},
   {"KEY3", FEEDBACK_PATTERN_KEY3},
   {"KEY4", FEEDBACK_PATTERN_KEY4},
   {"KEY5", FEEDBACK_PATTERN_KEY5},
   {"KEY6", FEEDBACK_PATTERN_KEY6},
   {"KEY7", FEEDBACK_PATTERN_KEY7},
   {"KEY8", FEEDBACK_PATTERN_KEY8},
   {"KEY9", FEEDBACK_PATTERN_KEY9},
   {"KEY_STAR", FEEDBACK_PATTERN_KEY_STAR},
   {"KEY_SHARP", FEEDBACK_PATTERN_KEY_SHARP},
   {"KEY_BACK", FEEDBACK_PATTERN_KEY_BACK},
   {"HOLD", FEEDBACK_PATTERN_HOLD},
   {"HW_TAP", FEEDBACK_PATTERN_HW_TAP},
   {"HW_HOLD", FEEDBACK_PATTERN_HW_HOLD},
   {"MESSAGE", FEEDBACK_PATTERN_MESSAGE},
   {"EMAIL", FEEDBACK_PATTERN_EMAIL},
   {"WAKEUP", FEEDBACK_PATTERN_WAKEUP},
   {"SCHEDULE", FEEDBACK_PATTERN_SCHEDULE},
   {"TIMER", FEEDBACK_PATTERN_TIMER},
   {"GENERAL", FEEDBACK_PATTERN_GENERAL},
   {"POWERON", FEEDBACK_PATTERN_POWERON},
   {"POWEROFF", FEEDBACK_PATTERN_POWEROFF},
   {"CHARGERCONN", FEEDBACK_PATTERN_CHARGERCONN},
   {"CHARGING_ERROR", FEEDBACK_PATTERN_CHARGING_ERROR},
   {"FULLCHARGED", FEEDBACK_PATTERN_FULLCHARGED},
   {"LOWBATT", FEEDBACK_PATTERN_LOWBATT},
   {"LOCK", FEEDBACK_PATTERN_LOCK},
   {"UNLOCK", FEEDBACK_PATTERN_UNLOCK},
   {"VIBRATION_ON", FEEDBACK_PATTERN_VIBRATION_ON},
   {"SILENT_OFF", FEEDBACK_PATTERN_SILENT_OFF},
   {"BT_CONNECTED", FEEDBACK_PATTERN_BT_CONNECTED},
   {"BT_DISCONNECTED", FEEDBACK_PATTERN_BT_DISCONNECTED},
   {"LIST_REORDER", FEEDBACK_PATTERN_LIST_REORDER},
   {"LIST_SLIDER", FEEDBACK_PATTERN_LIST_SLIDER},
   {"VOLUME_KEY", FEEDBACK_PATTERN_VOLUME_KEY}
};

static int _edje_feedback_log_dom = -1;
static int _init_count = 0;
static Eina_Bool _init_feedback = EINA_FALSE;
static int _feedback_type_count = 0;
static int _feedback_pattern_count = 0;

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_edje_feedback_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR(_edje_feedback_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_edje_feedback_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_edje_feedback_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_edje_feedback_log_dom, __VA_ARGS__)

#define ERR_GOTO(label, fmt, args...)\
   do { \
        ERR(fmt"\n", ##args); \
        goto label; \
   } while (0)

#define ERR_RET_VAL(ret, fmt, args...)\
   do { \
        ERR(fmt"\n", ##args); \
        return ret; \
   } while (0)

#define ERR_RET(fmt, args...)\
   do { \
        ERR(fmt"\n", ##args); \
        return; \
   } while (0)

#ifdef __cplusplus
extern "C" {
#endif

EXPORTAPI Eina_Bool
edje_plugin_run(Evas_Object *edje,
                const char *name,
                const char *param)
{
   if (!edje) return EINA_FALSE;
   if (!name) return EINA_FALSE;

   if (!_init_feedback)
     {
        feedback_initialize();
        _init_feedback = EINA_TRUE;
     }

   int i;
   int type = -1;
   int pattern = -1;
   unsigned int cnt;
   char **tok;
   char *tmp = NULL;
   Eina_Bool ret = EINA_FALSE;

   tok = eina_str_split_full(param, " ", 0, &cnt);
   if (tok && (cnt == 2) && tok[0] && tok[1])
     {
        feedback_error_e err = FEEDBACK_ERROR_INVALID_PARAMETER;

        for (i = 0; i < _feedback_type_count; i++)
          {
             if (!strncmp(_feedback_types[i].key_str, tok[0],
                    strlen(_feedback_types[i].key_str)))
               {
                  type = _feedback_types[i].num;
                  break;
               }
          }

        if ((i < _feedback_type_count) &&
            (!strncmp(_FEEDBACK_PATTERN_PREFIX, tok[1], _FEEDBACK_PATTERN_PREFIX_LEN)))
          {
             if ((tmp = tok[1] + _FEEDBACK_PATTERN_PREFIX_LEN) && (tmp != '\0'))
               {
                  for (i = 0; i < _feedback_pattern_count; i++)
                  {
                     if (!strncmp(_feedback_patterns[i].key_str, tmp,
                            strlen(_feedback_patterns[i].key_str)))
                       {
                          pattern = _feedback_patterns[i].num;
                          err = feedback_play_type(type, pattern);
                          break;
                       }
                  }
               }
          }


        if (err != FEEDBACK_ERROR_NONE)
          {
             WRN("feedback_play_type() failed: %d, (%s, %s)", err, tok[0], tok[1]);
             ret = EINA_FALSE;
          }
        else
          ret = EINA_TRUE;

        free(tok[0]);
        free(tok);
        return ret;
     }
   free(tok);
   return ret;
}

Eina_Bool
_module_init(void)
{
   _init_count++;
   if (_init_count == 1)
     {
        _edje_feedback_log_dom = eina_log_domain_register("elm_feedback",
                                                          EINA_COLOR_LIGHTBLUE);
        if (!_edje_feedback_log_dom)
          {
             EINA_LOG_ERR("could not register elm_feedback log domain.");
             _edje_feedback_log_dom = EINA_LOG_DOMAIN_GLOBAL;
          }
     }

   _feedback_pattern_count = (int)(sizeof(_feedback_patterns) / sizeof(_feedback_patterns[0]));
   _feedback_type_count = (int)(sizeof(_feedback_types) / sizeof(_feedback_types[0]));
   return EINA_TRUE;
}

void
_module_shutdown(void)
{
   _init_count--;
   if (_init_count == 0)
     {
        eina_log_domain_unregister(_edje_feedback_log_dom);
        _edje_feedback_log_dom = -1;
        if (_init_feedback)
          {
             feedback_deinitialize();
             _init_feedback = EINA_FALSE;
          }
     }
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
#ifdef __cplusplus
}
#endif
