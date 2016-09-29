#!/bin/sh

ps -A | tr -s " " | grep "bgfx" | cut -d ' ' -f 2 | xargs kill -9
ps -A | tr -s " " | grep "cetech" | cut -d ' ' -f 2 | xargs kill -9
