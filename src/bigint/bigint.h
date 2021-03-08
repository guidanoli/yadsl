#ifndef __YADSL_BIGINT_H__
#define __YADSL_BIGINT_H__

#include <stdbool.h>
#include <stdint.h>

/**
 * \defgroup bigint Big Integer
 * @brief Integer of arbitrary precision
 * If any of the functions that returns a pointer end up
 * returning NULL, it means that it could not allocate.
 * Every big integer is immutable to the user.
 * @{
*/

typedef void yadsl_BigIntHandle; /**< Big Integer handle */

/**
 * @brief Creates a big int
 * @return newly created bigint
*/
yadsl_BigIntHandle*
yadsl_bigint_from_int(
	intmax_t i);

/**
 * @brief Converts big int to C int
 * @param bigint bigint to be converted
 * @param i_ptr converted big int, if the function returns 0
 * @return whether the big integer couldn't fit into a C int
*/
bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle* bigint,
	intmax_t* i_ptr);

/**
 * @brief Copy bigint
 * @param bigint
 * @return bigint
*/
yadsl_BigIntHandle*
yadsl_bigint_copy(
	yadsl_BigIntHandle* bigint);

/**
 * @brief Multiplicates a big integer by -1
 * @param bigint
 * @return -bigint
*/
yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle* bigint);

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
 * @return -1 if bigint1 < bigint2
 *          1 if bigint1 > bigint2
 *          0 else
*/
int
yadsl_bigint_compare(
	yadsl_BigIntHandle* bigint1,
	yadsl_BigIntHandle* bigint2);

/**
 * @brief Converts big int to string
 * @param bigint
 * @return newly allocated string
*/
char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle* bigint);

/**
 * @brief Destroy a bigint
 * @param bigint bigint to be destroyed
*/
void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint);

/**
 * @brief Dump bigint information
 * @param bigint bigint to be dumped
*/
void
yadsl_bigint_dump(
    yadsl_BigIntHandle* bigint);


/** @} */

#endif
