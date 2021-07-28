#ifndef __YADSL_BIGINT_H__
#define __YADSL_BIGINT_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/**
 * @brief Status code for Big Integer representation
 * correctness for debugging purposes.
*/
typedef enum
{
    YADSL_BIGINT_STATUS_OK, /**< Correct representation */
    YADSL_BIGINT_STATUS_INVALID_HANDLE, /**< Handle is invalid */
    YADSL_BIGINT_STATUS_INVALID_SIZE, /**< The 'size' field is invalid */
    YADSL_BIGINT_STATUS_INVALID_DIGITS, /**< The 'digits' field is invalid */
    YADSL_BIGINT_STATUS_LEADING_ZEROS, /**< There are excessive leading zeros */
}
yadsl_BigIntStatus;

/**
 * \defgroup bigint Big Integer
 * @brief Integer of arbitrary precision
 *
 * BigInts can be first created from fixed-size integers via the ::yadsl_bigint_from_int
 * function. Just like every other function in this module that returns a BigInt handle,
 * it returns NULL on memory allocation failure.
 *
 * Mathematical operations can be performed over pre-existing BigInts via functions
 * in this module like ::yadsl_bigint_add. All functions that take BigInt handles
 * expect valid BigInt handles (not NULL).
 *
 * BigInts can be converted back to fixed-size integers via the ::yadsl_bigint_to_int
 * function. If the integer is too large to fit in a fixed-size integer of type intmax_t,
 * then the function returns false.
 *
 * BigInts can also be converted to strings via the ::yadsl_bigint_to_string function.
 * If memory allocation occurrs, the function returns NULL.
 *
 * If compiled in Debug mode (with the macro YADSL_DEBUG defined), the module performs
 * internal checks to guarantee pre and postconditions established in this documentation.
 * @{
*/

typedef void yadsl_BigIntHandle; /**< Big Integer handle */

/**
 * @brief BigInt constructor from fixed-size integer
 * @param i fixed-size integer
 * @return new BigInt representation of i
*/
yadsl_BigIntHandle*
yadsl_bigint_from_int(
	intmax_t i);

/**
 * @brief Converts BigInt to a fixed-size integer
 * @param bigint BigInt to be converted
 * @param i_ptr fixed-size integer representation of bigint
 * @pre i_ptr points to a valid intmax_t variable
 * @return whether conversion was successful
 * @post If the function returns:
 * <ul>
 * <li>true, *i_ptr is updated</li>
 * <li>false, nothing is done</li>
 * </ul>
*/
bool
yadsl_bigint_to_int(
	yadsl_BigIntHandle const* bigint,
	intmax_t* i_ptr);

/**
 * @brief BigInt identity
 * @note Returns a new BigInt with the same value
 * @param a
 * @return a
*/
yadsl_BigIntHandle*
yadsl_bigint_copy(
	yadsl_BigIntHandle const* a);

/**
 * @brief BigInt additive inverse
 * @param a
 * @return -a
*/
yadsl_BigIntHandle*
yadsl_bigint_opposite(
	yadsl_BigIntHandle const* a);

/**
 * @brief BigInt addition
 * @param a
 * @param b
 * @return a + b
*/
yadsl_BigIntHandle*
yadsl_bigint_add(
	yadsl_BigIntHandle const* a,
	yadsl_BigIntHandle const* b);

/**
 * @brief BigInt subtraction
 * @param a
 * @param b
 * @return a - b
*/
yadsl_BigIntHandle*
yadsl_bigint_subtract(
	yadsl_BigIntHandle const* a,
	yadsl_BigIntHandle const* b);

/**
 * @brief BigInt multiplication
 * @param a
 * @param b
 * @return a * b
*/
yadsl_BigIntHandle*
yadsl_bigint_multiply(
	yadsl_BigIntHandle const* a,
	yadsl_BigIntHandle const* b);

/**
 * @brief BigInt division
 * @param a
 * @param b
 * @return a / b
*/
yadsl_BigIntHandle*
yadsl_bigint_divide(
	yadsl_BigIntHandle const* a,
	yadsl_BigIntHandle const* b);

/**
 * @brief BigInt comparison
 * @param a
 * @param b
 * @return
 * <ul>
 * <li>
 * -1, if a < b;
 * </li>
 * <li>
 * 1, if a > b;
 * </li>
 * <li>
 * 0, if a = b;
 * </li>
 * </ul>
*/
int
yadsl_bigint_compare(
	yadsl_BigIntHandle const* a,
	yadsl_BigIntHandle const* b);

/**
 * @brief Converts BigInt to a null-terminated
 *        ASCII string with characters 0-9
 * @param bigint BigInt to be converted
 * @return newly allocated string
*/
char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle const* bigint);

/**
 * @brief BigInt destructor
 * @param bigint BigInt to be destroyed
*/
void
yadsl_bigint_destroy(
	yadsl_BigIntHandle* bigint);

/**
 * @brief BigInt debug information dump
 * @param bigint BigInt to be dumped
 * @param fp file pointer for dumping
 * @return number of written characters
 * @pre fp is a valid file pointer
 * @post if returns:
 * <ul>
 * <li>
 * zero, does nothing
 * </li>
 * <li>
 * non-zero value, writes debug information to file
 * </li>
 * </ul>
*/
int
yadsl_bigint_dump(
    yadsl_BigIntHandle const* bigint,
    FILE* fp);

/**
 * @brief BigInt correctness check
 * @param bigint BigInt to be checked (can be NULL)
 * @return status code
*/
yadsl_BigIntStatus
yadsl_bigint_check(
        yadsl_BigIntHandle const* bigint);

/** @} */

#endif
