#include <celib/api.h>
#include <celib/module.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/log.h>
#include <celib/cdb.h>
#include <celib/math/math.h>

#include <cetech/ecs/ecs.h>
#include <cetech/physics3d/physics3d.h>
#include <cetech/transform/transform.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/debugdraw/dd.h>
#include <cetech/default_rg/default_rg.h>

#include "btBulletDynamicsCommon.h"

#define _G physics3d_bullet_global

#define PHYSICS3D_SPAWN_WORLD_SYSTEM \
    CE_ID64_0("spawn_world3d", 0xa14efa7b3aa472a7ULL)

#define PHYSICS3D_DESTROY_WORLD_SYSTEM \
    CE_ID64_0("destory_world3d", 0x7a0025d41f210757ULL)


#define BULLET_WORLD_COMPONENT \
    CE_ID64_0("bullet_world", 0xabe7215aff86c8acULL)

#define BODY_COMPONENT \
    CE_ID64_0("bullet_body", 0xfcfe65c726f26b49ULL)


struct MyDebugDrawer : public btIDebugDraw {
    MyDebugDrawer() {
    }

    void drawLine(const btVector3 &from,
                  const btVector3 &to,
                  const btVector3 &color) {
        ct_dd_a0->push();
        ct_dd_a0->set_color(color.x(), color.y(), color.z(), 1.0f);
        ct_dd_a0->move_to(from.x(), from.y(), from.z());
        ct_dd_a0->line_to(to.x(), to.y(), to.z());
        ct_dd_a0->close();
        ct_dd_a0->pop();
    }

    void drawContactPoint(const btVector3 &pointOnB,
                          const btVector3 & /*normalOnB*/,
                          btScalar /*distance*/,
                          int /*lifeTime*/,
                          const btVector3 & /*color*/) {
    }

    void reportErrorWarning(const char *warningString) {
    }

    void draw3dText(const btVector3 & /*location*/,
                    const char * /*textString*/) {
    }

    void setDebugMode(int /*debugMode*/) {
    }

    int getDebugMode() const {
        return DBG_DrawWireframe
               | DBG_DrawConstraints
               | DBG_DrawConstraintLimits
               | DBG_FastWireframe;
    }
};

static struct _G {
    MyDebugDrawer dd;

    ce_alloc_t0 *allocator;
} _G;

typedef struct bullet_world_component {
    btDefaultCollisionConfiguration *configuration;
    btCollisionDispatcher *dispatcher;
    btBroadphaseInterface *interf;
    btSequentialImpulseConstraintSolver *solver;

    btDiscreteDynamicsWorld *w;
} bullet_world_component;

typedef struct body_component {
    btRigidBody *body;
} body_component;

static const char *bullet_world_dispaly_name() {
    return "Bullet world";
}

static struct ct_ecs_component_i0 bullet_world_component_i = {
        .cdb_type = BULLET_WORLD_COMPONENT,
        .size = sizeof(bullet_world_component),
        .is_system_state = true,
        .display_name = bullet_world_dispaly_name,
};

static const char *bullet_body_dispaly_name() {
    return "Bullet body";
}

static struct ct_ecs_component_i0 body_component_i = {
        .cdb_type = BODY_COMPONENT,
        .size = sizeof(body_component),
        .is_system_state = true,
        .display_name = bullet_body_dispaly_name,
};

static inline btVector3 _to_bvect(ce_vec3_t v) {
    return btVector3(v.x, v.y, v.z);
}

static inline btQuaternion _to_bquat(ce_vec4_t v) {
    return btQuaternion(v.x, v.y, v.z, v.w);
}

static inline ce_vec3_t _to_ctvect(btVector3 v) {
    return (ce_vec3_t) {
            .x = v.x(),
            .y = v.y(),
            .z = v.z(),
    };
}

static inline ce_vec4_t _to_tquat(btQuaternion v) {
    return (ce_vec4_t) {
            .x = v.x(),
            .y = v.y(),
            .z = v.z(),
            .w = v.w(),
    };
}

static ct_physics_world3d_c *_get_world(ct_world_t0 world) {

    ct_entity_t0 ent = ct_ecs_q_a0->first(world,
                                          (ct_ecs_query_t0) {
                                                  .all=CT_ECS_ARCHETYPE(PHYSICS3D_WORLD_COMPONENT)
                                          });

    auto *w = (ct_physics_world3d_c *) ct_ecs_c_a0->get_one(world, PHYSICS3D_WORLD_COMPONENT,
                                                            ent, false);

    return w;
}

