#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include "types.h"

//==============================================================================
// Interface
//==============================================================================

int entity_manager_init();

void entity_manager_shutdown();

entity_t entity_manager_create();

void entity_manager_destroy(entity_t entity);

int entity_manager_alive(entity_t entity);

#endif //CETECH_ENTITY_MANAGER_H
