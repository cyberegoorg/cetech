# cetech.Log

Module contain functions for logging

## Example

```lua
local Log = cetech.Log

Log.info("lua_example", "INFO")
Log.warning("lua_example", "WARNING")
Log.error("lua_example", "ERROR")
Log.debug("lua_example", "DEBUG")
```

## Methods

* [info( where, msg, ... )](#info-where-msg)
* [warning( where, msg, ... )](#warning-where-msg)
* [error( where, msg, ... )](#error-where-msg)
* [debug( where, msg, ... )](#debug-where-msg)

------------------------------------------------------------------------------------------------------------------------

### info( where, msg, ... )

Log info message.
  
#### Arguments
* **string** `where` - Where is message logged.
* **string** `msg` - Log message
* **object** `...` - Log message arguments

------------------------------------------------------------------------------------------------------------------------

### warning( where, msg, ...)

Log warning message.

#### Arguments
* **string** `where` - Where is message logged.
* **string** `msg` - Log message
* **object** `...` - Log message arguments

------------------------------------------------------------------------------------------------------------------------

### error( where, msg, ... )

Log error message.

#### Arguments
* **string** `where` - Where is message logged.
* **string** `msg` - Log message
* **object** `...` - Log message arguments

------------------------------------------------------------------------------------------------------------------------

### debug( where, msg, ... )

Log debug message.

#### Arguments
* **string** `where` - Where is message logged.
* **string** `msg` - Log message
* **object** `...` - Log message arguments

