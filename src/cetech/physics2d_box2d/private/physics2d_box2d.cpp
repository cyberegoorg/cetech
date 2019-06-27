#include <celib/api.h>
#include <celib/module.h>
#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/cdb.h>

#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/renderer/gfx.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugdraw/dd.h>
#include <cetech/render_graph/render_graph.h>


#include "cetech/physics2d/physics2d.h"

#include <Box2D/Box2D.h>
#include <cetech/default_rg/default_rg.h>
#include <celib/math/math.h>

#define _G physics_global

static struct _G {
    ce_alloc_t0 *allocator;
} _G;

struct DebugDraw : b2Draw {
    void DrawPolygon(const b2Vec2 *vertices,
                     int32 vertexCount,
                     const b2Color &color) override {

        ct_dd_a0->push();
        ct_dd_a0->set_color(color.r, color.g, color.b, color.a);
        b2Vec2 p1 = vertices[vertexCount - 1];
        for (int32 i = 0; i < vertexCount; ++i) {
            b2Vec2 p2 = vertices[i];
            ct_dd_a0->move_to(p1.x, p1.y, 5.0f);
            ct_dd_a0->line_to(p2.x, p2.y, 5.0f);
            p1 = p2;
        }

        ct_dd_a0->close();
        ct_dd_a0->pop();
    }

    void DrawSolidPolygon(const b2Vec2 *vertices,
                          int32 vertexCount,
                          const b2Color &color) override {

    }

    void DrawCircle(const b2Vec2 &center,
                    float32 radius,
                    const b2Color &color) override {

    }

    void DrawSolidCircle(const b2Vec2 &center,
                         float32 radius,
                         const b2Vec2 &axis,
                         const b2Color &color) override {

    }

    void DrawSegment(const b2Vec2 &p1,
                     const b2Vec2 &p2,
                     const b2Color &color) override {

    }

    void DrawTransform(const b2Transform &xf) override {

        ce_mat4_t world_mat;
        ce_mat4_srt(world_mat.m,
                    1.0f, 1.0f, 1.0f,
                    0.0f, 0.0f, -xf.q.GetAngle(),
                    xf.p.x, xf.p.y, 0.0f);

        ct_dd_a0->set_transform_mtx(world_mat.m);
        ct_dd_a0->draw_axis(0, 0, 5.0, 100.0f, CT_DD_AXIS_NONE, 0.0f);
        ct_dd_a0->set_transform_mtx(NULL);
    }

    void DrawPoint(const b2Vec2 &p,
                   float32 size,
                   const b2Color &color) override {
    }

};

static DebugDraw gb2Draw;


// component
// WORLD2D

#define BOX2D_WORLD_COMPONENT \
    CE_ID64_0("box2d_world", 0x6353c3848cbdf789ULL)

typedef struct box2d_world_component {
    b2World *w;
} box2d_world_component;

static struct ct_ecs_component_i0 box2d_world_component_i = {
        .cdb_type = BOX2D_WORLD_COMPONENT,
        .size = sizeof(box2d_world_component),
        .is_system_state = true,
};

// BOX2D_BODY
#define BOX2D_COMPONENT \
    CE_ID64_0("box2d_body", 0xa180e6c3d0207037ULL)

typedef struct box2d_body_component {
    b2Body *body;
    union {
        struct {
            ce_vec2_t half_size;
        } rectangle;
    };
    b2FixtureDef fix_def;
    b2Fixture *fix;
} box2d_component;

static struct ct_ecs_component_i0 box2d_component_i = {
        .cdb_type = BOX2D_COMPONENT,
        .size = sizeof(box2d_component),
        .is_system_state = true,
};

// system

typedef struct spaspawn_box2d_body_data_t {
    ct_ecs_cmd_buffer_t *cmd;
} spawn_box2d_body_data_t;

typedef struct rectangle_component {
    ce_vec2_t half_size;
} rectangle_component;

static box2d_world_component *_get_b2world(ct_world_t0 world) {
    ct_entity_t0 ent = ct_ecs_q_a0->first(world,
                                          (ct_ecs_query_t0) {
                                                  .all=CT_ECS_ARCHETYPE(BOX2D_WORLD_COMPONENT)
                                          });

    auto *w = (box2d_world_component *) ct_ecs_c_a0->get_one(world, BOX2D_WORLD_COMPONENT,
                                                             ent, false);
    return w;
}

static ct_physics_world2d_c *_get_world(ct_world_t0 world) {

    ct_entity_t0 ent = ct_ecs_q_a0->first(world,
                                          (ct_ecs_query_t0) {
                                                  .all=CT_ECS_ARCHETYPE(PHYSICS_WORLD2D_COMPONENT)
                                          });

    auto *w = (ct_physics_world2d_c *) ct_ecs_c_a0->get_one(world, PHYSICS_WORLD2D_COMPONENT,
                                                            ent, false);

    return w;
}

static void _spawn_world(ct_world_t0 world,
                         ct_entity_t0 *ent,
                         ct_ecs_ent_chunk_o0 *item,
                         uint32_t n,
                         void *_data) {
    auto data = static_cast<spawn_box2d_body_data_t *>(_data);

    auto *phys_world = (ct_physics_world2d_c *) ct_ecs_c_a0->get_all(world,
                                                                     PHYSICS_WORLD2D_COMPONENT,
                                                                     item);

    for (uint32_t i = 0; i < n; ++i) {
        box2d_world_component bw = {
                .w = new b2World(b2Vec2(phys_world[i].gravity.x, phys_world[i].gravity.y))
        };

        bw.w->SetDebugDraw(&gb2Draw);

        ct_ecs_a0->buff_add_component(data->cmd, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BOX2D_WORLD_COMPONENT,
                                                      .data = &bw,
                                              }
                                      })));

    }
}

void _spawn_dynamic_body(struct ct_world_t0 world,
                         struct ct_entity_t0 *ent,
                         ct_ecs_ent_chunk_o0 *item,
                         uint32_t n,
                         void *_data) {
    spawn_box2d_body_data_t *data = static_cast<spawn_box2d_body_data_t *>(_data);

    box2d_world_component *w = _get_b2world(world);

    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *velocity = (ct_velocity2d_c *) ct_ecs_c_a0->get_all(world, VELOCITY2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        b2BodyDef bodyDef;
        b2Body *b = w->w->CreateBody(&bodyDef);

        ce_vec3_t pos = position[i].pos;
        float angle = -(rotation[i].rot.z * CE_DEG_TO_RAD);
        b->SetTransform(b2Vec2(pos.x, pos.y), angle);

        b->SetAngularVelocity(velocity[i].angular);
        b->SetLinearVelocity(b2Vec2(velocity->linear.x, velocity->linear.x));

        box2d_component result_body = {.body=b};

        ct_ecs_a0->buff_add_component(data->cmd, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BOX2D_COMPONENT,
                                                      .data = &result_body,
                                              }
                                      })));
    }
}

static void _spawn_static_body(struct ct_world_t0 world,
                               struct ct_entity_t0 *ent,
                               ct_ecs_ent_chunk_o0 *item,
                               uint32_t n,
                               void *_data) {
    spawn_box2d_body_data_t *data = static_cast<spawn_box2d_body_data_t *>(_data);

    box2d_world_component *w = _get_b2world(world);

    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        b2BodyDef bodyDef;
        b2Body *b = w->w->CreateBody(&bodyDef);

        b->SetType(b2_staticBody);

        ce_vec3_t pos = position[i].pos;
        float angle = -(rotation[i].rot.z * CE_DEG_TO_RAD);
        b->SetTransform(b2Vec2(pos.x, pos.y), angle);

        box2d_component result_body = {.body=b};

        ct_ecs_a0->buff_add_component(data->cmd, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BOX2D_COMPONENT,
                                                      .data = &result_body,
                                              }
                                      })));

    }
}

static void _collider_change(struct ct_world_t0 world,
                             struct ct_entity_t0 *ent,
                             ct_ecs_ent_chunk_o0 *item,
                             uint32_t n,
                             void *data) {
    auto *collider = (ct_collider2d_c *) ct_ecs_c_a0->get_all(world, COLLIDER2D_COMPONENT, item);

    auto *box2d = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        box2d_component *body = &box2d[i];

        if (!body->body) {
            continue;
        }

        bool recreate = false;
        b2FixtureDef fixtureDef;

        switch (collider[i].type) {
            case COLLIDER2D_RECTANGLE: {
                if (!ce_vec2_equal(body->rectangle.half_size,
                                   collider[i].rectangle.half_size, FLT_EPSILON)) {

                    ce_vec2_t half_size = collider[i].rectangle.half_size;
                    float width = half_size.x;
                    float height = half_size.y;

                    if (!width) {
                        width = 1.0f;
                    }

                    if (!height) {
                        height = 1.0f;
                    }

                    collider[i].rectangle.half_size = (ce_vec2_t) {width, height};
                    body->rectangle.half_size = collider[i].rectangle.half_size;

                    b2PolygonShape b2_shape;
                    b2_shape.SetAsBox(width, height);

                    body->fix_def.shape = &b2_shape;

                    recreate = true;
                }
            }
                break;
        }


        if (recreate) {
            if (body->fix) {
                body->body->DestroyFixture(body->fix);
            }

            body->fix = body->body->CreateFixture(&body->fix_def);
            body->fix_def = fixtureDef;
        }
    }
}

