// build `clang -xc fireworks.c ../dh/src/*.c -o fireworks -DNDEBUG -O3 -static`
// run with `.\launcher fireworks 160 50`

/*
use crossterm::style::Print;
use crossterm::terminal::{ Clear, ClearType };
use crossterm::{ cursor, execute, terminal };
use pixel_loop::canvas::CrosstermCanvas;
use pixel_loop::input::{ CrosstermInputState, KeyboardKey, KeyboardState };
use pixel_loop::{ Canvas, Color, HslColor, RenderableCanvas };
*/


#include <dh/core/prim.h>
#include <dh/core/pp.h>
#include <dh/core.h>
#include <dh/debug/assert.h>

#include <dh/mem.h>
#include <dh/defer.h>
#include <dh/random.h>

// FIXME: Expected memory access error
#include "../dh-terminal/terminal.h"
// #include "../dh-terminal/input_state.h"
// #include "../dh-terminal/keyboard_key.h"
// #include "../dh-terminal/keyboard_state.h"
#include "../dh-terminal/color.h"
#include "../dh-terminal/canvas.h"
#include "../dh-terminal/window.h" /* WIP FIXING HEADER INTERNAL */

#include <locale.h>
#include <stdio.h>


// TODO(dev-dasae): Move to library and make it more generic.

#define Terminal_Width  (80ull * 2)
#define Terminal_Height (25ull * 2)
#define Terminal_Size   (Terminal_Width * Terminal_Height)

#define Terminal_FontWidth  (6)
#define Terminal_FontHeight (Terminal_FontWidth * 2)

#define Display_Width  (Terminal_Width)
#define Display_Height (Terminal_Height * 2)
#define Display_Size   (Display_Width * Display_Height)

void TerminalCursor_resetColor(void) {
    printf("\033[0m");
}

void TerminalCursor_setColor(Color foreground, Color background) {
    printf(
        "\033[38;2;%d;%d;%d;48;2;%d;%d;%dm", // background
        foreground.r,
        foreground.g,
        foreground.b,
        background.r,
        background.g,
        background.b
    );
}

// Set locale to UTF-8
void Terminal_ensureLocaleUTF8(void) {
    static const char* const locales[]      = { "en_US.UTF-8", "C.UTF-8", ".UTF-8", "" };
    static const i32         locales_length = sizeof(locales) / sizeof(locales[0]);

    const char* setted_locale = NULL;

    for (i32 i = 0; i < locales_length; ++i) {
        setted_locale = setlocale(LC_ALL, locales[i]);
        if (setted_locale) {
            printf("Successfully set locale to: %s\n", setted_locale);
            break;
        }
    }

    if (!setted_locale) {
        printf("Warning: Failed to set UTF-8 locale. Using system default.\n");
        setted_locale = setlocale(LC_ALL, "");
        if (!setted_locale) {
            perror("Failed to set locale\n");
            // Not exiting here, as the program might still work
        }
    }

#ifdef _WIN32
    // Set console output to UTF-8 on Windows
    if (!SetConsoleOutputCP(CP_UTF8)) {
        perror("Failed to set console output to UTF-8\n");
        // Not exiting here, as the program might still work
    }
#endif
}

// Enable ANSI escape sequence processing
void Terminal_enableANSI(void) {
#ifdef _WIN32
    HANDLE hOut   = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
    SetConsoleMode(hOut, dwMode);
#else
    // Not needed for Linux as it supports ANSI escape sequences by default
#endif
}

// Print diagnostic information for debugging
void Terminal_printDiagnosticInformation(void) {
    printf("Current locale: %s\n", setlocale(LC_ALL, NULL));
#ifdef _WIN32
    printf("Active code page: %d\n", GetACP());
    printf("Output code page: %d\n", GetConsoleOutputCP());
#endif
}

