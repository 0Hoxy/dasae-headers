/**
 * @copyright Copyright 2024. Gyeongtae Kim All rights reserved.
 *
 * @file    builtin.h
 * @author  Gyeongtae Kim(dev-dasae) <codingpelican@gmail.com>
 * @date    2024-11-06 (date of creation)
 * @updated 2025-01-11 (date of last update)
 * @version v0.1-alpha.1
 * @ingroup dasae-headers(dh)
 * @prefix  builtin
 *
 * @brief   Builtin configurations and compiler-specific macros
 * @details Includes architecture, compiler, language and platform specific
 *          configurations. Serves as a central point for builtin definitions
 *          and compiler-related functionality used throughout the project.
 */

#ifndef BUILTIN_INCLUDED
#define BUILTIN_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/*========== Includes =======================================================*/

#include "builtin/arch_cfg.h"
#include "builtin/comp_cfg.h"
#include "builtin/lang_cfg.h"
#include "builtin/pltf_cfg.h"
#include "builtin/comp.h"
#include "builtin/auto.h"
#include "builtin/type_info.h"
#include "builtin/container_info.h"
#include "builtin/lambda.h"

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
#endif /* BUILTIN_INCLUDED */