static bullet_world_component *_get_bworld(ct_world_t0 world) {
    ct_entity_t0 ent = ct_ecs_q_a0->first(world,
                                          (ct_ecs_query_t0) {
                                                  .all=CT_ECS_ARCHETYPE(BULLET_WORLD_COMPONENT)
                                          });

    auto *w = (bullet_world_component *) ct_ecs_c_a0->get_one(world, BULLET_WORLD_COMPONENT,
                                                              ent, false);
    return w;
}


static void _spawn_world(ct_world_t0 world,
                         ct_entity_t0 *ent,
                         ct_ecs_ent_chunk_o0 *item,
                         uint32_t n,
                         void *_data) {
    auto data = static_cast<ct_ecs_cmd_buffer_t *>(_data);

    auto *phys_world = (ct_physics_world3d_c *) ct_ecs_c_a0->get_all(world,
                                                                     PHYSICS3D_WORLD_COMPONENT,
                                                                     item);


    for (uint32_t i = 0; i < n; ++i) {
        auto *configuration = CE_NEW(_G.allocator, btDefaultCollisionConfiguration)();
        auto *dispatcher = CE_NEW(_G.allocator, btCollisionDispatcher)(configuration);
        auto *interf = CE_NEW(_G.allocator, btDbvtBroadphase)();
        auto *solver = CE_NEW(_G.allocator, btSequentialImpulseConstraintSolver)();

        bullet_world_component bw = {
                .configuration = configuration,
                .dispatcher = dispatcher,
                .interf = interf,
                .solver = solver,
                .w = CE_NEW(_G.allocator, btDiscreteDynamicsWorld)(dispatcher,
                                                                   interf,
                                                                   solver,
                                                                   configuration)
        };

        bw.w->setDebugDrawer(&_G.dd);

        bw.w->setGravity(_to_bvect(phys_world[i].gravity));

        ct_ecs_a0->buff_add_component(data, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BULLET_WORLD_COMPONENT,
                                                      .data = &bw,
                                              }
                                      })));

    }
}

static void _spawn_world_system(ct_world_t0 world,
                                float dt,
                                uint32_t rq_version,
                                ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(PHYSICS3D_WORLD_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(BULLET_WORLD_COMPONENT),
                                }, rq_version,
                                _spawn_world, cmd);
}

static btCollisionShape *_create_shape(const ct_collider3d_c *collider) {
    switch (collider->type) {
        case COLLIDER3D_BOX:
            return CE_NEW(_G.allocator, btBoxShape)(_to_bvect(collider->shape.box.half_size));
        case COLLIDER3D_SPHERE:
            return CE_NEW(_G.allocator, btSphereShape)(collider->shape.sphere.radius);
    }

    return NULL;
}

