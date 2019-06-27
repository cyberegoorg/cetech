#include <celib/api.h>
#include <celib/module.h>
#include <celib/memory/allocator.h>


#include "../dd.h"

#include "bgfx_debugdraw/debugdraw.h"

#define _G dd_global

static struct _G {
    DebugDrawEncoder enc;
} _G;


static void begin(uint8_t viewid) {
    _G.enc = DebugDrawEncoder();
    _G.enc.begin(viewid, true);
}

static void end() {
    _G.enc.end();
}

static Axis::Enum _to_axis_enum(ct_dd_axis_e0 axis) {
    static Axis::Enum _tbl[] = {
            [CT_DD_AXIS_NONE] = Axis::Count,
            [CT_DD_AXIS_X] = Axis::X,
            [CT_DD_AXIS_Y] = Axis::Y,
            [CT_DD_AXIS_Z] = Axis::Z,
    };

    return _tbl[axis];
}

static void draw_axis(float x,
                      float y,
                      float z,
                      float len,
                      ct_dd_axis_e0 highlight,
                      float thickness) {
    _G.enc.drawAxis(x, y, z, len, _to_axis_enum(highlight), thickness);
}

static void set_transform_mtx(float *mtx) {
    _G.enc.setTransform(mtx);
}

void move_to(float x,
             float y,
             float z) {
    _G.enc.moveTo(x, y, z);
}

void line_to(float x,
             float y,
             float z) {
    _G.enc.lineTo(x, y, z);
}

void close() {
    _G.enc.close();
}

void push() {
    _G.enc.push();
}

void pop() {
    _G.enc.pop();
}

void set_color(float r,
               float g,
               float b,
               float a) {

    union {
        struct {
            uint8_t r, g, b, a;
        };
        uint32_t argba;
    } argba = {
            .r = static_cast<uint8_t>(r * 255),
            .g = static_cast<uint8_t>(g * 255),
            .b = static_cast<uint8_t>(b * 255),
            .a = static_cast<uint8_t>(a * 255),
    };

    _G.enc.setColor(argba.argba);
}


static struct ct_dd_a0 api = {
        .begin = begin,
        .end = end,
        .push = push,
        .pop = pop,
        .set_transform_mtx = set_transform_mtx,
        .draw_axis = draw_axis,
        .move_to = move_to,
        .line_to = line_to,
        .close = close,
        .set_color = set_color,
};

struct ct_dd_a0 *ct_dd_a0 = &api;

extern "C" {
void CE_MODULE_LOAD(dd)(struct ce_api_a0 *api,
                        int reload) {
    _G = (struct _G) {};

    api->add_api(CT_DD_A0, ct_dd_a0, sizeof(struct ct_dd_a0));

    ddInit();
}

void CE_MODULE_UNLOAD(dd)(struct ce_api_a0 *api,
                          int reload) {
    ddShutdown();
    _G = (struct _G) {};
}
}