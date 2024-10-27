#ifndef DEBUG_CFG_INCLUDED
#define DEBUG_CFG_INCLUDED (1)
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */


#if !defined(NDEBUG)
#  define DEBUG_ENABLED (1)
#else
#  define DEBUG_ENABLED (0)
#endif /* !defined(NDEBUG) */


#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */
#endif /* DEBUG_CFG_INCLUDED */