static void _spawn_dynamic_body(struct ct_world_t0 world,
                                struct ct_entity_t0 *ent,
                                ct_ecs_ent_chunk_o0 *item,
                                uint32_t n,
                                void *_data) {
    auto *data = static_cast<ct_ecs_cmd_buffer_t *>(_data);

    bullet_world_component *w = _get_bworld(world);

    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *velocity = (ct_velocity3d_c *) ct_ecs_c_a0->get_all(world, VELOCITY3D_COMPONENT, item);
    auto *mass = (ct_mass3d_c *) ct_ecs_c_a0->get_all(world, MASS3D_COMPONENT, item);
    auto *collider = (ct_collider3d_c *) ct_ecs_c_a0->get_all(world, COLLIDER3D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        ce_vec3_t pos = position[i].pos;
        ct_collider3d_c *col = &collider[i];

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(_to_bvect(pos));
        startTransform.setRotation(_to_bquat(rotation[i].rot));

        auto *shape = _create_shape(col);
        btVector3 inertia;
        shape->calculateLocalInertia(mass[i].mass, inertia);

        auto *myMotionState = CE_NEW(_G.allocator, btDefaultMotionState)(startTransform);
        auto *body = CE_NEW(_G.allocator, btRigidBody)(mass[i].mass, myMotionState, shape, inertia);

        body->setAngularVelocity(_to_bvect(velocity[i].angular));
        body->setLinearVelocity(_to_bvect(velocity[i].linear));

        body_component result_body = {.body=body};
        w->w->addRigidBody(body);

        ct_ecs_a0->buff_add_component(data, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BODY_COMPONENT,
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
    auto *data = static_cast<ct_ecs_cmd_buffer_t *>(_data);

    bullet_world_component *w = _get_bworld(world);

    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *collider = (ct_collider3d_c *) ct_ecs_c_a0->get_all(world, COLLIDER3D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        ce_vec3_t pos = position[i].pos;
        ct_collider3d_c *col = &collider[i];

        btTransform startTransform;
        startTransform.setIdentity();
        startTransform.setOrigin(_to_bvect(pos));
        startTransform.setRotation(_to_bquat(rotation[i].rot));

        auto *shape = _create_shape(col);
        auto *myMotionState = CE_NEW(_G.allocator, btDefaultMotionState)(startTransform);
        auto *body = CE_NEW(_G.allocator, btRigidBody)(0, myMotionState, shape);

        body_component result_body = {.body=body};

        w->w->addRigidBody(body);

        ct_ecs_a0->buff_add_component(data, world, ent[i],
                                      CE_ARR_ARG(((ct_component_pair_t0[]) {
                                              {
                                                      .type = BODY_COMPONENT,
                                                      .data = &result_body,
                                              }
                                      })));
    }
}

static void _transform_change(struct ct_world_t0 world,
                              struct ct_entity_t0 *ent,
                              ct_ecs_ent_chunk_o0 *item,
                              uint32_t n,
                              void *data) {
    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *body = (body_component *) ct_ecs_c_a0->get_all(world, BODY_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        btRigidBody *b2body = body[i].body;

        ce_vec3_t pos3 = position[i].pos;

        btTransform transform = b2body->getCenterOfMassTransform();
        ce_vec3_t body_pos = _to_ctvect(transform.getOrigin());

        bool change = false;
        if (!ce_vec3_equal(pos3, body_pos, FLT_EPSILON)) {
            change = true;
        }

        if (!ce_vec4_equal(rotation[i].rot, _to_tquat(transform.getRotation()), FLT_EPSILON)) {
            change = true;
        }


        if (change) {
            auto t = b2body->getCenterOfMassTransform();
            t.setOrigin(_to_bvect(pos3));
            t.setRotation(_to_bquat(rotation[i].rot));
            b2body->setCenterOfMassTransform(t);

            b2body->activate(true);
        }
    }
}

static void _velocity_change(struct ct_world_t0 world,
                             struct ct_entity_t0 *ent,
                             ct_ecs_ent_chunk_o0 *item,
                             uint32_t n,
                             void *data) {
    auto *velocity = (ct_velocity3d_c *) ct_ecs_c_a0->get_all(world, VELOCITY3D_COMPONENT, item);
    auto *box_body = (body_component *) ct_ecs_c_a0->get_all(world, BODY_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        body_component *b = &box_body[i];

        btRigidBody *body = b->body;

        ct_velocity3d_c *v = &velocity[i];

        ce_vec3_t b2_linear = _to_ctvect(body->getLinearVelocity());


        bool changed = false;

        if (!ce_vec3_equal(b2_linear, v->linear, FLT_EPSILON)) {
            body->setLinearVelocity(_to_bvect(v->linear));
            changed = true;
        }

        ce_vec3_t angluar = _to_ctvect(body->getAngularVelocity());

        if (!ce_vec3_equal(v->angular, angluar, FLT_EPSILON)) {
            body->setAngularVelocity(_to_bvect(v->angular));
            changed = true;
        }

        if (changed) {
            body->activate(true);
        }
    }
}

static void _collider_change(struct ct_world_t0 world,
                             struct ct_entity_t0 *ent,
                             ct_ecs_ent_chunk_o0 *item,
                             uint32_t n,
                             void *data) {
    auto *collider = (ct_collider3d_c *) ct_ecs_c_a0->get_all(world, COLLIDER3D_COMPONENT, item);
    auto *body = (body_component *) ct_ecs_c_a0->get_all(world, BODY_COMPONENT, item);
    auto *mass = (ct_mass3d_c *) ct_ecs_c_a0->get_all(world, MASS3D_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        btRigidBody *rbody = body[i].body;
        ct_collider3d_c *col = &collider[i];

        const btCollisionShape *shape = rbody->getCollisionShape();

        btCollisionShape *new_shape = NULL;

        switch (col->type) {
            case COLLIDER3D_BOX: {
                if (shape->getShapeType() == BOX_SHAPE_PROXYTYPE) {
                    btBoxShape *s = (btBoxShape *) shape;
                    ce_vec3_t half = _to_ctvect(s->getHalfExtentsWithMargin());
                    if (!ce_vec3_equal(half, col->shape.box.half_size, FLT_EPSILON)) {
                        CE_DELETE(_G.allocator, s);
                        new_shape = _create_shape(col);
                    }
                } else {
                    new_shape = _create_shape(col);
                }
            }
                break;

            case COLLIDER3D_SPHERE: {
                if (shape->getShapeType() == SPHERE_SHAPE_PROXYTYPE) {
                    auto *s = (btSphereShape *) shape;
                    float radius = s->getRadius();
                    if (!ce_fequal(radius, col->shape.sphere.radius, FLT_EPSILON)) {
                        CE_DELETE(_G.allocator, s);
                        new_shape = _create_shape(col);
                    }
                } else {
                    new_shape = _create_shape(col);
                }
            }
                break;
        }

        if (new_shape) {
            rbody->setCollisionShape(new_shape);

            if (mass) {
                btVector3 inertia;
                new_shape->calculateLocalInertia(mass[i].mass, inertia);
                rbody->setMassProps(mass[i].mass, inertia);
                rbody->updateInertiaTensor();
            }

        }
    }
}

static void _write_back(ct_world_t0 world,
                        ct_entity_t0 *ent,
                        ct_ecs_ent_chunk_o0 *item,
                        uint32_t n,
                        void *data) {

    auto *velocity = (ct_velocity3d_c *) ct_ecs_c_a0->get_all(world, VELOCITY3D_COMPONENT, item);
    auto *position = (ct_position_c *) ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    auto *rotation = (ct_rotation_c *) ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    auto *body = (body_component *) ct_ecs_c_a0->get_all(world, BODY_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        btRigidBody *b2body = body[i].body;
        ce_vec4_t *rot = &rotation[i].rot;

        btTransform transform = b2body->getWorldTransform();

        position[i].pos = _to_ctvect(transform.getOrigin());

        *rot = _to_tquat(transform.getRotation());

        velocity[i].linear = _to_ctvect(b2body->getLinearVelocity());
        velocity[i].angular = _to_ctvect(b2body->getAngularVelocity());
    }
}

static void _destroy_all_body(struct ct_world_t0 world,
                              struct ct_entity_t0 *ent,
                              ct_ecs_ent_chunk_o0 *item,
                              uint32_t n,
                              void *_data) {
    auto *data = (ct_ecs_cmd_buffer_t *) _data;

    bullet_world_component *w = _get_bworld(world);

    auto *body = (body_component *) ct_ecs_c_a0->get_all(world, BODY_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        btRigidBody *b2body = body[i].body;

        btCollisionShape *shape = b2body->getCollisionShape();
        btMotionState *mstate = b2body->getMotionState();

        w->w->removeRigidBody(b2body);

        CE_DELETE(_G.allocator, shape);
        CE_DELETE(_G.allocator, mstate);
        CE_DELETE(_G.allocator, b2body);

        ct_ecs_a0->buff_remove_component(data, world, ent[i],
                                         (uint64_t[]) {BODY_COMPONENT}, 1);
    }
}

static void _destroy_world(struct ct_world_t0 world,
                           struct ct_entity_t0 *ent,
                           ct_ecs_ent_chunk_o0 *item,
                           uint32_t n,
                           void *_data) {
    auto *data = (ct_ecs_cmd_buffer_t *) _data;

    auto *b_world = (bullet_world_component *) ct_ecs_c_a0->get_all(world, BULLET_WORLD_COMPONENT,
                                                                    item);

    for (uint32_t i = 0; i < n; ++i) {
        ct_ecs_q_a0->foreach(world,
                             (ct_ecs_query_t0) {
                                     .all=CT_ECS_ARCHETYPE(BODY_COMPONENT)
                             }, 0, _destroy_all_body, data);

        btDynamicsWorld *w = b_world[i].w;
        CE_DELETE(_G.allocator, w);

        CE_DELETE(_G.allocator, b_world[i].configuration);
        CE_DELETE(_G.allocator, b_world[i].dispatcher);
        CE_DELETE(_G.allocator, b_world[i].interf);
        CE_DELETE(_G.allocator, b_world[i].solver);

        ct_ecs_a0->buff_remove_component(data, world, ent[i],
                                         (uint64_t[]) {BULLET_WORLD_COMPONENT}, 1);
    }
}

static void _spawn_dynamic_body_system(ct_world_t0 world,
                                       float dt,
                                       uint32_t rq_version,
                                       ct_ecs_cmd_buffer_t *cmd) {
//    return;
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                VELOCITY3D_COMPONENT,
                                                                MASS3D_COMPONENT,
                                                                COLLIDER3D_COMPONENT),

                                        .none = CT_ECS_ARCHETYPE(BODY_COMPONENT),
                                }, rq_version,
                                _spawn_dynamic_body, cmd);
}

