/***********************************************************************
**** Log handlers
***********************************************************************/

#ifndef CETECH_HANDLERS_H
#define CETECH_HANDLERS_H


/***********************************************************************
**** Includes
***********************************************************************/

#include <time.h>

/***********************************************************************
**** Stdout handler
***********************************************************************/
void log_stdout_handler(enum log_level level,
                        time_t time,
                        char worker_id,
                        const char *where,
                        const char *msg,
                        void *data);

/***********************************************************************
**** Nanomsg/mapack handler
***********************************************************************/
void nano_log_handler(enum log_level level,
                      time_t time,
                      char worker_id,
                      const char *where,
                      const char *msg,
                      void *data);

#endif //CETECH_HANDLERS_H
