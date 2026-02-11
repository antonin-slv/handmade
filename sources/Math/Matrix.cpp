#include <immintrin.h>
#include <cstring>

template<typename T>
struct Matrix {
    int rows;
    int cols;
    T * data;

    // The memory will be managed elsewhere.
    Matrix() : rows(0), cols(0), data(nullptr) {}

    Matrix(int r, int c, T * d) : rows(r), cols(c), data(d) {}

    bool operator==(const Matrix& m) const {
        if (rows != m.rows || cols != m.cols)
            return false;

        for (int i = 0; i < rows * cols; ++i) {
            //si les données sont différentes, elles ne sont pas égales de toute façon
            if (data[i] != m.data[i])
                return false;
        }
        return true;
    }

    bool operator!=(const Matrix& m) const {
        return !(*this == m);
    }

    T& operator()(int r, int c) {
        return data[r * cols + c];
    }

    const T& operator()(int r, int c) const {
        return data[r * cols + c];
    }
};


/**
 * !!! Multiply two matrices a and b, storing the result in resultData.
 * 
    * @tparam T The data type of the matrix elements.
    * @param a The first matrix.
    * @param b The second matrix.
    * @param resultData Pointer to the memory where the result matrix will be stored (ALREADY ALLOCATED).
 * 
 */
template<typename T>
Matrix<T> multiply(const Matrix<T>& a, const Matrix<T>& b, T* resultData) {
    if (a.cols != b.rows) {
        return Matrix<T>(0, 0, nullptr); // Return an empty matrix on dimension mismatch
    }

    Matrix<T> result(a.rows, b.cols, resultData);
    
    std::memset(result.data, T(0), result.rows * result.cols * sizeof(T)); // Zero out the result matrix

    for (int i = 0; i < a.rows; ++i) {
        for (int k = 0; k < a.cols; ++k) {
            T val = a(i, k);
            for (int j = 0; j < b.cols; ++j) {
                result(i, j) += val * b(k, j);
            }
        }
    }

    return result;
};

// SIMD Specialization for float
template<>
inline Matrix<float> multiply(const Matrix<float>& a, const Matrix<float>& b, float* resultData) {
    if (a.cols != b.rows) {
        return Matrix<float>(0, 0, nullptr);
    }

    Matrix<float> result(a.rows, b.cols, resultData);
    
    // Efficiently zero out memory
    std::memset(result.data, 0, result.rows * result.cols * sizeof(float));
    
    for (int i = 0; i < a.rows; ++i) {
        for (int k = 0; k < a.cols; ++k) {
            float val = a(i, k);
            __m128 val_vec = _mm_set1_ps(val);

            int j = 0;
            // Unroll loop for 4 floats at a time using SSE
            for (; j <= b.cols - 4; j += 4) {
                __m128 res_vec = _mm_loadu_ps(&result(i, j)); // Load unaligned
                __m128 b_vec = _mm_loadu_ps(&b(k, j));        // Load unaligned
                __m128 prod = _mm_mul_ps(val_vec, b_vec);
                res_vec = _mm_add_ps(res_vec, prod);
                _mm_storeu_ps(&result(i, j), res_vec);        // Store unaligned
            }

            // Handle leftovers
            for (; j < b.cols; ++j) {
                result(i, j) += val * b(k, j);
            }
        }
    }
    return result;
}