#ifndef EVG_H_
#define EVG_H_

#include <Eina.h>
#include <Eo.h>
#include <Efl.h>
#include <Evas.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVG_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EO_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @page evg_main Evg
 *
 * @date 2015 (created)
 *
 *
 * @addtogroup Evg
 * @{
 */

#ifdef EFL_BETA_API_SUPPORT

/**
 * @brief Init the evg subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see evg_shutfown()
 */
EAPI int evg_init(void);

/**
 * @brief Shutdown the evg subsystem
 * @return @c EINA_TRUE on success.
 *
 * @see evg_init()
 */
EAPI int evg_shutdown(void);

#include <efl_vg_loader.eo.h>
#include <efl_vg_loader.eo.legacy.h>

#endif

/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif
