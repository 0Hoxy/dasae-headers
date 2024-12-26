#ifndef GAME_STATE_INCLUDE
#define GAME_STATE_INCLUDE (1)

#include "../../engine/include/engine/platform.h"
#include "../../engine/include/engine/input.h"

typedef struct game_State {
    f32         player_x;
    f32         player_y;
    f32         player_angle;
    f32         fov;
    f32         depth;
    const char* map;
    i32         map_width;
    i32         map_height;
    bool        is_running;
} game_State;
using_ExtTypes(game_State);

ResErr_Ptr_game_State game_State_create(void);
void                  game_State_update(Ptr_game_State state, f32 elapsed_time);

#endif // GAME_STATE_INCLUDE
