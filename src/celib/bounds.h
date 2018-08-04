#ifndef BOUNDS_H
#define BOUNDS_H

struct ce_aabb {
    float min[3];
    float max[3];
};

struct ce_cylinder {
    float pos[3];
    float end[3];
    float radius;
};

struct ce_capsule {
    float pos[3];
    float end[3];
    float radius;
};

struct ce_cone {
    float pos[3];
    float end[3];
    float radius;
};

struct ce_disk {
    float center[3];
    float normal[3];
    float radius;
};

struct ce_obb {
    float mtx[16];
};

struct ce_plane {
    float normal[3];
    float dist;
};

struct ce_ray {
    float pos[3];
    float dir[3];
};

struct ce_sphere {
    float center[3];
    float radius;
};

struct ce_tris {
    float v0[3];
    float v1[3];
    float v2[3];
};

struct ce_hit {
    float pos[3];
    float normal[3];
    float dist;
};


#endif // BOUNDS_H
