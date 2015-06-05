#ifndef EVG_PRIVATE_H_
#define EVG_PRIVATE_H_

/*
 * variable and macros used for the eina_log module
 */
extern int _evg_log_dom_global;

/*
 * Macros that are used everywhere
 *
 * the first four macros are the general macros for the lib
 */
#ifdef EVG_DEFAULT_LOG_COLOR
# undef EVG_DEFAULT_LOG_COLOR
#endif /* ifdef EVG_DEFAULT_LOG_COLOR */
#define EVG_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_evg_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_evg_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_evg_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_evg_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_evg_log_dom_global, __VA_ARGS__)

static inline void
_eo_ref_replace(Eo **dest, const Eo *src)
{
   Eo *tmp = *dest;

   *dest = eo_ref(src);
   eo_unref(tmp);
}

#endif
