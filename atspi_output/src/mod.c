/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* to enable this module
 * export ELM_MODULES="atspi_output>access/api"
 * export ELM_ATSPI_MODE=1
 */
#include <Elementary.h>
#include <dlog.h>

#undef LOG_TAG
#define LOG_TAG "atspi-output"

#define BUS "org.tizen.ScreenReader"
#define PATH "/org/tizen/DirectReading"
#define INTERFACE "org.tizen.DirectReading"

static void (*cb_func) (void *data);
static void *cb_data;

static Eldbus_Connection *conn = NULL;
static Eldbus_Object *obj = NULL;
static Eldbus_Proxy *proxy = NULL;

static void
_on_send_text(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   char *s;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        SLOG(LOG_ERROR, LOG_TAG, "%s %s", errname, errmsg);
        return;
     }
}

EAPI int
elm_modapi_init(void *m )
{
   SLOG(LOG_DEBUG, LOG_TAG, "[START] module init");
   int ret = 0;
   Eldbus_Object *obj;
   Eldbus_Proxy *proxy;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   obj = eldbus_object_get(conn, BUS, PATH);
   proxy = eldbus_proxy_get(obj, INTERFACE);

   SLOG(LOG_DEBUG, LOG_TAG, "[END] module init");
   return 1;
}

EAPI int
elm_modapi_shutdown(void *m )
{
   SLOG(LOG_DEBUG, LOG_TAG, "[START] module shutdown");
   int ret = 1;
   if (conn)
      eldbus_connection_unref(conn);

   SLOG(LOG_DEBUG, LOG_TAG, "[END] module shutdown (ret: %d)", ret);
   return ret;
}

EAPI void
out_read(const char *txt)
{
   int ret = 0;
   SLOG(LOG_DEBUG, LOG_TAG, "[START] sending text (txt: %s)", txt);
   eldbus_proxy_call(proxy, "Hello", NULL, NULL, -1, "s", txt);
   return;
}


EAPI void
out_cancel(void)
{
   int ret = 0;
   return;
}

EAPI void
out_done_callback_set(void (*func) (void *data), const void *data)
{
   /* data and cb_data could be NULL */
   if (!func && (data != cb_data)) return;
   cb_func = func;
   cb_data = (void *)data;
}
