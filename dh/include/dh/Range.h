/**
 * @copyright Copyright 2024-2025. Gyeongtae Kim All rights reserved.
 *
 * @file    Range.h
 * @author  Gyeongtae Kim(dev-dasae) <codingpelican@gmail.com>
 * @date    2024-10-23 (date of creation)
 * @updated 2025-03-10 (date of last update)
 * @version v0.1-alpha.1
 * @ingroup dasae-headers(dh)
 * @prefix  Range
 *
 * @brief   Header of some software
 * @details Some detailed explanation
 */

#ifndef RANGE_INCLUDED
#define RANGE_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/*========== Includes =======================================================*/

#include "core.h"
#include "fn.h"

/*========== Macros and Declarations ========================================*/

/// @brief Range type for slice indexing operations
typedef struct Range {
    usize begin; ///< Beginning index (inclusive)
    usize end;   ///< Ending index (exclusive)
} Range;

/// @brief Create a Range from begin and end indices [begin..end] => [begin, end)
force_inline fn_(Range_from(usize begin, usize end), Range);
/// @brief Get the length of a Range
force_inline fn_(Range_len(Range self), usize);
/// @brief Check if an index is within a Range
force_inline fn_(Range_contains(Range self, usize index), bool);
/// @brief Check if Range is valid (begin < end)
force_inline fn_(Range_isValid(Range self), bool);

/// @brief Compare two Ranges for equality
force_inline fn_(Range_eq(Range lhs, Range rhs), bool);
/// @brief Compare two Ranges for inequality
force_inline fn_(Range_ne(Range lhs, Range rhs), bool);

/*========== Macros and Definitions =========================================*/

force_inline fn_(Range_from(usize begin, usize end), Range) {
    debug_assert_fmt(begin < end, "Invalid range: begin(%zu) >= end(%zu)", begin, end);
    return (Range){ .begin = begin, .end = end };
}
force_inline fn_(Range_len(Range self), usize) { return self.end - self.begin; }
force_inline fn_(Range_contains(Range self, usize index), bool) { return self.begin <= index && index < self.end; }
force_inline fn_(Range_isValid(Range self), bool) { return self.begin < self.end; }

force_inline fn_(Range_eq(Range lhs, Range rhs), bool) { return lhs.begin == rhs.begin && lhs.end == rhs.end; }
force_inline fn_(Range_ne(Range lhs, Range rhs), bool) { return !Range_eq(lhs, rhs); }

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */
#endif /* RANGE_INCLUDED */