static void _destroy_all_body(struct ct_world_t0 world,
                              struct ct_entity_t0 *ent,
                              ct_ecs_ent_chunk_o0 *item,
                              uint32_t n,
                              void *_data) {
    auto *data = (spawn_box2d_body_data_t *) _data;

    for (uint32_t i = 0; i < n; ++i) {
        ct_ecs_a0->buff_remove_component(data->cmd, world, ent[i],
                                         (uint64_t[]) {BOX2D_COMPONENT}, 1);
    }
}

static void _destroy_world(struct ct_world_t0 world,
                           struct ct_entity_t0 *ent,
                           ct_ecs_ent_chunk_o0 *item,
                           uint32_t n,
                           void *_data) {
    auto *data = (spawn_box2d_body_data_t *) _data;

    auto *b2_world = (box2d_world_component *) ct_ecs_c_a0->get_all(world, BOX2D_WORLD_COMPONENT,
                                                                    item);

    for (uint32_t i = 0; i < n; ++i) {
        ct_ecs_q_a0->foreach(world,
                             (ct_ecs_query_t0) {
                                     .all=CT_ECS_ARCHETYPE(BOX2D_COMPONENT)
                             }, 0, _destroy_all_body, data);


        box2d_world_component *w = &b2_world[i];
        delete w->w;

        ct_ecs_a0->buff_remove_component(data->cmd, world, ent[i],
                                         (uint64_t[]) {BOX2D_WORLD_COMPONENT}, 1);
    }
}

static void _destroy_state(struct ct_world_t0 world,
                           struct ct_entity_t0 *ent,
                           ct_ecs_ent_chunk_o0 *item,
                           uint32_t n,
                           void *_data) {

    box2d_world_component *w = _get_b2world(world);
    if (!w->w) {
        return;
    }

    auto *data = (spawn_box2d_body_data_t *) _data;

    auto *box_body = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        box2d_component *b = &box_body[i];
        w->w->DestroyBody(b->body);
        ct_ecs_a0->buff_remove_component(data->cmd, world, ent[i],
                                         (uint64_t[]) {BOX2D_COMPONENT}, 1);
    }
}

void _transform_change(struct ct_world_t0 world,
                       struct ct_entity_t0 *ent,
                       ct_ecs_ent_chunk_o0 *item,
                       uint32_t n,
                       void *data) {
    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *body = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        b2Body *b2body = body[i].body;

        ce_vec3_t pos3 = position[i].pos;
        ce_vec2_t pos2 = {.x=pos3.x, .y=pos3.y};

        b2Vec2 cur_pos = b2body->GetPosition();
        ce_vec2_t pos = {.x=cur_pos.x, .y=cur_pos.y};

        float angle = b2body->GetAngle();

        bool change = false;
        if (!ce_vec2_equal(pos, pos2, FLT_EPSILON)) {
            pos.x = pos2.x;
            pos.y = pos2.y;
            change = true;
        }

        if (!ce_fequal(angle, -(rotation[i].rot.z * CE_DEG_TO_RAD), FLT_EPSILON)) {
            angle = -(rotation[i].rot.z * CE_DEG_TO_RAD);
            change = true;
        }

        if (change) {
            b2body->SetTransform(b2Vec2(pos.x, pos.y), angle);
        }
    }
}

