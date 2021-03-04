#ifndef __YADSL_VECTOR_H__
#define __YADSL_VECTOR_H__

#include <stddef.h>
#include <stdbool.h>

/**
 * \defgroup vector Vector
 * @brief Resizable vector
 * @{
*/

typedef void yadsl_VectorHandle; /**< Vector handle */

/**
 * @brief Creates a vector
 * @param dtype data type size (`sizeof(int)`, for example)
 * @param size vector initial size (number of elements)
 * @return newly created vector or NULL if could not allocate memory
*/
yadsl_VectorHandle*
yadsl_vector_create(
	size_t dtype,
	size_t size);

/**
 * @brief Get vector element count
 * @param vector
 * @return element count in vector
*/
size_t
yadsl_vector_size(
	yadsl_VectorHandle* vector);

/**
 * @brief Get pointer at a given index
 * @param vector
 * @param index
 * @return pointer to data at index
*/
void*
yadsl_vector_at(
	yadsl_VectorHandle* vector,
	size_t index);

/**
 * @brief Resize vector
 * @param vector_ptr pointer to vector
 * @param new_size new element count
 * @return whether resize was made. If true, *vector_ptr is updated.
*/
bool
yadsl_vector_resize(
	yadsl_VectorHandle** vector_ptr,
	size_t new_size);

/**
 * @brief Destroy a vector
 * @param vector vector to be destroyed
*/
void
yadsl_vector_destroy(
	yadsl_VectorHandle* vector);

/** @} */

#endif
