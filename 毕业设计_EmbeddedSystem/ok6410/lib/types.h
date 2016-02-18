#ifndef _TYPES_H_
#define _TYPES_H_

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef unsigned long long	u64;
typedef signed char		s8;
typedef short			s16;
typedef int			s32;
typedef long long		s64;



typedef u32		size_t;
typedef s32		ssize_t;

/* Stack type (32 bit) */
typedef u32		stk_t;
/* CPSR type (32 bit) */
typedef u32		cpsr_t;
/* Priority type (8 bit) */
typedef u8		prio_t;



#define min(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x < _y ? _x : _y; })

#define max(x,y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);	\
	_x > _y ? _x : _y; })

#endif /* _TYPES_H_ */