static void _spawn_static_body_system(ct_world_t0 world,
                                      float dt,
                                      uint32_t rq_version,
                                      ct_ecs_cmd_buffer_t *cmd) {
//    return;
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                COLLIDER3D_COMPONENT),

                                        .none = CT_ECS_ARCHETYPE(BODY_COMPONENT,
                                                                 VELOCITY3D_COMPONENT,
                                                                 MASS3D_COMPONENT),
                                }, rq_version,
                                _spawn_static_body, cmd);
}

static void _collider_change_system(ct_world_t0 world,
                                    float dt,
                                    uint32_t rq_version,
                                    ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(COLLIDER3D_COMPONENT,
                                                                BODY_COMPONENT),
                                        .any = CT_ECS_ARCHETYPE(MASS3D_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(BODY_COMPONENT),
                                        .only_changed = true,
                                }, rq_version,
                                _collider_change, cmd);

}

static void _transform_change_system(ct_world_t0 world,
                                     float dt,
                                     uint32_t rq_version,
                                     ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                                ROTATION_COMPONENT,
                                                                BODY_COMPONENT),

                                        .write = CT_ECS_ARCHETYPE(BODY_COMPONENT),

                                        .only_changed = true,
                                }, rq_version,
                                _transform_change, cmd);

}

static void _velocity_change_system(ct_world_t0 world,
                                    float dt,
                                    uint32_t rq_version,
                                    ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(VELOCITY3D_COMPONENT,
                                                                BODY_COMPONENT),
                                        .write = CT_ECS_ARCHETYPE(BODY_COMPONENT),
                                        .only_changed = true,
                                }, rq_version,
                                _velocity_change, cmd);
}

