/**
 * @copyright Copyright 2024. Gyeongtae Kim All rights reserved.
 *
 * @file    color.h
 * @author  Gyeongtae Kim(dev-dasae) <codingpelican@gmail.com>
 * @date    2024-10-21 (date of creation)
 * @updated 2024-10-21 (date of last update)
 * @version v1.0.0
 * @ingroup SOME_GROUP
 * @prefix  NONE
 *
 * @brief   Header of some software
 * @details Some detailed explanation
 */


#ifndef COLOR_INCLUDED
#define COLOR_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */


#include "primitive_types.h"


typedef struct Color    Color;
typedef struct ColorHSL ColorHSL;


struct Color {
    union {
        u8 rgba[4]; // RGBA color components [0,255]
        struct {
            u8 r; // Red in [0,255]
            u8 g; // Green in [0,255]
            u8 b; // Blue in [0,255]
            u8 a; // Alpha in [0,255]
        };
    };
};
#define Color_(...)                    ((Color){ __VA_ARGS__ })
#define Color_fromRGBA(_r, _g, _b, _a) ((Color){ .rgba = { _r, _g, _b, _a } })
#define Color_fromRGB(_r, _g, _b)      Color_fromRGBA(_r, _g, _b, 255)
Color    Color_fromHSLA(ColorHSL color, u8 alpha);
Color    Color_fromHSL(ColorHSL color);
ColorHSL Color_toHSL(Color color);

static const Color Color_transparent = Color_fromRGBA(0, 0, 0, 0);
static const Color Color_black       = Color_fromRGB(0, 0, 0);
static const Color Color_white       = Color_fromRGB(255, 255, 255);
static const Color Color_red         = Color_fromRGB(255, 0, 0);
static const Color Color_green       = Color_fromRGB(0, 255, 0);
static const Color Color_blue        = Color_fromRGB(0, 0, 255);
static const Color Color_yellow      = Color_fromRGB(255, 255, 0);
static const Color Color_cyan        = Color_fromRGB(0, 255, 255);
static const Color Color_magenta     = Color_fromRGB(255, 0, 255);


struct ColorHSL {
    union {
        f64 hsl[3]; // HSL color components, Hue in [0,360], Saturation in [0,100], Lightness in [0,100]
        struct {
            f64 h; // Hue in [0,360]
            f64 s; // Saturation in [0,100]
            f64 l; // Lightness in [0,100]
        };
    };
};
#define ColorHSL_(...)     ((ColorHSL){ __VA_ARGS__ })
#define ColorHSL_from(...) ((ColorHSL){ .hsl = { __VA_ARGS__ } })
ColorHSL   ColorHSL_fromRGBA(Color color);
Color      ColorHSL_toRGBA(ColorHSL color, u8 alpha);
Color      ColorHSL_toRGB(ColorHSL color);
static f64 ColorHSL__HueToRgb(f64 p, f64 q, f64 t);


#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */
#endif /* COLOR_INCLUDED */


#if defined(DH_IMPL) && !defined(COLOR_IMPL)
#define COLOR_IMPL
#endif
#ifdef COLOR_IMPL
#define COLOR_IMPL_INCLUDED (1)

#include "floats.h"

Color Color_fromHSLA(ColorHSL color, u8 alpha) {
    return ColorHSL_toRGBA(color, alpha);
}
Color Color_fromHSL(ColorHSL color) {
    return ColorHSL_toRGBA(color, 255);
}
// RGB to HSL conversion
ColorHSL Color_toHSL(Color color) {
    f64 r = color.r / 255.0;
    f64 g = color.g / 255.0;
    f64 b = color.b / 255.0;

    f64 max  = f64_max(f64_max(r, g), b);
    f64 min  = f64_min(f64_min(r, g), b);
    f64 diff = max - min;

    f64 h = f64_nan;
    f64 s = f64_nan;
    f64 l = (max + min) / 2.0;

    if (diff != 0.0) {
        s = 0.5 < l ? diff / (2.0 - max - min) : diff / (max + min);

        if (max == r) {
            h = (g - b) / diff + g < b ? 6.0 : 0.0;
        } else if (max == g) {
            h = (b - r) / diff + 2.0;
        } else {
            h = (r - g) / diff + 4.0;
        }
        h /= 6.0;
    }
    return ColorHSL_from(
        h * 360.0,
        s * 100.0,
        l * 100.0
    );
}

ColorHSL ColorHSL_fromRGBA(Color color) {
    return Color_toHSL(color);
}
// HSL to RGBA conversion
Color ColorHSL_toRGBA(ColorHSL color, u8 alpha) {
    f64 h = color.h / 360.0;
    f64 s = color.s / 100.0;
    f64 l = color.l / 100.0;

    f64 r = f64_nan;
    f64 g = f64_nan;
    f64 b = f64_nan;

    if (s == 0.0) {
        r = g = b = l;
    } else {
        f64 q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        f64 p = 2.0 * l - q;
        r     = ColorHSL__HueToRgb(p, q, h + 1.0 / 3.0);
        g     = ColorHSL__HueToRgb(p, q, h);
        b     = ColorHSL__HueToRgb(p, q, h - 1.0 / 3.0);
    }

    return Color_fromRGBA(
        (u8)f64_clamp(r * 255.0, 0, 255.0),
        (u8)f64_clamp(g * 255.0, 0, 255.0),
        (u8)f64_clamp(b * 255.0, 0, 255.0),
        alpha
    );
}
// HSL to RGB conversion
Color ColorHSL_toRGB(ColorHSL color) {
    return ColorHSL_toRGBA(color, 255);
}
// Helper function for HSL to RGB conversion
f64 ColorHSL__HueToRgb(f64 p, f64 q, f64 t) {
    if (t < 0.0) {
        t += 1;
    }
    if (1.0 < t) {
        t -= 1.0;
    }
    if (t < 1.0 / 6.0) {
        return p + (q - p) * 6.0 * t;
    }
    if (t < 1.0 / 2.0) {
        return q;
    }
    if (t < 2.0 / 3.0) {
        return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
    }
    return p;
}

#endif /* COLOR_IMPL */
