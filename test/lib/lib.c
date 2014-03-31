/*
 *
 */

#include "types.h"

/**
 * memset() - Fill a region of memory with the given value
 * @s: Pointer to the start of the area
 * @c: he byte to fill the area with
 * @n: The size of the area
 *
 * The memset() function fills the first n bytes of the memory area
 * pointed to by s with the constant byte c.
 *
 * Return: The memset() function returns a pointer to the memory area s
 */
void *
memset(void *s, int c, size_t n)
{
	char *xs = s;

	while (n--)
		*xs++ = c;
	return s;
}
