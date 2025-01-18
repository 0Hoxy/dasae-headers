#include "dh/main.h"
#include "dh/core.h"
#include "dh/heap/Page.h"
#include "dh/log.h"

#include "dh/debug.h"

#include "dh/math/vec.h"
#include "dh/mem/cfg.h"
#include "dh/Mat.h"

#include "engine.h"

#define window_res_width__320x200  /* template value */ (320)
#define window_res_height__320x200 /* template value */ (200)
#define window_res_width__160x100  /* template value */ (160)
#define window_res_height__160x100 /* template value */ (100)
#define window_res_width__80x50    /* template value */ (80)
#define window_res_height__80x50   /* template value */ (50)
#define window_res_width__40x25    /* template value */ (40)
#define window_res_height__40x25   /* template value */ (25)

#define window_res_width  (window_res_width__80x50)
#define window_res_height (window_res_height__80x50)
#define window_res_size   (as$(usize, window_res_width) * window_res_height)

/* (1.0 / target_fps__62_50) ~16ms => ~60 FPS, Assume 62.5 FPS for simplicity */
#define target_fps__125_0 /* template value */ (125.0)
#define target_fps__62_50 /* template value */ (62.50)
#define target_fps__50_00 /* template value */ (50.00)
#define target_fps__31_25 /* template value */ (31.25)

#define target_fps (target_fps__62_50)
#define target_spf (1.0 / target_fps)

// #define render_depth_steps (24)     // Increased for smoother rendering
// #define render_step_size   (0.005f) // Smaller steps for higher resolution
#define render_depth_steps   (12)
#define render_step_size     (0.01f)
#define heart_beat_frequency (6.0f)

use_Mat$(f32);
typedef struct RenderBuffer {
    engine_Canvas* canvas;
    Sli$f32        z_buffer;
} RenderBuffer;

// Helper function to scale colors
force_inline Color Color_scale(Color color, f32 factor) {
    return Color_fromOpaque(
        as$(u8, as$(f32, color.r) * factor),
        as$(u8, as$(f32, color.g) * factor),
        as$(u8, as$(f32, color.b) * factor)
    );
}
// Blend two colors based on factor
force_inline Color Color_blend(Color lhs, Color rhs, f32 factor) {
    return Color_fromOpaque(
        as$(u8, lhs.r * (1.0f - factor) + rhs.r * factor),
        as$(u8, lhs.g * (1.0f - factor) + rhs.g * factor),
        as$(u8, lhs.b * (1.0f - factor) + rhs.b * factor)
    );
}

// Rotation angles and scaling factor
static Vec3f angle           = math_Vec3f_zero;
static f32   scale           = 1.0f;
static f32   scale_direction = 0.001f;

// Light position and properties
static Vec3f light_pos      = literal_math_Vec3f_from(2.0f, 2.0f, -2.0f);
static Color heart_color    = literal_Color_fromOpaque(220, 20, 60); // Crimson red
static Color specular_color = literal_Color_fromOpaque(255, 255, 255);

// Calculate surface normal for the heart at a given point
Vec3f calculateHeartNormal(Vec2f point, f32 radius) {
    unused(radius); // Unused parameter
    // Partial derivatives of the heart equation
    const Vec3f dp = {
        .x = -2.0f * point.x,
        .y = -2.0f * (1.2f * point.y - fabsf(point.x) * 2.0f / 3.0f) * 1.2f,
        .z = 1.0f
    };
    return math_Vec3f_norm(dp);
}

// Calculate the radius of the heart at a given y value
f32 calculateHeartRadius(f32 t, f32 y) {
    return 0.4f + 0.05f * powf(0.5f + 0.5f * sinf(t * heart_beat_frequency + y * 2), 8);
}

// Calculate heart surface z value for a given x,y point (determins shape)
f32 calculateHeartZ(Vec2f point, f32 radius) {
    const f32 z = -point.x * point.x - powf(1.2f * point.y - fabsf(point.x) * 2.0f / 3.0f, 2) + radius * radius;
    /*
    const f32 z = -math_Vec2f_lenSq(point)
                - powf(1.2f * point.y - math_abs(point.x) * 2.0f / 3.0f, 2)
                + radius * radius;
     */
    if (z < 0.0f) { return -1.0f; /* Invalid point */ }
    return sqrtf(z) / (2.0f - point.y);
}

// Apply rotation to a point
Vec3f applyRotation(Vec3f point, Vec3f angles) {
    Vec3f rotated = math_Vec3f_rotate(point, math_Vec3f_unit_x, angles.x);
    rotated       = math_Vec3f_rotate(rotated, math_Vec3f_unit_y, angles.y);
    rotated       = math_Vec3f_rotate(rotated, math_Vec3f_unit_z, angles.z);
    return rotated;
}

