/**
 * @copyright Copyright 2025. Gyeongtae Kim All rights reserved.
 *
 * @file    Pair.h
 * @author  Gyeongtae Kim(dev-dasae) <codingpelican@gmail.com>
 * @date    2025-03-20 (date of creation)
 * @updated 2025-03-20 (date of last update)
 * @version v0.1
 * @ingroup dasae-headers(dh)
 * @prefix  Pair
 *
 * @brief   Header of some software
 * @details Some detailed explanation
 */

#ifndef PAIR_INCLUDED
#define PAIR_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/*========== Includes =======================================================*/

#include "dh/core.h"

/*========== Macros and Declarations ========================================*/

#define Pair(T_First, T_Second)      comp_type_token__Pair(T_First, T_Second)
#define use_Pair(T_First, T_Second)  comp_type_gen__Pair(T_First, T_Second)
#define decl_Pair(T_First, T_Second) comp_type_decl__Pair(T_First, T_Second)
#define impl_Pair(T_First, T_Second) comp_type_impl__Pair(T_First, T_Second)

#define Pair$(T_First, T_Second) comp_type_anon__Pair$(T_First, T_Second)

/*========== Macros and Definitions =========================================*/

#define comp_type_token__Pair(T_First, T_Second) \
    pp_join3($, Pair, pp_cat(1, T_First), pp_cat(2, T_Second))
#define comp_type_gen__Pair(T_First, T_Second) \
    decl_Pair(T_First, T_Second);              \
    impl_Pair(T_First, T_Second)
#define comp_type_decl__Pair(T_First, T_Second) \
    typedef struct Pair(T_First, T_Second)
#define comp_type_impl__Pair(T_First, T_Second) \
    struct Pair(T_First, T_Second) {            \
        T_First  first;                         \
        T_Second second;                        \
    }
#define comp_type_anon__Pair$(T_First, T_Second) \
    struct {                                     \
        T_First  first;                          \
        T_Second second;                         \
    }

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
#endif /* PAIR_INCLUDED  */
