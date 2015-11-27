# Popis

Logovací systém slouží překvapivě pro logování zpráv. Po zalogování se zpráva předá registrovaným [handlerům](#handler_t) a ty jí zpracují.

## Include

```cpp
#include "cetech/log/log.h"
```

# Enumy

## LogLevel

```cpp
struct LogLevel {
    enum Enum {
        INFO,
        WARNING,
        ERROR,
        DBG
    };
};
```

### Hodnoty

Hodnota   | Popis
----------|--------------------
`INFO`    | Informační zpráva.
`WARNING` | Varování.
`ERROR`   | Chyba.
`DBG`     | Ladící informace.


---

# Callbacky

## handler_t
```cpp
typedef void (* handler_t)(const LogLevel::Enum level,
                           const time_t time,
                           const uint32_t worker_id,
                           const char* where,
                           const char* msg,
                           void* data);
```

Logovací handler.

### Parametry

Parametr    | Popis
------------|-------------------
`level`     | [Log level](#LogLevel) zprávy.
`time`      | Čas kdy k logování došlo.
`worker_id` | ID workeru ketrý logoval.
`where`     | Kde k logování došlo.
`msg`       | Zpráva.
`data`      | Data pro handler.

---

# Handlery

## stdout_handler

```cpp
void log::stdout_handler(const LogLevel::Enum level,
                         const time_t time,
                         const uint32_t worker_id,
                         const char* where,
                         const char* msg,
                         void* data);
```

Logovaní na standartní, chybový a výstup.

---

## file_handler

```cpp
void log::file_handler(const LogLevel::Enum level,
                       const time_t time,
                       const uint32_t worker_id,
                       const char* where,
                       const char* msg,
                       void* data);
```

Logovaní do souboru.

!!! important

    Při [registraci](#register_handler) handleru je potřeba do parametru `data` uložit ukazatel na [FILE][CFILE]

```cpp
FILE *f;
CE_CHECK_PTR(f = fopen ("logfile.txt" , "w+"));
log::register_handler(&log::file_handler, f);
```

---

## LogLevel

# Metody

## init

```cpp
void log_globals::init();
```

Inicializuje logovací systém.

---

## shutdown

```cpp
void log_globals::shutdown();
```

Ukončí logovací systém.

---

## register_handler
```cpp
void log::register_handler(handler_t handler, void* data = nullptr);
```

Zaregistruje handler.

### Parametry

Parametr      | Popis
--------------|-----------------------------------------------
`handler`     | [Callback](#handler_t)
`data`        | Data která se předají handleru při zalogování.

---

## unregister_handler
```cpp
void log::unregister_handler(handler_t handler);
```

Odregistruje handler.

### Parametry

Parametr      | Popis
--------------|-----------------------------------------------
`handler`     | [Callback](#handler_t)

---

## info
```cpp
void log::info(const char* where, const char* format, ...);
```

Zaloguje informační zprávu.

### Parametry

Parametr      | Popis
--------------|------------------------------
`where`       | Kde k logování došlo např. `"resource_manager"`.
`format`      | Formát zprávy (stejný syntax jako `printf` ).
`...`         | Formátovací proměnné.

### Ukázka

```cpp
log::info("file_manager", "loading file %s", "foo/bar.baz");
```

---

## warning
```cpp
void log::warning(const char* where, const char* format, ...);
```

Zaloguje varování.

### Parametry

Parametr      | Popis
--------------|------------------------------
`where`       | Kde k logování došlo např. `"resource_manager"`.
`format`      | Formát zprávy (stejný syntax jako `printf` ).
`...`         | Formátovací proměnné.

### Ukázka

```cpp
log::warning("file_manager", "file %s not found. Creating...", "foo/bar.baz");
```

---

## error

```cpp
void log::error(const char* where, const char* format, ...);
```

Zaloguje chybu.

### Parametry

Parametr      | Popis
--------------|--------------------------------------------------
`where`       | Kde k logování došlo např. `"resource_manager"`.
`format`      | Formát zprávy (stejný syntax jako `printf` ).
`...`         | Formátovací proměnné.

### Ukázka

```cpp
log::error("file_manager", "file %s not found.", "foo/bar.baz");
```

---

## debug

```cpp
void log::debug(const char* where, const char* format, ...);
```
Zaloguje debug informaci. Debug informace jsou povolené jenom v `debug` [konfiguraci](../../overview.md#build-konfigurace).

### Parametry

Parametr      | Popis
--------------|------------------------------
`where`       | Kde k logování došlo např. `"resource_manager"`.
`format`      | Formát zprávy (stejný syntax jako `printf` ).
`...`         | Formátovací proměnné.

### Ukázka

``` cpp
log::debug("file_manager", "file %s loaded (%fms).", "foo/bar.baz", 22.22f);
```

---

[CFILE]: http://www.cplusplus.com/reference/cstdio/FILE