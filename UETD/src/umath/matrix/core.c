/// @file src/umath/matrix/core.c
/// @brief Caller-workspace row-major dense matrix kernels.
#include <math.h>
#include <float.h>
#include <stdint.h>
#include <umath/matrix/core.h>
#include <umath/scalar.h>

static bool valid_shape(size_t rows, size_t columns) {
    size_t elements;
    if (rows == 0U || columns == 0U || rows > SIZE_MAX / columns) return false;
    elements = rows * columns;
    return elements <= SIZE_MAX / sizeof(umath_scalar_t);
}

static umath_status_t matrix_workspace_count(size_t dimension, size_t matrix_count, size_t* workspace_count) {
    size_t elements;
    if (workspace_count == NULL || dimension == 0U || matrix_count == 0U || dimension > SIZE_MAX / dimension) {
        return UMATH_INVALID_ARGUMENT;
    }
    elements = dimension * dimension;
    if (elements > SIZE_MAX / sizeof(umath_scalar_t)) return UMATH_INVALID_ARGUMENT;
    if (elements > SIZE_MAX / matrix_count) return UMATH_INVALID_ARGUMENT;
    elements *= matrix_count;
    if (elements > SIZE_MAX / sizeof(umath_accumulator_t)) return UMATH_INVALID_ARGUMENT;
    *workspace_count = elements;
    return UMATH_OK;
}

