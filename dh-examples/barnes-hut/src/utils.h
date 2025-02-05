#ifndef UTILS_INCLUDED
#define UTILS_INCLUDED (1)

#include "dh/core.h"
#include "dh/meta.h"
#include "dh/mem/Allocator.h"
#include "dh/ArrList.h"
#include "Body.h"

// Helper function to perform a safe multiplication, avoiding potential overflow
use_ErrSet(utils_MulErr, Overflow);
use_ErrSet$(utils_MulErr, usize);
force_inline utils_MulErr$usize utils_usize_mulSafe(usize lhs, usize rhs) must_check;

// Insertion sort for small arrays
extern void utils_insertionSort(
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs)
);

// Insertion sort with arg
extern void utils_insertionSortWithArg(
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs, anyptr_const arg),
    anyptr_const arg
);

use_ErrSet(
    utils_SortErr,
    utils_MulErr_Overflow,
    mem_Alloc_OutOfMemory
);
// Modernized merge sort using temporary buffer instead of allocating new memory
extern utils_SortErr$void utils_mergeSortUsingTempRecur(
    Sli$u8   temp_buffer,
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs)
) must_check;

// Modernized merge sort using temporary buffer with arg
extern utils_SortErr$void utils_mergeSortWithArgUsingTempRecur(
    Sli$u8   temp_buffer,
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs, anyptr_const arg),
    anyptr_const arg
) must_check;

#define utils_stableSort_threshold_merge_to_insertion 32

// Modernized stable sort (using merge sort with insertion sort)
extern utils_SortErr$void utils_stableSort(
    mem_Allocator allocator,
    meta_Sli      base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs)
) must_check;

// Modernized stable sort with arg (using merge sort with insertion sort)
extern utils_SortErr$void utils_stableSortWithArg(
    mem_Allocator allocator,
    meta_Sli      base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs, anyptr_const arg),
    anyptr_const arg
) must_check;

// Modernized stable sort (using merge sort with insertion sort)
extern utils_SortErr$void utils_stableSortUsingTemp(
    Sli$u8   temp_buffer,
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs)
) must_check;

// Modernized stable sort with arg (using merge sort with insertion sort)
extern utils_SortErr$void utils_stableSortWithArgUsingTemp(
    Sli$u8   temp_buffer,
    meta_Sli base_slice,
    cmp_Ord (*compareFn)(anyptr_const lhs, anyptr_const rhs, anyptr_const arg),
    anyptr_const arg
) must_check;

decl_ArrList$(Body);
use_ErrSet$(utils_SortErr, ArrList$Body);
extern utils_SortErr$ArrList$Body utils_uniformDisc(mem_Allocator allocator, usize n) must_check;

/*========== Implementations ================================================*/

force_inline utils_MulErr$usize utils_usize_mulSafe(usize lhs, usize rhs) {
    reserveReturn(utils_MulErr$usize);
    if (0 < lhs && usize_limit / lhs < rhs) {
        // Multiplication would overflow
        return_err(utils_MulErr_Overflow());
    }
    return_ok(lhs * rhs);
}

#endif /* UTILS_INCLUDED */
