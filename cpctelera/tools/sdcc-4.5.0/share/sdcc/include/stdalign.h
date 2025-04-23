#ifndef __STDC_VERSION_STDALIGN_H__
#define __STDC_VERSION_STDALIGN_H__ __STDC_VERSION__

#if __STDC_VERSION_STDBOOL_H__ < 202311L

#ifndef __alignas_is_defined
#define __alignas_is_defined 1

#define alignas _Alignas

#endif

#ifndef __alignof_is_defined
#define __alignof_is_defined 1

#define alignof _Alignof

#endif

#endif

#endif