static void physics3d_system(ct_world_t0 world,
                             float dt,
                             uint32_t rq_version,
                             ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ce_vec3_t g = _to_ctvect(bw->w->getGravity());

    if (!ce_vec3_equal(g, w->gravity, FLT_EPSILON)) {
        bw->w->setGravity(_to_bvect(w->gravity));
    }

    bw->w->stepSimulation(dt);
}

static void _write_back_system(ct_world_t0 world,
                               float dt,
                               uint32_t rq_version,
                               ct_ecs_cmd_buffer_t *cmd) {
    ct_physics_world3d_c *w = _get_world(world);

    if (!w) {
        return;
    }

    bullet_world_component *bw = _get_bworld(world);

    if (!bw) {
        return;
    }

    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                         ROTATION_COMPONENT,
                                                         VELOCITY3D_COMPONENT,
                                                         BODY_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                           ROTATION_COMPONENT,
                                                           VELOCITY3D_COMPONENT)
                         }, rq_version,
                         _write_back, NULL);
}

static void _destroy_world_system(ct_world_t0 world,
                                  float dt,
                                  uint32_t rq_version,
                                  ct_ecs_cmd_buffer_t *cmd) {
    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(BULLET_WORLD_COMPONENT),
                                        .none = CT_ECS_ARCHETYPE(PHYSICS3D_WORLD_COMPONENT),
                                }, rq_version,
                                _destroy_world, cmd);
}

#define PHYSICS3D_SPAWN_DYNAMIC_BODY_SYSTEM \
    CE_ID64_0("physics3d_spawn_dynamic_body_system", 0x778703b33420ddbcULL)

#define PHYSICS3D_SPAWN_STATIC_BODY_SYSTEM \
    CE_ID64_0("physics3d_spawn_static_body_system", 0x1bd3c5c5e8d78c9bULL)