void _mass_change(struct ct_world_t0 world,
                  struct ct_entity_t0 *ent,
                  ct_ecs_ent_chunk_o0 *item,
                  uint32_t n,
                  void *data) {
    auto *mass = (ct_mass2d_c *) ct_ecs_c_a0->get_all(world, MASS2D_COMPONENT, item);
    auto *body = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        b2Body *b2body = body[i].body;

        b2MassData mass_data;
        b2body->GetMassData(&mass_data);

        bool change = false;
        if (!ce_fequal(mass_data.mass, mass[i].mass, FLT_EPSILON)) {
            mass_data.mass = mass[i].mass;
            change = true;
        }

        if (change) {
            if (mass_data.mass == 0.0f) {
                b2body->SetType(b2_kinematicBody);
            } else {
                b2body->SetType(b2_dynamicBody);
            }

            b2body->SetMassData(&mass_data);
        }
    }
}

void _velocity_change(struct ct_world_t0 world,
                      struct ct_entity_t0 *ent,
                      ct_ecs_ent_chunk_o0 *item,
                      uint32_t n,
                      void *data) {
    auto *velocity = (ct_velocity2d_c *) ct_ecs_c_a0->get_all(world, VELOCITY2D_COMPONENT, item);
    auto *box_body = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        box2d_component *b = &box_body[i];
        ct_velocity2d_c *v = &velocity[i];

        ce_vec2_t b2_linear = (ce_vec2_t) {
                .x=b->body->GetLinearVelocity().x,
                .y=b->body->GetLinearVelocity().y
        };

        b2Body *b2body = b->body;

        if (!ce_vec2_equal(b2_linear, v->linear, FLT_EPSILON)) {
            b2body->SetLinearVelocity(b2Vec2(v->linear.x, v->linear.x));
        }

        if (!ce_fequal(v->angular, b2body->GetAngularVelocity(), FLT_EPSILON)) {
            b2body->SetAngularVelocity(v->angular);
        }
    }
}

static void _write_back(ct_world_t0 world,
                        ct_entity_t0 *ent,
                        ct_ecs_ent_chunk_o0 *item,
                        uint32_t n,
                        void *data) {

    auto *velocity = (ct_velocity2d_c *) ct_ecs_c_a0->get_all(world, VELOCITY2D_COMPONENT, item);
    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *body = (box2d_component *) ct_ecs_c_a0->get_all(world, BOX2D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        b2Body *b2body = body[i].body;

        b2Vec2 pos = b2body->GetPosition();
        float angle = b2body->GetAngle();

        position[i].pos.x = pos.x;
        position[i].pos.y = pos.y;

        rotation[i].rot.z = -(angle * CE_RAD_TO_DEG);

        velocity[i].angular = b2body->GetAngularVelocity();

        velocity[i].linear = (ce_vec2_t) {
                .x = b2body->GetLinearVelocity().x,
                .y = b2body->GetLinearVelocity().y
        };
    }
}

static void physics2d_system(ct_world_t0 world,
                             float dt,
                             uint32_t rq_version,
                             ct_ecs_cmd_buffer_t *cmd) {


    spawn_box2d_body_data_t data = {.cmd=cmd};

    ct_physics_world2d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    box2d_world_component *b2w = _get_b2world(world);

    if (!b2w) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                COLLIDER2D_COMPONENT),

                                        .none = CT_ECS_ARCHETYPE(VELOCITY2D_COMPONENT,
                                                                 BOX2D_COMPONENT),
                                }, rq_version,
                                _spawn_static_body, &data);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                VELOCITY2D_COMPONENT,
                                                                MASS2D_COMPONENT,
                                                                COLLIDER2D_COMPONENT),

                                        .none = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),
                                }, rq_version,
                                _spawn_dynamic_body, &data);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(COLLIDER2D_COMPONENT,
                                                                BOX2D_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),
                                        .only_changed = true,
                                }, rq_version,
                                _collider_change, &data);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                BOX2D_COMPONENT),

                                        .write = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),

                                        .only_changed = true,
                                }, rq_version,
                                _transform_change, &data);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(MASS2D_COMPONENT,
                                                                BOX2D_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),

                                        .only_changed = true,
                                }, rq_version,
                                _mass_change, &data);

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(VELOCITY2D_COMPONENT,
                                                                BOX2D_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),

                                        .only_changed = true,
                                }, rq_version,
                                _velocity_change, &data);

    float time_step = 1.0f / 60.0f;
    int velocity_iterations = 6;
    int position_iterations = 2;

    b2w->w->SetGravity(b2Vec2(w->gravity.x, w->gravity.y));
    b2w->w->Step(time_step, velocity_iterations, position_iterations);

    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                         ROTATION_COMPONENT,
                                                         VELOCITY2D_COMPONENT,
                                                         BOX2D_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                           ROTATION_COMPONENT,
                                                           VELOCITY2D_COMPONENT)
                         }, rq_version,
                         _write_back, NULL);

}

