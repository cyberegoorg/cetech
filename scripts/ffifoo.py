from cffi import FFI

ffi = FFI()
ffi.cdef("""

//==============================================================================
// Structs
//==============================================================================

struct ct_api_entry {
    void *entry;
    void *api;
};


//==============================================================================
// Api
//==============================================================================

struct ct_api_a0 {
    void (*register_api)(const char *name,
                         void *api);

    int (*exist)(const char *name);

    struct ct_api_entry (*first)(const char *name);

    struct ct_api_entry (*next)(struct ct_api_entry *entry);
};


//==============================================================================
// Enums
//==============================================================================

typedef void* va_list;
typedef uint64_t time_t;

enum ct_log_level {
    LOG_INFO = 0,
    LOG_DBG = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
};

//==============================================================================
// Handlers
//==============================================================================

//! Stdout handler
void ct_log_stdout_handler(enum ct_log_level level,
                           time_t time,
                           char worker_id,
                           const char *where,
                           const char *msg,
                           void *data);


//! Nanomsg/mapack handler
void ct_nano_log_handler(enum ct_log_level level,
                         time_t time,
                         char worker_id,
                         const char *where,
                         const char *msg,
                         void *data);


//==============================================================================
// Typedefs
//==============================================================================

//! Log handler callback
typedef void (*ct_log_handler_t)(enum ct_log_level level,
                                 time_t time,
                                 char worker_id,
                                 const char *where,
                                 const char *msg,
                                 void *data);

//! Worker id callback
typedef char (*ct_log_get_wid_clb_t)();

//==============================================================================
// Api
//==============================================================================

struct ct_log_a0 {
    void (*set_wid_clb)(ct_log_get_wid_clb_t get_wid_clb);

    //! Register log handler
    //! \param handler Handler
    //! \param data Handler data
    void (*register_handler)(ct_log_handler_t handler,
                             void *data);

    //! Log info
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*info_va)(const char *where,
                    const char *format,
                    va_list va);

    //! Log info
    //! \param where Where
    //! \param format Format
    void (*info)(const char *where,
                 const char *format,
                 ...) ;

    //! Log warnig
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*warning_va)(const char *where,
                       const char *format,
                       va_list va);

    //! Log warning
    //! \param where Where
    //! \param format Format
    void (*warning)(const char *where,
                    const char *format,
                    ...) ;

    //! Log error
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*error_va)(const char *where,
                     const char *format,
                     va_list va);

    //! Log error
    //! \param where Where
    //! \param format Format
    void (*error)(const char *where,
                  const char *format,
                  ...) ;

    //! Log debug
    //! \param where Where
    //! \param format Format
    //! \param va va args
    void (*debug_va)(const char *where,
                     const char *format,
                     va_list va);

    //! Log debug
    //! \param where Where
    //! \param format Format
    void (*debug)(const char *where,
                  const char *format,
                  ...) ;

};



struct ct_api_a0 *ct_api_get();

int cetech_kernel_init(int argc, const char **argv);
void cetech_kernel_shutdown();

void application_start();
""")

C = ffi.dlopen("./bin/linux64/libcetech_develop_lib.so")

args = [b"ctech_ffi_develop", b"-compile",  b"-continue", b"-build", b"examples/develop/build", b"-src", b"examples/develop/src"]

keep_args = [ffi.new("char[]", x) for x in args]
argv = ffi.new("char*[]", keep_args)

C.cetech_kernel_init(len(args), argv)
C.application_start()

log_void = C.ct_api_get().first(b"ct_log_a0").api
log_api = ffi.cast("struct ct_log_a0*", log_void)
log_api.info(b"python", b"python %d", ffi.cast("int", 5555))


#C.cetech_kernel_shutdown()