// Calculate lighting for a point
Color calculateHeartLighting(Vec3f normal, Vec3f view_pos, Vec3f frag_pos) {
    // Calculate lighting vectors
    const Vec3f light_dir = math_Vec3f_norm(math_Vec3f_sub(light_pos, frag_pos));
    const Vec3f view_dir  = math_Vec3f_norm(math_Vec3f_sub(view_pos, frag_pos));

    // Calculate light angle
    const f32 ambient_strength = 0.1f;
    const f32 n_dot_l          = math_Vec3f_dot(normal, light_dir);
    // Early exit if surface faces away from light
    if (n_dot_l <= 0.0f) { return Color_scale(heart_color, ambient_strength); /* Ambient only */ }

    // Ambient light
    const Color ambient  = Color_scale(heart_color, ambient_strength);
    // Diffuse light
    const Color diffuse  = Color_scale(heart_color, n_dot_l);
    // Specular light using Blinn-Phong
    const Color specular = Color_scale(
        specular_color,
        eval(
            const Vec3f halfway_dir = math_Vec3f_norm(
                math_Vec3f_add(light_dir, view_dir)
            );
            const f32 spec = powf(
                prim_max(math_Vec3f_dot(normal, halfway_dir), 0.0f),
                64.0f
            );
            const f32   spec_mod_by_light_angle = spec * n_dot_l; // Modulate by light angle
            eval_return spec_mod_by_light_angle;
        )
    );

    // Combine components
    return Color_fromOpaque(
        as$(u8, prim_min(ambient.r + diffuse.r + specular.r, 255)),
        as$(u8, prim_min(ambient.g + diffuse.g + specular.g, 255)),
        as$(u8, prim_min(ambient.b + diffuse.b + specular.b, 255))
    );
}

// Previous functions remain the same up to renderHeart
void renderHeart(RenderBuffer* buffer, f32 t) {
    // Clear buffers
    engine_Canvas_clearDefault(buffer->canvas);

    let z_buffer = Mat_fromSli$(Mat$f32, buffer->z_buffer, window_res_width, window_res_height);
    for_slice(z_buffer.items, depth) {
        *depth = -math_f32_inf;
    }

    // Update scale with pulsing effect
    scale += scale_direction;
    if (scale < 0.8f || 1.2f < scale) {
        scale_direction = -scale_direction;
    }

    const Vec3f view_pos = math_Vec3f_scale(math_Vec3f_backward, 5.0f);

    // Render the heart
    for (f32 py = -0.5f; py <= 0.5f; py += render_step_size) {
        const f32 radius = calculateHeartRadius(t, py);

        for (f32 px = -0.5f; px <= 0.5f; px += render_step_size) {
            const Vec2f point = math_Vec2f_from(px, py);
            const f32   pz    = calculateHeartZ(point, radius);
            if (pz < 0.0f) { continue; }

            const Vec3f normal = calculateHeartNormal(point, radius);
            const f32   step_z = pz / render_depth_steps;
            for (isize i = 0; i <= (render_depth_steps * 2LL); ++i) {
                const f32 tz = -pz + ((f32)i * step_z);

                // Scale and rotate point
                const Vec3f pos = eval(
                    let         scaled  = math_Vec3f_scale(math_Vec3f_from(px, py, tz), scale);
                    let         rotated = applyRotation(scaled, angle);
                    eval_return rotated;
                );

                // Transform normal
                // const Vec3f transformed_normal = applyRotation(normal, angle);
                const Vec3f transformed_normal = math_Vec3f_norm(applyRotation(normal, angle));

                // Project to screen space
                const f32 perspective = prim_max(0.01f, 1.0f + pos.z * 0.5f);
                const i32 screen_x    = lroundf((pos.x * perspective + 0.5f) * (window_res_width - 20) + 10);
                const i32 screen_y    = lroundf((-pos.y * perspective + 0.5f) * (window_res_height - 1) + 2);

                if (screen_x < 0 || window_res_width <= screen_x) { continue; }
                if (screen_y < 0 || window_res_height <= screen_y) { continue; }

                scope_if(
                    let cell = Mat_at(z_buffer, screen_x, screen_y),
                    *cell < pos.z
                ) {
                    *cell = pos.z;
                    engine_Canvas_drawPixel(
                        buffer->canvas,
                        screen_x,
                        screen_y,
                        calculateHeartLighting(
                            transformed_normal,
                            view_pos,
                            pos
                        )
                    );
                }
            }
        }
    }

    // for next frame
    // Update rotation angles for smooth 360-degree rotation
    // Keep angles in range [0, 2π]
    angle.x = math_wrap(angle.x + 0.02f, 0, math_tau);
    angle.y = math_wrap(angle.y + 0.03f, 0, math_tau);
    angle.z = math_wrap(angle.z + 0.01f, 0, math_tau);
}

