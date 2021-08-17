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
    YADSL_BIGINT_STATUS_INTEGER_OVERFLOW, /**< BigInt doesn't fit in fixed-size integer */
    YADSL_BIGINT_STATUS_DIGIT_OVERFLOW, /**< The 'size' field is larger than the 'ndigits' field */
    YADSL_BIGINT_STATUS_STRING_FORMAT, /**< String does not follow format */
    YADSL_BIGINT_STATUS_MEMORY, /**< Could not allocate enough memory space */
}
yadsl_BigIntStatus;

/**
 * \defgroup bigint Big Integer
 * @brief Integer of arbitrary precision
 *
 * BigInts can be created from fixed-size integers via the ::yadsl_bigint_from_int
 * function or from strings via the ::yadsl_bigint_from_string function.
 * Just like every function in this module that returns a BigInt handle,
 * they return NULL on memory allocation failure.
 *
 * The string format returned by ::yadsl_bigint_to_string consists of a non-empty
 * sequence of numbers with a leading minus sign for negative numbers and no extra leading zero.
 * The regular expression that matches this format would be `-?(0|[1-9][0-9]*)`.
 *
 * The string format accepted by ::yadsl_bigint_from_string is a little bit more flexible,
 * as it accepts an optional plus sign for positive numbers and leading zeros.
 * The regular expression that matches this format would be `[-+]?[0-9]+`.
 *
 * Mathematical operations can be performed over pre-existing BigInts via functions
 * in this module like ::yadsl_bigint_add. All functions that take BigInt handles
 * expect valid BigInt handles (not NULL).
 *
 * BigInts can be converted back to fixed-size integers via the ::yadsl_bigint_to_int
 * function. If the integer is too large to fit in a fixed-size integer of type intmax_t,
 * then the function returns false. BigInts can also be converted to strings via the
 * ::yadsl_bigint_to_string function.
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
 * <li>::YADSL_BIGINT_STATUS_OK, *i_ptr is updated</li>
 * <li>::YADSL_BIGINT_STATUS_INTEGER_OVERFLOW, nothing is done</li>
 * </ul>
*/
yadsl_BigIntStatus
yadsl_bigint_to_int(
	yadsl_BigIntHandle const* bigint,
	intmax_t* i_ptr);

/**
 * @brief BigInt constructor from string
 * @param str string with number in decimal base
 * @param bigint_ptr pointer to bigint to be returned
 * @return whether conversion was successful
 * @pre str points to a valid null-terminated string
 * @pre bigint_ptr points to a valid BigInt handle
 * @post If the function returns:
 * <ul>
 * <li>::YADSL_BIGINT_STATUS_OK, *bigint_ptr is updated</li>
 * <li>::YADSL_BIGINT_STATUS_STRING_FORMAT, nothing is done</li>
 * </ul>
*/
yadsl_BigIntStatus
yadsl_bigint_from_string(
    const char* str,
    yadsl_BigIntHandle** bigint_ptr);

/**
 * @brief Converts BigInt to a string
 * @param bigint BigInt to be converted
 * @return newly allocated string
*/
char*
yadsl_bigint_to_string(
	yadsl_BigIntHandle const* bigint);

/**
 * @brief Optimize space usage of BigInt
 * @param bigint BigInt to be optimized
 * @return new handle for optimized BigInt
 * or given handle if could not optimize BigInt
*/
yadsl_BigIntHandle*
yadsl_bigint_optimize(
    yadsl_BigIntHandle* bigint);

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