umath_status_t umath_matrix_vector(const umath_scalar_t* matrix, const umath_scalar_t* vector, size_t rows, size_t columns,
                                   umath_scalar_t* output) {
    size_t row, column;
    if (matrix == NULL || vector == NULL || output == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < rows; ++row) {
        umath_accumulator_t sum = UMATH_ACCUMULATOR_C(0);
        for (column = 0U; column < columns; ++column) {
            const umath_accumulator_t a = (umath_accumulator_t)matrix[row * columns + column];
            const umath_accumulator_t b = (umath_accumulator_t)vector[column];
            if (!umath_accumulator_is_finite(a) || !umath_accumulator_is_finite(b)) return UMATH_NUMERIC_FAILURE;
            sum += a * b;
        }
        if (!umath_accumulator_is_finite(sum) || !umath_scalar_is_finite((umath_scalar_t)sum)) { return UMATH_NUMERIC_FAILURE; }
        output[row] = (umath_scalar_t)sum;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_multiply(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t inner,
                                     size_t columns, umath_scalar_t* output) {
    size_t row, column, k;
    if (left == NULL || right == NULL || output == NULL || !valid_shape(rows, inner) || !valid_shape(inner, columns) ||
        !valid_shape(rows, columns))
        return UMATH_INVALID_ARGUMENT;
    for (row = 0U; row < rows; ++row) {
        for (column = 0U; column < columns; ++column) {
            umath_accumulator_t sum = UMATH_ACCUMULATOR_C(0);
            for (k = 0U; k < inner; ++k) {
                const umath_accumulator_t a = (umath_accumulator_t)left[row * inner + k];
                const umath_accumulator_t b = (umath_accumulator_t)right[k * columns + column];
                if (!umath_accumulator_is_finite(a) || !umath_accumulator_is_finite(b)) return UMATH_NUMERIC_FAILURE;
                sum += a * b;
            }
            if (!umath_accumulator_is_finite(sum) || !umath_scalar_is_finite((umath_scalar_t)sum)) {
                return UMATH_NUMERIC_FAILURE;
            }
            output[row * columns + column] = (umath_scalar_t)sum;
        }
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_transpose(const umath_scalar_t* input, size_t rows, size_t columns, umath_scalar_t* output) {
    size_t row, column;
    if (input == NULL || output == NULL || !valid_shape(rows, columns)) { return UMATH_INVALID_ARGUMENT; }
    for (row = 0U; row < rows; ++row) {
        for (column = 0U; column < columns; ++column) {
            const umath_scalar_t value = input[row * columns + column];
            if (!umath_scalar_is_finite(value)) return UMATH_NUMERIC_FAILURE;
            output[column * rows + row] = value;
        }
    }
    return UMATH_OK;
}

static umath_status_t matrix_binary(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                    umath_scalar_t* output, bool subtract) {
    size_t index, count;
    if (left == NULL || right == NULL || output == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    for (index = 0U; index < count; ++index) {
        const umath_accumulator_t a = (umath_accumulator_t)left[index];
        const umath_accumulator_t b = (umath_accumulator_t)right[index];
        const umath_accumulator_t result = subtract ? a - b : a + b;
        if (!umath_accumulator_is_finite(a) || !umath_accumulator_is_finite(b) || !umath_accumulator_is_finite(result) ||
            !umath_scalar_is_finite((umath_scalar_t)result)) { return UMATH_NUMERIC_FAILURE; }
        output[index] = (umath_scalar_t)result;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_add(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                umath_scalar_t* output) {
    return matrix_binary(left, right, rows, columns, output, false);
}

umath_status_t umath_matrix_sub(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns,
                                umath_scalar_t* output) {
    return matrix_binary(left, right, rows, columns, output, true);
}

umath_status_t umath_matrix_scale(const umath_scalar_t* input, size_t rows, size_t columns, umath_scalar_t scale,
                                  umath_scalar_t* output) {
    size_t index, count;
    if (input == NULL || output == NULL || !valid_shape(rows, columns) || !umath_scalar_is_finite(scale))
        return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    for (index = 0U; index < count; ++index) {
        const umath_accumulator_t value = (umath_accumulator_t)input[index] * (umath_accumulator_t)scale;
        if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite((umath_scalar_t)value)) {
            return UMATH_NUMERIC_FAILURE;
        }
        output[index] = (umath_scalar_t)value;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_symmetrize(umath_scalar_t* matrix, size_t dimension) {
    size_t row, column;
    if (matrix == NULL || !valid_shape(dimension, dimension)) { return UMATH_INVALID_ARGUMENT; }
    for (row = 0U; row < dimension; ++row) {
        for (column = row + 1U; column < dimension; ++column) {
            const umath_accumulator_t a = (umath_accumulator_t)matrix[row * dimension + column];
            const umath_accumulator_t b = (umath_accumulator_t)matrix[column * dimension + row];
            const umath_accumulator_t average = UMATH_ACCUMULATOR_C(0.5) * (a + b);
            if (!umath_accumulator_is_finite(a) || !umath_accumulator_is_finite(b) || !umath_accumulator_is_finite(average) ||
                !umath_scalar_is_finite((umath_scalar_t)average)) { return UMATH_NUMERIC_FAILURE; }
            matrix[row * dimension + column] = (umath_scalar_t)average;
            matrix[column * dimension + row] = (umath_scalar_t)average;
        }
    }
    return UMATH_OK;
}

static bool matrix_finite(const umath_scalar_t* matrix, size_t count) {
    size_t i;
    for (i = 0; i < count; ++i) if (!umath_scalar_is_finite(matrix[i])) return false;
    return true;
}

static umath_status_t matrix_store(umath_accumulator_t value, umath_scalar_t* output) {
    const umath_scalar_t converted = (umath_scalar_t)value;
    if (!umath_accumulator_is_finite(value) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *output = converted;
    return UMATH_OK;
}

umath_status_t umath_matrix_identity(size_t dimension, umath_scalar_t* output) {
    size_t row, column;
    if (output == NULL || !valid_shape(dimension, dimension)) return UMATH_INVALID_ARGUMENT;
    for (row = 0; row < dimension; ++row)
        for (column = 0; column < dimension; ++column) output[row*dimension+column] = row == column ? (umath_scalar_t)1 : (umath_scalar_t)0;
    return UMATH_OK;
}

umath_status_t umath_matrix_diagonal(const umath_scalar_t* diagonal, size_t dimension, umath_scalar_t* output) {
    size_t row, column;
    if (diagonal == NULL || output == NULL || !valid_shape(dimension, dimension)) return UMATH_INVALID_ARGUMENT;
    for (row = 0; row < dimension; ++row) if (!umath_scalar_is_finite(diagonal[row])) return UMATH_NUMERIC_FAILURE;
    for (row = 0; row < dimension; ++row)
        for (column = 0; column < dimension; ++column) output[row*dimension+column] = row == column ? diagonal[row] : (umath_scalar_t)0;
    return UMATH_OK;
}

umath_status_t umath_matrix_elementwise_mul(const umath_scalar_t* left, const umath_scalar_t* right, size_t rows, size_t columns, umath_scalar_t* output) {
    size_t i, count;
    if (left == NULL || right == NULL || output == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    if (!matrix_finite(left, count) || !matrix_finite(right, count)) return UMATH_NUMERIC_FAILURE;
    for (i = 0; i < count; ++i) {
        const umath_status_t status = matrix_store((umath_accumulator_t)left[i] * (umath_accumulator_t)right[i], &output[i]);
        if (status != UMATH_OK) return status;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_trace(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* trace) {
    size_t i; umath_accumulator_t sum = 0;
    if (matrix == NULL || trace == NULL || !valid_shape(dimension, dimension)) return UMATH_INVALID_ARGUMENT;
    for (i = 0; i < dimension; ++i) {
        if (!umath_scalar_is_finite(matrix[i*dimension+i])) return UMATH_NUMERIC_FAILURE;
        sum += (umath_accumulator_t)matrix[i*dimension+i];
    }
    return matrix_store(sum, trace);
}

umath_status_t umath_matrix_norm_frobenius(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm) {
    size_t i, count; umath_accumulator_t result = 0; umath_scalar_t converted;
    if (matrix == NULL || norm == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    count = rows * columns;
    for (i = 0; i < count; ++i) {
        if (!umath_scalar_is_finite(matrix[i])) return UMATH_NUMERIC_FAILURE;
        result = UETD_UMATH_ACCUM_BINARY_MATH(hypot, result, (umath_accumulator_t)matrix[i]);
    }
    converted = (umath_scalar_t)result;
    if (!umath_accumulator_is_finite(result) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *norm = converted;
    return UMATH_OK;
}

umath_status_t umath_matrix_norm_one(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm) {
    size_t row, column; umath_accumulator_t maximum = 0; umath_scalar_t converted;
    if (matrix == NULL || norm == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    if (!matrix_finite(matrix, rows*columns)) return UMATH_NUMERIC_FAILURE;
    for (column = 0; column < columns; ++column) {
        umath_accumulator_t sum = 0;
        for (row = 0; row < rows; ++row) sum += UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)matrix[row*columns+column]);
        if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
        if (sum > maximum) maximum = sum;
    }
    converted = (umath_scalar_t)maximum;
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *norm = converted;
    return UMATH_OK;
}

umath_status_t umath_matrix_norm_infinity(const umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t* norm) {
    size_t row, column; umath_accumulator_t maximum = 0; umath_scalar_t converted;
    if (matrix == NULL || norm == NULL || !valid_shape(rows, columns)) return UMATH_INVALID_ARGUMENT;
    if (!matrix_finite(matrix, rows*columns)) return UMATH_NUMERIC_FAILURE;
    for (row = 0; row < rows; ++row) {
        umath_accumulator_t sum = 0;
        for (column = 0; column < columns; ++column) sum += UETD_UMATH_ACCUM_UNARY_MATH(fabs, (umath_accumulator_t)matrix[row*columns+column]);
        if (!umath_accumulator_is_finite(sum)) return UMATH_NUMERIC_FAILURE;
        if (sum > maximum) maximum = sum;
    }
    converted = (umath_scalar_t)maximum;
    if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    *norm = converted;
    return UMATH_OK;
}

umath_status_t umath_matrix_determinant_workspace_count(size_t dimension, size_t* workspace_count) {
    return matrix_workspace_count(dimension, 1U, workspace_count);
}

umath_status_t umath_matrix_determinant(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* determinant,
                                        umath_accumulator_t* workspace, size_t workspace_count) {
    umath_accumulator_t value = UMATH_ACCUMULATOR_C(1);
    size_t row, column, pivot, elements, required;
    if (matrix == NULL || determinant == NULL || workspace == NULL ||
        umath_matrix_determinant_workspace_count(dimension, &required) != UMATH_OK || workspace_count < required)
        return UMATH_INVALID_ARGUMENT;
    elements = dimension * dimension;
    if (!matrix_finite(matrix, elements)) return UMATH_NUMERIC_FAILURE;
    for (row = 0; row < elements; ++row) workspace[row] = (umath_accumulator_t)matrix[row];
    for (pivot = 0; pivot < dimension; ++pivot) {
        size_t best = pivot;
        for (row = pivot+1; row < dimension; ++row)
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs, workspace[row*dimension+pivot]) > UETD_UMATH_ACCUM_UNARY_MATH(fabs, workspace[best*dimension+pivot])) best = row;
        if (workspace[best*dimension+pivot] == 0) { *determinant = UMATH_SCALAR_C(0); return UMATH_OK; }
        if (best != pivot) {
            for (column = pivot; column < dimension; ++column) {
                const umath_accumulator_t temp = workspace[pivot*dimension+column];
                workspace[pivot*dimension+column] = workspace[best*dimension+column]; workspace[best*dimension+column] = temp;
            }
            value = -value;
        }
        value *= workspace[pivot*dimension+pivot];
        if (!umath_accumulator_is_finite(value)) return UMATH_NUMERIC_FAILURE;
        for (row = pivot+1; row < dimension; ++row) {
            const umath_accumulator_t factor = workspace[row*dimension+pivot] / workspace[pivot*dimension+pivot];
            for (column = pivot+1; column < dimension; ++column) {
                workspace[row*dimension+column] -= factor*workspace[pivot*dimension+column];
                if (!umath_accumulator_is_finite(workspace[row*dimension+column])) return UMATH_NUMERIC_FAILURE;
            }
        }
    }
    return matrix_store(value, determinant);
}

umath_status_t umath_matrix_inverse_workspace_count(size_t dimension, size_t* workspace_count) {
    return matrix_workspace_count(dimension, 2U, workspace_count);
}

umath_status_t umath_matrix_inverse(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* inverse,
                                    umath_accumulator_t* workspace, size_t workspace_count) {
    umath_accumulator_t* left = workspace;
    umath_accumulator_t* right;
    size_t row, column, pivot, elements, required;
    if (matrix==NULL || inverse==NULL || workspace==NULL ||
        umath_matrix_inverse_workspace_count(dimension, &required) != UMATH_OK || workspace_count < required)
        return UMATH_INVALID_ARGUMENT;
    elements = dimension * dimension;
    right = workspace + elements;
    if (!matrix_finite(matrix,elements)) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column) {
        left[row*dimension+column]=matrix[row*dimension+column]; right[row*dimension+column]=row==column ? 1 : 0;
    }
    for (pivot=0; pivot<dimension; ++pivot) {
        size_t best=pivot; umath_accumulator_t divisor;
        for (row=pivot+1; row<dimension; ++row)
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs,left[row*dimension+pivot])>UETD_UMATH_ACCUM_UNARY_MATH(fabs,left[best*dimension+pivot])) best=row;
        if (left[best*dimension+pivot]==0) return UMATH_SINGULAR;
        if (best!=pivot) for (column=0; column<dimension; ++column) {
            umath_accumulator_t temp=left[pivot*dimension+column]; left[pivot*dimension+column]=left[best*dimension+column]; left[best*dimension+column]=temp;
            temp=right[pivot*dimension+column]; right[pivot*dimension+column]=right[best*dimension+column]; right[best*dimension+column]=temp;
        }
        divisor=left[pivot*dimension+pivot];
        for (column=0; column<dimension; ++column) {
            left[pivot*dimension+column]/=divisor; right[pivot*dimension+column]/=divisor;
            if (!umath_accumulator_is_finite(left[pivot*dimension+column]) || !umath_accumulator_is_finite(right[pivot*dimension+column])) return UMATH_NUMERIC_FAILURE;
        }
        for (row=0; row<dimension; ++row) if (row!=pivot) {
            const umath_accumulator_t factor=left[row*dimension+pivot];
            for (column=0; column<dimension; ++column) {
                left[row*dimension+column]-=factor*left[pivot*dimension+column];
                right[row*dimension+column]-=factor*right[pivot*dimension+column];
                if (!umath_accumulator_is_finite(left[row*dimension+column]) || !umath_accumulator_is_finite(right[row*dimension+column])) return UMATH_NUMERIC_FAILURE;
            }
        }
    }
    for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column)
        if (!umath_accumulator_is_finite(right[row*dimension+column]) || !umath_scalar_is_finite((umath_scalar_t)right[row*dimension+column])) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<dimension*dimension; ++row) inverse[row]=(umath_scalar_t)right[row];
    return UMATH_OK;
}

umath_status_t umath_matrix_outer_product(const umath_scalar_t* left, size_t rows, const umath_scalar_t* right, size_t columns, umath_scalar_t* output) {
    size_t row,column;
    if (left==NULL || right==NULL || output==NULL || rows==0 || columns==0 || !valid_shape(rows,columns)) return UMATH_INVALID_ARGUMENT;
    for (row=0; row<rows; ++row) if (!umath_scalar_is_finite(left[row])) return UMATH_NUMERIC_FAILURE;
    for (column=0; column<columns; ++column) if (!umath_scalar_is_finite(right[column])) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<rows; ++row) for (column=0; column<columns; ++column) {
        const umath_status_t status=matrix_store((umath_accumulator_t)left[row]*(umath_accumulator_t)right[column],&output[row*columns+column]);
        if (status!=UMATH_OK) return status;
    }
    return UMATH_OK;
}

umath_status_t umath_matrix_rank_one_update(umath_scalar_t* matrix, size_t rows, size_t columns, umath_scalar_t alpha, const umath_scalar_t* left, const umath_scalar_t* right) {
    size_t row,column;
    if (matrix==NULL || left==NULL || right==NULL || !umath_scalar_is_finite(alpha) || !valid_shape(rows,columns)) return UMATH_INVALID_ARGUMENT;
    if (!matrix_finite(matrix,rows*columns)) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<rows; ++row) if (!umath_scalar_is_finite(left[row])) return UMATH_NUMERIC_FAILURE;
    for (column=0; column<columns; ++column) if (!umath_scalar_is_finite(right[column])) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<rows; ++row) for (column=0; column<columns; ++column) {
        const umath_accumulator_t updated=(umath_accumulator_t)matrix[row*columns+column]+(umath_accumulator_t)alpha*left[row]*right[column];
        const umath_scalar_t converted=(umath_scalar_t)updated;
        if (!umath_accumulator_is_finite(updated) || !umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    }
    for (row=0; row<rows; ++row) for (column=0; column<columns; ++column)
        matrix[row*columns+column]=(umath_scalar_t)((umath_accumulator_t)matrix[row*columns+column]+(umath_accumulator_t)alpha*left[row]*right[column]);
    return UMATH_OK;
}

umath_status_t umath_matrix_exponential_workspace_count(size_t dimension, size_t* workspace_count) {
    return matrix_workspace_count(dimension, 3U, workspace_count);
}

umath_status_t umath_matrix_exponential(const umath_scalar_t* matrix, size_t dimension, umath_scalar_t* exponential,
                                        umath_accumulator_t* workspace, size_t workspace_count) {
    umath_accumulator_t *sum, *term, *scratch, *next, *swap, norm=0, scaled_norm, epsilon;
    size_t row,column,k,steps=0,count,required; bool converged=false;
    if (matrix==NULL || exponential==NULL || workspace==NULL ||
        umath_matrix_exponential_workspace_count(dimension, &required) != UMATH_OK || workspace_count < required)
        return UMATH_INVALID_ARGUMENT;
    count=dimension*dimension;
    sum=workspace; term=workspace+count; scratch=workspace+(2U*count); next=scratch;
    if (!matrix_finite(matrix,count)) return UMATH_NUMERIC_FAILURE;
    for (column=0; column<dimension; ++column) {
        umath_accumulator_t colsum=0;
        for (row=0; row<dimension; ++row) colsum+=UETD_UMATH_ACCUM_UNARY_MATH(fabs,(umath_accumulator_t)matrix[row*dimension+column]);
        if (colsum>norm) norm=colsum;
    }
    if (!umath_accumulator_is_finite(norm)) return UMATH_NUMERIC_FAILURE;
    scaled_norm=norm;
    while (scaled_norm>UMATH_ACCUMULATOR_C(0.5) && steps<2048U) { scaled_norm*=UMATH_ACCUMULATOR_C(0.5); ++steps; }
    if (scaled_norm>UMATH_ACCUMULATOR_C(0.5)) return UMATH_NUMERIC_FAILURE;
    for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column) {
        const size_t index=row*dimension+column;
        sum[index]=row==column ? UMATH_ACCUMULATOR_C(1) : UMATH_ACCUMULATOR_C(0);
        term[index]=sum[index];
    }
    epsilon=sizeof(umath_accumulator_t)==sizeof(float) ? (umath_accumulator_t)FLT_EPSILON : (umath_accumulator_t)DBL_EPSILON;
    for (k=1; k<=64U; ++k) {
        umath_accumulator_t max_term=0,max_sum=0;
        for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column) {
            umath_accumulator_t product=0; size_t inner;
            for (inner=0; inner<dimension; ++inner)
                product+=term[row*dimension+inner]*(umath_accumulator_t)matrix[inner*dimension+column];
            next[row*dimension+column]=UETD_UMATH_ACCUM_BINARY_MATH(ldexp,product,-(int)steps)/(umath_accumulator_t)k;
        }
        for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column) {
            const size_t index=row*dimension+column;
            term[index]=next[index]; sum[index]+=term[index];
            if (!umath_accumulator_is_finite(sum[index])) return UMATH_NUMERIC_FAILURE;
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs,term[index])>max_term) max_term=UETD_UMATH_ACCUM_UNARY_MATH(fabs,term[index]);
            if (UETD_UMATH_ACCUM_UNARY_MATH(fabs,sum[index])>max_sum) max_sum=UETD_UMATH_ACCUM_UNARY_MATH(fabs,sum[index]);
        }
        swap=term; term=next; next=swap;
        if (max_term<=epsilon*(max_sum>1 ? max_sum : UMATH_ACCUMULATOR_C(1))) { converged=true; break; }
    }
    if (!converged) return UMATH_NO_CONVERGENCE;
    while (steps>0) {
        --steps;
        for (row=0; row<dimension; ++row) for (column=0; column<dimension; ++column) {
            umath_accumulator_t product=0; size_t inner;
            for (inner=0; inner<dimension; ++inner) product+=sum[row*dimension+inner]*sum[inner*dimension+column];
            scratch[row*dimension+column]=product;
            if (!umath_accumulator_is_finite(product)) return UMATH_NUMERIC_FAILURE;
        }
        for (row=0; row<count; ++row) sum[row]=scratch[row];
    }
    for (row=0; row<count; ++row) {
        const umath_scalar_t converted=(umath_scalar_t)sum[row];
        if (!umath_scalar_is_finite(converted)) return UMATH_NUMERIC_FAILURE;
    }
    for (row=0; row<count; ++row) exponential[row]=(umath_scalar_t)sum[row];
    return UMATH_OK;
}