Err$void dh_main(int argc, const char* argv[]) {
    unused(argc), unused(argv);
    scope_reserveReturn(Err$void) {
        // Initialize logging to a file
        scope_if(let debug_file = fopen("main-debug.log", "w"), debug_file) {
            log_initWithFile(debug_file);
            // Configure logging behavior
            log_setLevel(log_Level_debug);
            log_showTimestamp(true);
            log_showLevel(true);
            log_showLocation(false);
            log_showFunction(true);
        }
        defer(log_fini());

        // Initialize platform with terminal backend
        let window = try(engine_Window_create(
            &(engine_PlatformParams){
                .backend_type  = engine_RenderBackendType_vt100,
                .window_title  = "Heart 3D",
                .width         = window_res_width,
                .height        = window_res_height,
                .default_color = Color_black,
            }
        ));
        defer(engine_Window_destroy(window));
        log_info("engine initialized\n");

        // Create canvases
        let game_canvas = catch (
            engine_Canvas_create(
                window_res_width,
                window_res_height,
                engine_CanvasType_rgba
            ),
            err,
            {
                log_error("Failed to create canvas: %s\n", err);
                return_err(err);
            }
        );
        defer(engine_Canvas_destroy(game_canvas));
        log_info("canvas created\n");

        engine_Canvas_clearDefault(game_canvas);
        log_info("canvas cleared\n");

        // Add canvas views
        engine_Window_addCanvasView(window, game_canvas, 0, 0, window_res_width, window_res_height);
        log_info("canvas views added\n");

        // Create render buffer with engine canvas
        var allocator = heap_Page_allocator(&(heap_Page){});
        var buffer    = (RenderBuffer){
               .canvas   = game_canvas,
               .z_buffer = meta_cast$(Sli$f32, try(mem_Allocator_alloc(allocator, typeInfo(f32), window_res_size)))
        };
        defer(mem_Allocator_free(allocator, anySli(buffer.z_buffer)));
        log_info("game state created\n");
        ignore getchar();

        // Initialize timing variables
        let time_frame_target = time_Duration_fromSecs_f64(target_spf);
        var time_frame_prev   = time_Instant_now();
        f32 time_total        = 0.0f;
        log_info("game loop started\n");

        bool is_running = true;
        while (is_running) {
            // 1) Capture the start of the frame (and capture the end-of-frame time of prev iteration’s dt includes sleep)
            let time_frame_curr = time_Instant_now();

            // 2) Compute how long since last frame (purely for your dt usage)
            let time_elapsed = time_Instant_durationSince(time_frame_curr, time_frame_prev);
            let time_dt      = time_Duration_asSecs_f64(time_elapsed);

            // 3) Process input/events
            try(engine_Window_processEvents(window));

            // 4) Update game state
            if (engine_Key_pressed(engine_KeyCode_esc)) {
                is_running = false;
            }
            time_total += 0.003f;

            // 5) Render all views
            engine_Canvas_clearDefault(game_canvas);
            renderHeart(&buffer, time_total);
            engine_Window_present(window);

            // The canvas is already updated through engine_Canvas_drawPixel calls
            // Add any additional engine-specific frame handling here

            // 6) (Optional) Display instantaneous FPS
            const f64 fps = (0.0 < time_dt) ? (1.0 / time_dt) : 9999.0;
            printf("\033[H"); // move cursor to top-left
            printf("\rFPS: %6.2f\n", fps);
            debug_only(
                // log frame every 1s
                static f64 total_game_time_for_timestamp = 0.0;
                static f64 logging_after_duration        = 0.0;
                total_game_time_for_timestamp += time_dt;
                logging_after_duration += time_dt;
                if (1.0 < logging_after_duration) {
                    logging_after_duration = 0.0;
                    log_debug("[t=%6.2f] dt: %6.2f, fps %6.2f\n", total_game_time_for_timestamp, time_dt, 1.0 / time_dt);
                }
            );

            // 7) Measure how long the update+render actually took
            let time_now        = time_Instant_now();
            let time_frame_used = time_Instant_durationSince(time_now, time_frame_curr);

            // 8) Subtract from our target; clamp to zero if negative
            let time_leftover = time_Duration_sub(time_frame_target, time_frame_used);

            const bool is_positive_time_leftover = 0 < time_leftover.secs_ || 0 < time_leftover.nanos_;
            if (is_positive_time_leftover) {
                time_sleep(time_leftover);
            }
            time_frame_prev = time_frame_curr;
        }
        return_ok({});
    }
    scope_returnReserved;
}
