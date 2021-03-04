#ifndef __YADSL_BIGINT_H__
#define __YADSL_BIGINT_H__

#include <stdbool.h>

/**
 * \defgroup bigint Big Integer
 * @brief Integer of arbitrary precision
 * If any of the functions that return yadsl_BigIntHandle*
 * return a null pointer, it means that it could not allocate
 * enough memory.
 * @{
*/

typedef void yadsl_BigIntHandle; /**< Big Integer handle */

/**
 * @brief Creates a big int
 * @return newly created bigint
*/
yadsl_BigIntHandle*
yadsl_bigint_from_int(
	int i);

/**
 * @brief Converts big int to C int
 * @param bigint bigint to be converted
 * @param i_ptr converted big int, if the function returns 0
 * @return whether the big integer couldn't fit into a C int
*/
bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle* bigint,
	int* i_ptr);

/**
 * @brief Add two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 + bigint2
*/
yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);
/**
 * @brief Subtract two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 - bigint2
*/
yadsl_BigIntHandle*
yadsl_bigint_subtract(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Multiply two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 * bigint2
*/
yadsl_BigIntHandle*
yadsl_bigint_multiply(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Divide two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 / bigint2
*/
yadsl_BigIntHandle*
yadsl_bigint_divide(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Compare two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 > bigint2
*/
bool
yadsl_bigint_compare_gt(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Compare two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 >= bigint2
*/
bool
yadsl_bigint_compare_ge(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Compare two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 == bigint2
*/
bool
yadsl_bigint_compare_eq(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Compare two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 < bigint2
*/
bool
yadsl_bigint_compare_le(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Compare two big integers
 * @param bigint1
 * @param bigint2
 * @return bigint1 < bigint2
*/
bool
yadsl_bigint_compare_lt(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Destroy a bigint
 * @param bigint bigint to be destroyed
*/
void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint);

/** @} */

#endif
