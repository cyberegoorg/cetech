#!/bin/sh

 ps -A | grep "bgfx - renderer" | tr -s " " | cut -d ' ' -f 1 | xargs kill -9
 ps -A | grep "cetech"          | tr -s " " | cut -d ' ' -f 1 | xargs kill -9
