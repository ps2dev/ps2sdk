/**
  * _printf(): enables debug printf on secrman.c
  * _printf2(): enables debug printf on keyman.c
  * _printf3(): enables debug printf on MechaAuth.c
  * _printf(): enables debug printf on CardAuth.c
  */

#ifdef DEBUG
#define _printf(args...) printf("SECRMAN: "args)
#define _printf2(args...) printf("SECRMAN: "args)
#define _printf3(args...) printf("SECRMAN: "args)
#define _printf4(args...) printf("SECRMAN: "args)
#else
#define _printf(args...)
#define _printf2(args...)
#define _printf3(args...)
#define _printf4(args...)
#endif