static void physics2d_spawn_world_system(ct_world_t0 world,
                                         float dt,
                                         uint32_t rq_version,
                                         ct_ecs_cmd_buffer_t *cmd) {


    spawn_box2d_body_data_t data = {.cmd=cmd};

    ct_physics_world2d_c *w = _get_world(world);

    if (!w) {
        return;
    }


    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(PHYSICS_WORLD2D_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(BOX2D_WORLD_COMPONENT),
                                }, rq_version,
                                _spawn_world, &data);
}

static void physics2d_destroy_system(ct_world_t0 world,
                                     float dt,
                                     uint32_t rq_version,
                                     ct_ecs_cmd_buffer_t *cmd) {
    spawn_box2d_body_data_t data = {.cmd=cmd};

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(BOX2D_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(COLLIDER2D_COMPONENT),
                                }, rq_version,
                                _destroy_state, &data);

}

static void physics2d_destroy_world(ct_world_t0 world,
                                    float dt,
                                    uint32_t rq_version,
                                    ct_ecs_cmd_buffer_t *cmd) {
    spawn_box2d_body_data_t data = {.cmd=cmd};
    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(BOX2D_WORLD_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(PHYSICS_WORLD2D_COMPONENT),
                                }, rq_version,
                                _destroy_world, &data);
}

#define PHYSICS2D_SPAWN_WORLD_SYSTEM \
    CE_ID64_0("spawn_world", 0xc50cd230095374ecULL)

#define PHYSICS2D_DESTROY_SYSTEM \
    CE_ID64_0("destroy", 0x57ff44bbecee397fULL)

#define PHYSICS2D_DESTROY_WORLD_SYSTEM \
    CE_ID64_0("destroy_world", 0x85a63cd5e80f23d1ULL)

static struct ct_system_i0 physics2D_spawn_world_system_i = {
        .name = PHYSICS2D_SPAWN_WORLD_SYSTEM,
        .group = PHYSICS2D_GROUP,
        .process = physics2d_spawn_world_system,
};

static struct ct_system_i0 physics2D_system_i = {
        .name = PHYSICS2D_SYSTEM,
        .group = PHYSICS2D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS2D_SPAWN_WORLD_SYSTEM),
        .process = physics2d_system,
};

static struct ct_system_i0 physics2D_destroy_system_i = {
        .name = PHYSICS2D_DESTROY_SYSTEM,
        .group = PHYSICS2D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS2D_SYSTEM),
        .process = physics2d_destroy_system,
};

static struct ct_system_i0 physics2D_destroy_worl_system_i = {
        .name = PHYSICS2D_DESTROY_WORLD_SYSTEM,
        .group = PHYSICS2D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS2D_DESTROY_SYSTEM),
        .process = physics2d_destroy_world,
};

/// Render
static void render(ct_world_t0 world,
                   struct ct_rg_builder_t0 *builder) {

    box2d_world_component *w = _get_b2world(world);

    if (!w) {
        return;
    }

    if (!w->w) {
        return;
    }

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);
    ct_dd_a0->begin(viewid);

    w->w->DrawDebugData();
    ct_dd_a0->end();
}

static struct ct_renderer_component_i0 debug_renderer_i = {
        .render = render
};

extern "C" {
void CE_MODULE_LOAD(physics_bullet)(struct ce_api_a0 *api,
                                    int reload) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_impl(CT_RENDERER_COMPONENT_I,
                  &debug_renderer_i, sizeof(debug_renderer_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics2D_system_i, sizeof(physics2D_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics2D_destroy_system_i, sizeof(physics2D_destroy_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics2D_destroy_worl_system_i, sizeof(physics2D_destroy_worl_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics2D_spawn_world_system_i, sizeof(physics2D_spawn_world_system_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &box2d_component_i, sizeof(box2d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &box2d_world_component_i, sizeof(box2d_world_component_i));

    gb2Draw.SetFlags(~0);
}

void CE_MODULE_UNLOAD(physics_bullet)(struct ce_api_a0 *api,
                                      int reload) {
    _G = (struct _G) {};
}
}