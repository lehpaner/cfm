#pragma once

#define SARAAPI_MESSAGES_BASE               (/*WM_USER + */ 1000)

#define SARAAPIMESSAGERANGE                 1000

#define CATTYPES_MESSAGES_BASE              (SARAAPI_MESSAGES_BASE + SARAAPIMESSAGERANGE * 0)
/* ATTENZIONE!!! LASCIARE 13000 POSIZIONI LIBERE DOPO CATTYPES_MESSAGES_BASE */
#define BASESID_MESSAGES_BASE               (SARAAPI_MESSAGES_BASE + SARAAPIMESSAGERANGE * 13)
#define EVENTMANAGER_MESSAGES_BASE          (SARAAPI_MESSAGES_BASE + SARAAPIMESSAGERANGE * 14)
#define BASECOMM_MESSAGES_BASE              (SARAAPI_MESSAGES_BASE + SARAAPIMESSAGERANGE * 15)
