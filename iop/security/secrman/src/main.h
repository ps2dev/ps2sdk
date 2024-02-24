/**
  * _printf(): enables debug printf on secrman.c
  * _printf2(): enables debug printf on keyman.c
  * _printf3(): enables debug printf on MechaAuth.c
  * _printf(): enables debug printf on CardAuth.c
  */

#ifdef DEBUG
#ifdef BUILDING_SYSTEM2x6_SECRMAN
#define LOGMODNAME "ARCADE_SECRMAN: "
#else
#define LOGMODNAME "SECRMAN: "
#endif

#define _printf(args...) printf(LOGMODNAME args)
#define _printf2(args...) printf(LOGMODNAME args)
#define _printf3(args...) printf(LOGMODNAME args)
#define _printf4(args...) printf(LOGMODNAME args)
#define _arcade_printf(args...) printf(LOGMODNAME args)
#else
#define _printf(args...)
#define _printf2(args...)
#define _printf3(args...)
#define _printf4(args...)
#define _arcade_printf(args...)
#endif
