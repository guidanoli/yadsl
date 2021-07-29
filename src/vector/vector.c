#include <vector/vector.h>

#include <stdint.h>
#include <assert.h>

#include <yadsl/stdlib.h>

typedef struct
{
	size_t size;
	size_t dtype_size;
	uint8_t data[1];
}
yadsl_Vector;

yadsl_VectorHandle* yadsl_vector_create(size_t dtype_size, size_t size)
{
	yadsl_Vector* vector = malloc(sizeof(*vector) + dtype_size * size * sizeof(uint8_t));
	if (vector) {
		vector->dtype_size = dtype_size;
		vector->size = size;
	}
	return vector;
}

size_t yadsl_vector_size(yadsl_VectorHandle* vector)
{
	return ((yadsl_Vector*) vector)->size;
}

void* yadsl_vector_at(yadsl_VectorHandle* vector, size_t index)
{
	assert(((yadsl_Vector*) vector)->size > index);
	return ((yadsl_Vector*) vector)->data + (index * ((yadsl_Vector*) vector)->dtype_size);
}

bool
yadsl_vector_resize(
	yadsl_VectorHandle** vector_ptr,
	size_t new_size)
{
	yadsl_Vector* new_vector = (yadsl_Vector*) *vector_ptr;
	
	new_vector = realloc(new_vector, sizeof(*new_vector) + new_vector->dtype_size * new_size * sizeof(uint8_t));

	if (new_vector) {
		new_vector->size = new_size;
		*vector_ptr = new_vector;
		return true;
	} else {
		return false;
	}
}

void yadsl_vector_destroy(yadsl_VectorHandle* vector)
{
	free(vector);
}
