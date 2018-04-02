#ifndef BOUNDS_H
#define BOUNDS_H

struct ct_aabb {
    float min[3];
    float max[3];
};

struct ct_cylinder {
    float pos[3];
    float end[3];
    float radius;
};

struct ct_capsule {
    float pos[3];
    float end[3];
    float radius;
};

struct ct_cone {
    float pos[3];
    float end[3];
    float radius;
};

struct ct_disk {
    float center[3];
    float normal[3];
    float radius;
};

struct ct_obb {
    float mtx[16];
};

struct ct_plane {
    float normal[3];
    float dist;
};

struct ct_ray {
    float pos[3];
    float dir[3];
};

struct ct_sphere {
    float center[3];
    float radius;
};

struct ct_tris {
    float v0[3];
    float v1[3];
    float v2[3];
};

struct ct_hit {
    float pos[3];
    float normal[3];
    float dist;
};


#endif // BOUNDS_H
