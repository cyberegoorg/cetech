#!/bin/sh

 ps -Al | grep "bgfx - renderer" | tr -s " " | cut -d ' ' -f 4 | xargs kill -9
 ps -Al | grep "cetech"          | tr -s " " | cut -d ' ' -f 4 | xargs kill -9
