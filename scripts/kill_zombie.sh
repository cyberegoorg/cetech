#!/bin/sh

ps -A | grep cetech | cut -d ' ' -f 1 | xargs kill -9 2>/dev/null
ps -A | grep bgfx | cut -d ' ' -f 1 | xargs kill -9 2>/dev/null
