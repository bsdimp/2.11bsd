/*
 * Public domain - 2002/1/30 - sms
 *
 *	@(#)stdint.h	1.0 (2.11BSD) 2002/1/30
*/

#ifndef	_STDINT_H_
#define	_STDINT_H_

/* Exact-size types  */
typedef char                  int8_t;
typedef unsigned char         uint8_t;
typedef int                   int16_t;
typedef unsigned int          uint16_t;
typedef long                  int32_t;
typedef unsigned long         uint32_t;
#ifdef notyet
typedef long long             int64_t;
typedef unsigned long long    uint64_t;
#endif

/* BSD compat types */
typedef uint8_t                       u_int8_t;
typedef uint16_t                      u_int16_t;
typedef uint32_t                      u_int32_t;
#ifdef notyet
typedef uint64_t                      u_int64_t;
#endif
#endif /* _STDINT_H_ */
