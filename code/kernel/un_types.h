/*
 * kernel/types.h
 *
 * This file defines the system data types.
 */

#ifndef TYPES_H
#define TYPES_H

/*
 * System basic types
 */

typedef signed char	s8;
typedef signed short	s16;
typedef signed int	s32;

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;


/*
 * System define types
 */

typedef u32		size_t;
typedef s32		ssize_t;

/* Stack type (32 bit) */
typedef u32		stk_t;
/* CPSR type (32 bit) */
typedef u32		cpsr_t;
/* Priority type (8 bit) */
typedef u8		prio_t;



#endif	/* TYPES_H */