// Clear the terminal screen and move the cursor to the top left
void Terminal_clear(void) {
#ifdef _WIN32
    HANDLE                     hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD                      topLeft  = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO screen   = { 0 };
    DWORD                      written  = { 0 };

    GetConsoleScreenBufferInfo(hConsole, &screen);
    FillConsoleOutputCharacter(hConsole, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(hConsole, 0, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    // FillConsoleOutputAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(hConsole, topLeft);
#else
    // Clear screen and scrollback buffer
    printf("\033[2J\033[3J\033[H");
#endif
}

void Terminal_bootup(void) {
    Terminal_ensureLocaleUTF8();
    Terminal_enableANSI();
    Terminal_printDiagnosticInformation();
    printf("Terminal setup complete.\n");
    printf("Press any key to continue...\n");
    getch();
    Terminal_clear();
}

void Terminal_shutdown(void) {
    TerminalCursor_resetColor();
    Terminal_clear();
}

#define Display_UnitPixel1x2Format            "\033[38;2;%d;%d;%d;48;2;%d;%d;%dm▀"
#define Display_UnitPixel1x2FormatMaxCase     "\033[38;2;255;255;255;48;2;255;255;255m▀"
#define Display_UnitPixel1x2FormatMaxCaseSize (sizeof(Display_UnitPixel1x2FormatMaxCase) / sizeof(Display_UnitPixel1x2FormatMaxCase[0]))
#define Display_BufferSize                    ((usize)Terminal_Size * (usize)Display_UnitPixel1x2FormatMaxCaseSize)
#define Display_shows_title_in_buffer         true

static char  Display_front_buffer[Display_BufferSize] = { 0 };
static char  Display_back_buffer[Display_BufferSize]  = { 0 };
static char* Display_buffer_curr                      = Display_front_buffer;
static usize Display_buffer_curr_size                 = 0;
static char* Display_buffer_next                      = Display_back_buffer;
static usize Display_buffer_next_size                 = 0;

void Display_init(void) {
#ifdef _WIN32
    HANDLE     hConsoleOutput    = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSizeInitial = (SMALL_RECT){ 0, 0, 1, 1 };

    SetConsoleWindowInfo(hConsoleOutput, TRUE, &windowSizeInitial);

    // void SetConsoleScreenBuffer()
    COORD dwSize = (COORD){ (SHORT)Terminal_Width, (SHORT)Terminal_Height + Display_shows_title_in_buffer };
    SetConsoleActiveScreenBuffer(hConsoleOutput);
    SetConsoleScreenBufferSize(hConsoleOutput, dwSize);

    // void SetConsoleFontSize()
    CONSOLE_FONT_INFOEX fontInfo = (CONSOLE_FONT_INFOEX){ 0 };
    fontInfo.cbSize              = sizeof(CONSOLE_FONT_INFOEX);
    GetCurrentConsoleFontEx(hConsoleOutput, FALSE, &fontInfo);
    fontInfo.dwFontSize.X = Terminal_FontWidth;
    fontInfo.dwFontSize.Y = Terminal_FontHeight;
    SetCurrentConsoleFontEx(hConsoleOutput, FALSE, &fontInfo);

    // void SetConsoleWindowSize()
    SMALL_RECT windowSize = (SMALL_RECT){ 0, 0, (SHORT)Terminal_Width - 1, (SHORT)Terminal_Height - 1 + Display_shows_title_in_buffer };
    SetConsoleWindowInfo(hConsoleOutput, TRUE, &windowSize);

    // void DisableConsoleCursor()
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsoleOutput, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsoleOutput, &cursorInfo);
#endif
}

void Display_clear(void) {
    memset(Display_buffer_curr, 0, Display_buffer_curr_size);
}

void Display_swapBuffers(void) {
    swap(char*, Display_buffer_curr, Display_buffer_next);
    swap(usize, Display_buffer_curr_size, Display_buffer_next_size);
}

void Display_setBufferFromColors(const Window* const window, const Color colors[Display_Size]) {
    Display_clear();
    usize index = 0;

    // Render title line if enabled
    if (Display_shows_title_in_buffer) {
        // Black background, White text ANSI sequence
        const char* const title_seq = "\033[38;2;255;255;255;48;2;0;0;0m";
        memcpy(Display_buffer_next + index, title_seq, strlen(title_seq));
        index += strlen(title_seq);

        // Add title content
        const char* const title = Window_title(window, true);
        memcpy(Display_buffer_next + index, title, strlen(title));
        index += strlen(title);

        // Fill rest of line with spaces
        for (usize i = strlen(title); i < Display_Width; ++i) {
            Display_buffer_next[index++] = ' ';
        }
        Display_buffer_next[index++] = '\n';
    }

    for (usize y = 0; y < Display_Height; y += 2) {
        usize x = 0;
        while (x < Display_Width) {
            // Ensure we don't go out of bounds
            if (Display_Height <= (y + 1)) { break; }

            const Color upper = colors[x + y * Display_Width];
            const Color lower = colors[x + (y + 1) * Display_Width];

            // Start a run of characters with the same color
            usize run_length = 1;
            while ((x + run_length) < (usize)Display_Width) {
                const Color next_upper = colors[x + run_length + y * Display_Width];
                const Color next_lower = colors[x + run_length + (y + 1) * Display_Width];

                if (memcmp(&upper.rgba, &next_upper.rgba, sizeof(Color)) != 0
                    || memcmp(&lower.rgba, &next_lower.rgba, sizeof(Color)) != 0) {
                    break;
                }
                run_length++;
            }

            // Construct ANSI escape sequence once for the run
            const i32 formatted_size = sprintf(
                Display_buffer_next + index,
                "\033[38;2;%d;%d;%d;48;2;%d;%d;%dm",
                upper.r,
                upper.g,
                upper.b,
                lower.r,
                lower.g,
                lower.b
            );
            index += formatted_size;

            // Append the block character '▀' runLength times
            const char* const block_char        = "▀"; // Multibyte character
            const usize       block_char_length = strlen(block_char);

            // Copy the multibyte character multiple times
            for (usize i = 0; i < run_length; ++i) {
                memcpy(Display_buffer_next + index, block_char, block_char_length);
                index += block_char_length;
            }

            x += run_length;
        }
        // Append newline at the end of each line
        Display_buffer_next[index++] = '\n';
    }
    Display_buffer_next[--index] = '\0';
    Display_buffer_next_size     = index;

    Display_swapBuffers();
}

void Display_render() {
    // Reset cursor position
    printf("\033[H");

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  written  = 0;
    WriteConsoleA(hConsole, Display_buffer_curr, (DWORD)Display_buffer_curr_size, &written, NULL);
#else
    fwrite(Display_bufferCurrent, sizeof(Display_bufferCurrent[0]), Display_bufferCurrentSize, stdout);
    fflush(stdout);
#endif
}

Duration Display_testOverhead(Window* const window, Canvas* const canvas) {
    Duration overhead_duration = Duration_zero;

    Color* const buffer = FrameBuffer_accessData(Canvas_accessBuffer(canvas));

    const u64 overhead_iterations = 32;
    for (usize measurements = 0; measurements < overhead_iterations; ++measurements) {
        Instant start = Instant_now();
        for (usize i = 0; i < Display_Size; ++i) {
            buffer[i] = Color_fromOpaque(0, 0, 0);
        }
        Display_setBufferFromColors(window, buffer);
        Display_render();
        overhead_duration = Duration_add(overhead_duration, Instant_elapsed(start));
    }
    overhead_duration = Duration_div(overhead_duration, overhead_iterations);

    return overhead_duration;
}


#define Firework_effects_max        (25)
#define Firework_effects_per_rocket (25)
#define Fireworks_max               (16)


typedef struct Particle {
    f64   position[2];
    f64   speed[2];
    f64   acceleration[2];
    f64   fading;
    f64   lifetime;
    Color color;
    u32   dimensions[2];
} Particle;

Particle* Particle_init(Particle* const p, f64 x, f64 y, f64 width, f64 height, Color color) {
    *p = makeWith(
        Particle,
        .position     = { x, y },
        .speed        = { 0.0, 0.0 },
        .acceleration = { 0.0, 0.0 },
        .fading       = 0.0,
        .lifetime     = 1.0,
        .color        = color,
        .dimensions   = { width, height }
    );
    return p;
}

Particle* Particle_withSpeed(Particle* const p, f64 x, f64 y) {
    debug_assertNotNull(p);
    p->speed[0] = x;
    p->speed[1] = y;
    return p;
}

Particle* Particle_withAcceleration(Particle* const p, f64 x, f64 y) {
    debug_assertNotNull(p);
    p->acceleration[0] = x;
    p->acceleration[1] = y;
    return p;
}

Particle* Particle_withFading(Particle* const p, f64 fading) {
    debug_assertNotNull(p);
    p->fading = fading;
    return p;
}

bool Particle_isDead(const Particle* const p) {
    debug_assertNotNull(p);
    return p->lifetime <= 0.0;
}

void Particle_update(Particle* const p) {
    debug_assertNotNull(p);
    if (Particle_isDead(p)) { return; }

    p->speed[0] += p->acceleration[0];
    p->speed[1] += p->acceleration[1];

    p->position[0] += p->speed[0];
    p->position[1] += p->speed[1];

    p->lifetime -= p->fading;
}

void Particle_render(const Particle* const p, Canvas* const canvas) {
    debug_assertNotNull(p);
    if (Particle_isDead(p)) { return; }

    const Color render_color = Color_fromOpaque(
        prim_as(u8, prim_as(f64, p->color.r) * p->lifetime),
        prim_as(u8, prim_as(f64, p->color.g) * p->lifetime),
        prim_as(u8, prim_as(f64, p->color.b) * p->lifetime)
    );

    Canvas_fillRect(
        canvas,
        Canvas_normalizeRect(
            canvas,
            comptime_Rect_from(
                prim_as(i64, p->position[0]),
                prim_as(i64, p->position[1]),
                prim_as(u32, p->dimensions[0]),
                prim_as(u32, p->dimensions[1])
            )
        ),
        render_color
    );
}


typedef struct Firework {
    Particle* rocket;
    struct Vector_Particle {
        Particle* data;
        usize     length;
        usize     capacity;
    } effects;
    HSL effect_base_color;
} Firework;

Firework* Firework_init(Firework* const f, i64 x, i64 y, Color effect_base_color) {
    Particle* const rocket = mem_create(Particle);
    Particle_init(rocket, (f64)x, (f64)y, 1, 3, Color_white);
    Particle_withSpeed(rocket, 0.0, -2.0 - random_f64(random_rng) * -1.0);
    Particle_withAcceleration(rocket, 0.0, 0.02);

    *f = makeWith(
        Firework,
        .rocket  = rocket,
        .effects = {
            .data     = mem_alloc(Particle, Firework_effects_per_rocket),
            .length   = 0,
            .capacity = Firework_effects_per_rocket },
        .effect_base_color = Color_intoHSL(effect_base_color)
    );

    return f;
}

Firework* Firework_fini(Firework* const f) {
    if (f->rocket) {
        mem_destroy(&f->rocket);
    }
    if (f->effects.data) {
        mem_free(&f->effects.data);
    }
    return f;
}

void Firework_update(Firework* const f) {
    if (f->rocket) {
        Particle_update(f->rocket);
        if (-0.2 <= f->rocket->speed[1]) {
            for (i64 i = 0; i < Firework_effects_per_rocket; ++i) {
                if (Firework_effects_max <= f->effects.length) { break; }

                const i64 x      = prim_as(i64, f->rocket->position[0]);
                const i64 y      = prim_as(i64, f->rocket->position[1]);
                const i64 width  = 1;
                const i64 height = 1;
                const HSL color  = HSL_from(
                    f->effect_base_color.h,
                    f->effect_base_color.s + (random_f64(random_rng) - 0.5) * 20.0,
                    f->effect_base_color.l + (random_f64(random_rng) - 0.5) * 40.0
                );

                Particle* const particle = Particle_init(
                    create(Particle),
                    prim_as(f64, x),
                    prim_as(f64, y),
                    prim_as(f64, width),
                    prim_as(f64, height),
                    HSL_intoColorOpaque(color)
                );
                Particle_withSpeed(
                    particle,
                    (random_f64(random_rng) - 0.5) * 1.0,
                    (random_f64(random_rng) - 0.9) * 1.0
                );
                Particle_withAcceleration(particle, 0.0, 0.02);
                Particle_withFading(particle, 0.01);

                f->effects.data[f->effects.length++] = *particle;
            }
            mem_destroy(&f->rocket);
        }
    }
    foreach (Particle, particle, &f->effects) {
        Particle_update(particle);
    }
}

void Firework_render(const Firework* const f, Canvas* const canvas) {
    if (f->rocket) {
        Particle_render(f->rocket, canvas);
    }
    foreach (const Particle, particle, &f->effects) {
        Particle_render(particle, canvas);
    }
}

static bool Firework__deadsAllEffect(const Firework* const f) {
    foreach (const Particle, particle, &f->effects) {
        if (Particle_isDead(particle)) { continue; }
        return false;
    }
    return true;
}

bool Firework_isDead(const Firework* const f) {
    return f->rocket == null && (f->effects.length == 0 || Firework__deadsAllEffect(f));
}


typedef struct State {
    struct Vector_Firework {
        Firework* data;
        usize     length;
        usize     capacity;
    } fireworks;
    u32 width;
    u32 height;
} State;

State* State_init(State* const s, u32 width, u32 height) {
    *s = makeWith(
        State,
        .fireworks = {
            .data     = mem_alloc(Firework, Fireworks_max),
            .length   = 0,
            .capacity = Fireworks_max },
        .width  = width,
        .height = height
    );
    return s;
}

void State_spawnFirework(State* const s) {
    if (s->fireworks.capacity <= s->fireworks.length) { return; }
    Firework_init(
        &s->fireworks.data[s->fireworks.length++],
        prim_as(i64, random_u32(random_rng) % s->width),
        s->height,
        Color_fromOpaque(
            random_u8(random_rng),
            random_u8(random_rng),
            random_u8(random_rng)
        )
    );
}

void State_update(State* const s) {
    // Remove dead fireworks.
    foreach (Firework, firework, &s->fireworks) {
        if (!Firework_isDead(firework)) { continue; }
        Firework_fini(firework);
        *firework = s->fireworks.data[--s->fireworks.length];
        --firework;
    }
    // Add a new rocket with with 5% chance.
    if (random_f64(random_rng) < 0.05) {
        State_spawnFirework(s);
    }
    // Update all fireworks.
    foreach (Firework, firework, &s->fireworks) {
        Firework_update(firework);
    }
}

void State_fini(State* const s) {
    foreach (Firework, firework, &s->fireworks) {
        Firework_fini(firework);
        mem_free(&firework);
    }
}


// TODO: Apply descriptive type
typedef struct WindowTargetFrame {
    f32 rate;
} WindowTargetFrame;
const f32 target_frame_62_50_per_s = 62.50f;
const f32 target_frame_31_25_per_s = 31.25f;
const f32 target_frame_10_00_per_s = 10.00f;


// FIXME: Fix how defer is called/used
void Window_cleanup_defer(Window* w) {
    Window_fini(w);
    mem_destroy(&w);
}
void State_cleanup_defer(State* s) {
    State_fini(s);
    mem_destroy(&s);
}
void Canvas_cleanup_defer(Canvas* c) {
    Canvas_fini(c);
    mem_destroy(&c);
}
void Terminal_cleanup_defer(anyptr null_args) {
    pp_unused(null_args);
    Terminal_shutdown();
}


int main(int argc, const char* argv[]) {
    pp_unused(argc);
    pp_unused(argv);

    random_init(random_rng);

    defer_scope {
        Window* const window = mem_create(Window);
        {
            Window_init(window, WindowConfig_default);
            Window_withSize(window, 160, 50);
            Window_withTitle(window, "firework", Display_shows_title_in_buffer);
            Window_withFrameRate(window, true, target_frame_62_50_per_s, false, true);
        }
        defer(Window_cleanup_defer, window);

        State* const state = mem_create(State);
        {
            State_init(state, Window_width(window), Window_height(window) * 2);
        }
        defer(State_cleanup_defer, state);

        Canvas* const canvas = mem_create(Canvas);
        {
            Canvas_init(canvas, state->width, state->height);
            Canvas_withColor(canvas, Color_black);
        }
        defer(Canvas_cleanup_defer, canvas);

        Display_init();
        Terminal_bootup();
        defer(Terminal_cleanup_defer, null);

        bool is_running = true;
        while (is_running) {
            Window_update(window);

            /* UPDATE BEGIN */ {
                // TODO: use wrapper module instead of this
                if (GetAsyncKeyState(VK_ESCAPE)) {
                    is_running = false;
                }
                if (GetAsyncKeyState(VK_SPACE)) {
                    State_spawnFirework(state);
                }

                State_update(state);
            } /* UPDATE END */
            /* RENDER BEGIN */ {
                Canvas_clear(canvas, Color_black);
                foreach (const Firework, firework, &state->fireworks) {
                    Firework_render(firework, canvas);
                }
                // Canvas_render(canvas);
                {
                    Display_setBufferFromColors(window, FrameBuffer_peekData(Canvas_peekBuffer(canvas)));
                    Display_render();
                }
            } /* RENDER END */

            Window_delay(window);
        }
    }

    return 0;
}

/* fn main() -> Result<()> {
    let(terminal_width, terminal_height) = terminal::size() ? ;
    let width                            = terminal_width;
    let height                           = terminal_height * 2;

    let mut canvas = CrosstermCanvas::new (width, height);
    canvas.set_refresh_limit(120);

    let state = State::new (width as u32, height as u32);

    let input = CrosstermInputState::new ();

    pixel_loop::run(
        60,
        state,
        input,
        canvas,
        | e, s, input, canvas | {
            let width = canvas.width();
            let height = canvas.height();

            if input {
                .is_key_pressed(KeyboardKey::Q) {
                    // @HACK until we refactored PixelLoop to allow for a clean
                    // exit.
                    let mut stdout = std::io::stdout();
                    execute!(
                        stdout,
                        Clear(ClearType::All), // Clear all on screen
                        cursor::MoveTo(0, 0),  // Reset cursor position
                        Print("\x1b[!p"),      // Soft terminal reset (DECSTR)
                        Print("\x1bc"),        // Full terminal reset (RIS)
                    )?;
                    crossterm::terminal::disable_raw_mode()?;
                    std::process::exit(0);
                }
            }

            // eprintln!("Active fireworks: {}", s.fireworks.len());

            // Remove dead fireworks.
            s.fireworks.retain(|f| !f.is_dead());

            // Add a new rocket with with 5% chance.
            if rand {
                ::random::<f64>() < 0.05 {
                    let x = (rand::random::<u32>() % width) as i64;
                    let y = height as i64;
                    let            effect_base_color = Color::from_rgb(
                        rand::random::<u8>(),
                        rand::random::<u8>(),
                        rand::random::<u8>(),
                    );

                    s.fireworks.push(Firework::new (x, y, effect_base_color));
                }
            }

            for {
                firework in& mut s.fireworks {
                    firework.update();
                }
            }

            Ok(())
        }, | e, s, i, canvas, dt | {
            // RENDER BEGIN
            canvas.clear_screen(&Color::from_rgb(0, 0, 0));

            for {
                firework in& s.fireworks {
                    firework.render(canvas)?;
                }
            }
            // RENDER END

            canvas.render()?;

            Ok(())
        },
    )? ;
    Ok(())
}
 */
