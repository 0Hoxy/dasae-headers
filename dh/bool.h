/**
 * @copyright Copyright 2024. Gyeongtae Kim All rights reserved.
 *
 * @file    bool.h
 * @author  Gyeongtae Kim(dev-dasae) <codingpelican@gmail.com>
 * @date    2024-10-28 (date of creation)
 * @updated 2024-10-28 (date of last update)
 * @version v1.0.0
 * @ingroup dasae-headers(dh)
 * @prefix  bool
 *
 * @brief   Header of some software
 * @details Some detailed explanation
 */


#ifndef BOOL_INCLUDED
#define BOOL_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */


#include "compat/compat_bool.h"


/* Boolean operations that work identically in C and C++ */
#define bool_and(_lhs, _rhs) ((_lhs) && (_rhs))
#define bool_or(_lhs, _rhs)  ((_lhs) || (_rhs))
#define bool_not(_x)         (!(_x))
#define bool_xor(_lhs, _rhs) ((bool)((_lhs) != (_rhs)))

/* Boolean conversion macros */
#define to_bool(_x)   ((bool)(!!(_x)))
#define from_bool(_x) ((int)(_x))

#define bool_const(_x)                           \
    /* Compile-time boolean constant creation */ \
    ((bool)(!!(_x)))


#if defined(__cplusplus)
} /* extern "C" */
#endif /* __cplusplus */
#endif /* BOOL_INCLUDED */