#define PHYSICS3D_COLLIDER_CHANGE_SYSTEM \
    CE_ID64_0("physics3d_collider_change_system", 0x7575560b86a65afdULL)

#define PHYSICS3D_TRANSFORM_CHANGE_SYSTEM \
    CE_ID64_0("physics3d_transform_change_system", 0x32e9940ea17e8976ULL)

#define PHYSICS3D_VELOCITY_CHANGE_SYSTEM \
    CE_ID64_0("physics3d_velocity_change_system", 0xe766f3c72dde4906ULL)

#define PHYSICS3D_WRITE_BACK_SYSTEM \
    CE_ID64_0("physics3d_write_back_system", 0x6c2fc680d4b2fa80ULL)


static struct ct_system_i0 physics3d_spawn_world_system_i = {
        .name = PHYSICS3D_SPAWN_WORLD_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .process = _spawn_world_system,
};

static struct ct_system_i0 physics3d_spawn_dynamic_body_system_i = {
        .name = PHYSICS3D_SPAWN_DYNAMIC_BODY_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_SPAWN_WORLD_SYSTEM),
        .process = _spawn_dynamic_body_system,
};

static struct ct_system_i0 physics3d_spawn_static_body_system_i = {
        .name = PHYSICS3D_SPAWN_STATIC_BODY_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_SPAWN_DYNAMIC_BODY_SYSTEM),
        .process = _spawn_static_body_system,
};

static struct ct_system_i0 physics3d_collider_change_system_i = {
        .name = PHYSICS3D_COLLIDER_CHANGE_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_SPAWN_STATIC_BODY_SYSTEM),
        .process = _collider_change_system,
};

static struct ct_system_i0 physics3d_transform_change_system_i = {
        .name = PHYSICS3D_TRANSFORM_CHANGE_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_COLLIDER_CHANGE_SYSTEM),
        .process = _transform_change_system,
};

static struct ct_system_i0 physics3d_velocity_change_system_i = {
        .name = PHYSICS3D_VELOCITY_CHANGE_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_TRANSFORM_CHANGE_SYSTEM),
        .process = _velocity_change_system,
};

static struct ct_system_i0 physics3d_system_i = {
        .name = PHYSICS3D_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_VELOCITY_CHANGE_SYSTEM),
        .process = physics3d_system,
};

static struct ct_system_i0 physics3d_write_back_system_i = {
        .name = PHYSICS3D_WRITE_BACK_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_SYSTEM),
        .process = _write_back_system,
};

static struct ct_system_i0 physics3d_destroy_world_system_i = {
        .name = PHYSICS3D_DESTROY_WORLD_SYSTEM,
        .group = PHYSICS3D_GROUP,
        .after = CT_ECS_AFTER(PHYSICS3D_WRITE_BACK_SYSTEM),
        .process = _destroy_world_system,
};

/// Render
static void render(ct_world_t0 world,
                   ct_rg_builder_t0 *builder) {

    bullet_world_component *w = _get_bworld(world);

    if (!w) {
        return;
    }

    if (!w->w) {
        return;
    }

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);
    ct_dd_a0->begin(viewid);
    w->w->debugDrawWorld();
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

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_spawn_world_system_i, sizeof(physics3d_spawn_world_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_destroy_world_system_i, sizeof(physics3d_destroy_world_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_system_i, sizeof(physics3d_system_i));


    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_spawn_dynamic_body_system_i,
                  sizeof(physics3d_spawn_dynamic_body_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_spawn_static_body_system_i,
                  sizeof(physics3d_spawn_static_body_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_collider_change_system_i,
                  sizeof(physics3d_collider_change_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_transform_change_system_i,
                  sizeof(physics3d_transform_change_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_write_back_system_i,
                  sizeof(physics3d_write_back_system_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &physics3d_velocity_change_system_i,
                  sizeof(physics3d_velocity_change_system_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &bullet_world_component_i, sizeof(bullet_world_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &body_component_i, sizeof(body_component_i));

    api->add_impl(CT_RENDERER_COMPONENT_I,
                  &debug_renderer_i, sizeof(debug_renderer_i));
}

void CE_MODULE_UNLOAD(physics_bullet)(struct ce_api_a0 *api,
                                      int reload) {


    _G = (struct _G) {};
}
}