#ifndef ENGINE_WINDOW_INCLUDED
#define ENGINE_WINDOW_INCLUDED (1)

#include "canvas.h"

#define engine_Window_max_canvases (8)
typedef struct engine_Window        engine_Window;
typedef struct engine_WindowMetrics engine_WindowMetrics;
typedef struct engine_CanvasView    engine_CanvasView;

struct engine_WindowMetrics {
    i32  x, y;                        // Window position X, Y
    u32  width, height;               // Window width, height
    u32  client_width, client_height; // Client area width, height
    bool is_focused;                  // Window focus state
    bool is_minimized;                // Window minimized state
    bool is_maximized;                // Window maximized state
};
using_Ptr$(engine_WindowMetrics);
using_Opt$(engine_WindowMetrics);

extern bool engine_WindowMetrics_eq(const engine_WindowMetrics* lhs, const engine_WindowMetrics* rhs);

struct engine_CanvasView {
    i32            x, y;          // Position in window coordinates
    i32            width, height; // Size in window coordinates
    engine_Canvas* canvas;        // Associated canvas
    bool           visible;       // Visibility flag
};
using_Ptr$(engine_CanvasView);
using_Err$(engine_CanvasView);

struct engine_Window {
    engine_Platform*     platform;
    engine_WindowMetrics metrics;
    engine_CanvasView    views[engine_Window_max_canvases];
    u32                  view_count;
    engine_Canvas*       composite_buffer; // Final composition buffer
};
using_Ptr$(engine_Window);
using_Err$(engine_Window);

// Window management
extern Err$Ptr$engine_Window engine_Window_create(const engine_PlatformParams* params) must_check;
extern void                  engine_Window_destroy(engine_Window* window);
extern Err$void              engine_Window_processEvents(engine_Window* window) must_check;
extern void                  engine_Window_present(engine_Window* window);

// Canvas view management
extern i32  engine_Window_addCanvasView(engine_Window* window, engine_Canvas* canvas, i32 x, i32 y, i32 width, i32 height);
extern void engine_Window_removeCanvasView(engine_Window* window, i32 view_id);
extern void engine_Window_updateCanvasView(engine_Window* window, i32 view_id, i32 x, i32 y, i32 width, i32 height);

// Window metrics queries
force_inline engine_Vec2i32 engine_Window_getPosition(const engine_Window* window) {
    return (engine_Vec2i32){
        .scalars = {
            window->metrics.x,
            window->metrics.y,
        },
    };
}

force_inline engine_Vec2u32 engine_Window_getSize(const engine_Window* window) {
    return (engine_Vec2u32){
        .scalars = {
            window->metrics.width,
            window->metrics.height,
        },
    };
}

force_inline engine_Vec2u32 engine_Window_getClientSize(const engine_Window* window) {
    return (engine_Vec2u32){
        .scalars = {
            window->metrics.client_width,
            window->metrics.client_height,
        },
    };
}

force_inline bool engine_Window_isFocused(const engine_Window* window) {
    return window->metrics.is_focused;
}

force_inline bool engine_Window_isMinimized(const engine_Window* window) {
    return window->metrics.is_minimized;
}

force_inline bool engine_Window_isMaximized(const engine_Window* window) {
    return window->metrics.is_maximized;
}

#endif /* ENGINE_WINDOW_INCLUDED */
