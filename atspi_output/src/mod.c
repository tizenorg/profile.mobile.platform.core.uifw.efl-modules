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

static const char SIG_READ_STOP[] = "access,read,stop";
static const char SIG_READ_CANCEL[] = "access,read,cancel";

static void (*cb_func) (void *data);
static void *cb_data;

static Eldbus_Connection *conn = NULL;
static Eldbus_Object *dobj = NULL;
static Eldbus_Proxy *proxy = NULL;
static Eina_Hash *evas_obj_by_read_command_id = NULL;

static void
_on_proxy_call(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   const char *s;
   const Eina_Bool b;
   const int32_t i;
   Evas_Object *obj = data;

   SLOG(LOG_DEBUG, LOG_TAG, "[START] processing proxy call reply");

   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        SLOG(LOG_ERROR, LOG_TAG, "%s %s", errname, errmsg);
        return;
     }

   if (obj)
     {
        // get read command id and mapp it to obj
        if (eldbus_message_arguments_get(msg, "sbi", &s,&b,&i))
          {
             eina_hash_add(evas_obj_by_read_command_id,&i,obj);
          }
     }
}

static void
_on_reading_stopped(void *context EINA_UNUSED, const Eldbus_Message *msg)
{
   const int32_t i;
   if (eldbus_message_arguments_get(msg, "i", &i))
     {
        Evas_Object *obj = eina_hash_find(evas_obj_by_read_command_id,&i);
        if (obj)
          {
             evas_object_smart_callback_call(obj, SIG_READ_STOP, NULL);
          }
     }
}

static void
_on_reading_cancelled(void *context EINA_UNUSED, const Eldbus_Message *msg)
{
   const int32_t i;
   if (eldbus_message_arguments_get(msg, "i", &i))
     {
        Evas_Object *obj = eina_hash_find(evas_obj_by_read_command_id,&i);
        if (obj)
          {
             evas_object_smart_callback_call(obj, SIG_READ_CANCEL, NULL);
          }
     }
}

static void
_get_a11y_address(void *data, const Eldbus_Message * msg, Eldbus_Pending * pending)
{
   const char *sock_addr;
   const char *errname, *errmsg;
   Eldbus_Connection *session;

   SLOG(LOG_DEBUG, LOG_TAG, "[START]");
   session = data;
   if (eldbus_message_error_get(msg, &errname, &errmsg))
     {
        SLOG(LOG_ERROR, LOG_TAG, "GetAddress failed: %s %s", errname, errmsg);
        return;
     }

   if (!eldbus_message_arguments_get(msg, "s", &sock_addr) || !sock_addr)
     {
        SLOG(LOG_ERROR, LOG_TAG, "Could not get A11Y Bus socket address.");
        goto end;
     }

   if (!(conn = eldbus_address_connection_get(sock_addr)))
     {
        SLOG(LOG_ERROR, LOG_TAG, "Failed to connect to %s", sock_addr);
        goto end;
     }

   if (!(dobj = eldbus_object_get(conn, BUS, PATH)))
     {
        SLOG(LOG_ERROR, LOG_TAG, "Failed to get eldbus object");
        goto end;
     }

   if (!(proxy = eldbus_proxy_get(dobj, INTERFACE)))
     {
        SLOG(LOG_ERROR, LOG_TAG, "Failed to get proxy object");
        eldbus_object_unref(dobj);
        dobj = NULL;
     }

   SLOG(LOG_DEBUG, LOG_TAG, "Proxy object is created");

   eldbus_proxy_signal_handler_add(proxy, "ReadingStopped", _on_reading_stopped, NULL);
   eldbus_proxy_signal_handler_add(proxy, "ReadingCancelled", _on_reading_cancelled, NULL);

 end:
   eldbus_connection_unref(session);
   SLOG(LOG_DEBUG, LOG_TAG, "[END]");
}

EAPI int
elm_modapi_init(void *m )
{
   SLOG(LOG_DEBUG, LOG_TAG, "[START] module init");
   int ret = 0;
   Eldbus_Connection *session;
   Eldbus_Message *msg;

   evas_obj_by_read_command_id = eina_hash_int32_new(NULL);
   eldbus_init();
   session = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
   if (!session)
     {
        SLOG(LOG_ERROR, LOG_TAG, "Unable to get session bus");
        return;
     }

   msg = eldbus_message_method_call_new("org.a11y.Bus", "/org/a11y/bus", "org.a11y.Bus", "GetAddress");
   if (!msg)
     {
        SLOG(LOG_ERROR, LOG_TAG, "DBus message allocation failed");
        goto fail_msg;
     }

   if (!eldbus_connection_send(session, msg, _get_a11y_address, session, -1))
     {
        SLOG(LOG_ERROR, LOG_TAG, "Message send failed");
        goto fail_send;
     }

   SLOG(LOG_DEBUG, LOG_TAG, "[END] module init");
   return 1;

 fail_send:
   eldbus_message_unref(msg);
 fail_msg:
   eldbus_connection_unref(session);

   return 0;
}

EAPI int
elm_modapi_shutdown(void *m )
{
   SLOG(LOG_DEBUG, LOG_TAG, "[START] module shutdown");
   int ret = 1;
   if (conn)
      eldbus_connection_unref(conn);

   eldbus_shutdown();
   SLOG(LOG_DEBUG, LOG_TAG, "[END] module shutdown (ret: %d)", ret);
   return ret;
}

EAPI void
out_read(Evas_Object *obj, const char *txt)
{
   int ret = 0;
   SLOG(LOG_DEBUG, LOG_TAG, "[START] requesting interruptible reading (txt: %s)", txt);
   if (!proxy)
     {
        SLOG(LOG_ERROR, LOG_TAG, "Proxy object is NULL!");
        return;
     }
   //TODO: remove "Hello" call after dbus communication is verified
   eldbus_proxy_call(proxy, "Hello", _on_proxy_call, NULL, -1, "s", txt);
   eldbus_proxy_call(proxy, "ReadCommand", _on_proxy_call, obj, -1, "sb", txt, EINA_TRUE);
   return;
}


EAPI void
out_force_read(Evas_Object *obj, const char *txt)
{
   int ret = 0;
   SLOG(LOG_DEBUG, LOG_TAG, "[START] requesting uninterruptible reading (txt: %s)", txt);
   if (!proxy)
     {
        SLOG(LOG_ERROR, LOG_TAG, "Proxy object is NULL!");
        return;
     }

   eldbus_proxy_call(proxy, "ReadCommand", _on_proxy_call, obj, -1, "sb", txt, EINA_FALSE);
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
