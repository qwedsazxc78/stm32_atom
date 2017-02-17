#include "ist_math.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"

void SmoothFliter(float new_value, float *pre_value, float *pre_value2, float theta)
{
    *pre_value = new_value * theta + *pre_value * (1 - theta);
    *pre_value2 = *pre_value * theta + *pre_value2 * (1 - theta);
}

void DotMatrix_A_B_to_C(float *C, float *A, int nrows, int ncols, float *B, int mcols)
{
    float *pB = NULL, *pB1 = NULL;
    int i = 0, j = 0, k = 0;

    for (i = 0; i < nrows; A += ncols, i++) {
        for (pB1 = B, j = 0; j < mcols; C++, pB1++, j++) {
            pB = pB1;
            *C = 0;
            for (k = 0; k < ncols; pB += mcols, k++) {
                *C = (*C + (*(A + k) * *pB));
            }
        }
    }
}

// function uses Gauss-Jordan elimination to compute the inverse of matrix A
// in situon exit, A is replaced with its inverse
ISTBOOL InvertMatrix4by4(const float B[], float C[])
{
    float largest;                  // largest element used for pivoting
    float scaling;                  // scaling factor in pivoting
    float recippiv;                 // reciprocal of pivot element
    float ftmp;                     // temporary variable used in swaps
    int i, j, k, l, m;              // index counters
    int iPivotRow, iPivotCol;       // row and column of pivot element
    int iPivot[4] = {0};
    int isize = 4;
    int iRowInd[4] = {0};
    int iColInd[4] = {0};
    float A[4][4] = {0};
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            A[i][j] = B[i + j * 4];
        }
    }
    // to avoid compiler warnings
    iPivotRow = iPivotCol = 0;

    // main loop i over the dimensions of the square matrix A
    for (i = 0; i < isize; i++)
    {
        // zero the largest element found for pivoting
        largest = 0.0F;
        // loop over candidate rows j
        for (j = 0; j < isize; j++) {
            // check if row j has been previously pivoted
            if (iPivot[j] != 1) {
                // loop over candidate columns k
                for (k = 0; k < isize; k++) {
                    // check if column k has previously been pivoted
                    if (iPivot[k] == 0) {
                        // check if the pivot element is the largest found so far
                        if (fabs(A[j][k]) >= largest) {
                            // and store this location as the current best candidate for pivoting
                            iPivotRow = j;
                            iPivotCol = k;
                            largest = (float) fabs(A[iPivotRow][iPivotCol]);
                        }
                    }
                    else if (iPivot[k] > 1) {
                        // zero determinant situation: exit with identity matrix
                        //fmatrixAeqI(A, isize);
                        return IST_FALSE;
                    }
                }
            }
        }
        // increment the entry in iPivot to denote it has been selected for pivoting
        iPivot[iPivotCol]++;

        // check the pivot rows iPivotRow and iPivotCol are not the same before swapping
        if (iPivotRow != iPivotCol) {
            // loop over columns l
            for (l = 0; l < isize; l++) {
                // and swap all elements of rows iPivotRow and iPivotCol
                ftmp = A[iPivotRow][l];
                A[iPivotRow][l] = A[iPivotCol][l];
                A[iPivotCol][l] = ftmp;
            }
        }

        // record that on the i-th iteration rows iPivotRow and iPivotCol were swapped
        iRowInd[i] = iPivotRow;
        iColInd[i] = iPivotCol;

        // check for zero on-diagonal element (singular matrix) and return with identity matrix if detected
        if (A[iPivotCol][iPivotCol] == 0.0F) {
            // zero determinant situation: exit with identity matrix
            //fmatrixAeqI(A, isize);
            return IST_FALSE;
        }

        // calculate the reciprocal of the pivot element knowing it's non-zero
        recippiv = 1.0F / A[iPivotCol][iPivotCol];
        // by definition, the diagonal element normalizes to 1
        A[iPivotCol][iPivotCol] = 1.0F;
        // multiply all of row iPivotCol by the reciprocal of the pivot element including the diagonal element
        // the diagonal element A[iPivotCol][iPivotCol] now has value equal to the reciprocal of its previous value
        for (l = 0; l < isize; l++) {
            A[iPivotCol][l] *= recippiv;
        }
        // loop over all rows m of A
        for (m = 0; m < isize; m++) {
            if (m != iPivotCol) {
                // scaling factor for this row m is in column iPivotCol
                scaling = A[m][iPivotCol];
                // zero this element
                A[m][iPivotCol] = 0.0F;
                // loop over all columns l of A and perform elimination
                for (l = 0; l < isize; l++)
                {
                    A[m][l] -= A[iPivotCol][l] * scaling;
                }
            }
        }
    } // end of loop i over the matrix dimensions

    // finally, loop in inverse order to apply the missing column swaps
    for (l = isize - 1; l >= 0; l--) {
        // set i and j to the two columns to be swapped
        i = iRowInd[l];
        j = iColInd[l];

        // check that the two columns i and j to be swapped are not the same
        if (i != j) {
            // loop over all rows k to swap columns i and j of A
            for (k = 0; k < isize; k++) {
                ftmp = A[k][i];
                A[k][i] = A[k][j];
                A[k][j] = ftmp;
            }
        }
    }

    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            C[i + j * 4] = A[i][j];
        }
    }
    return IST_TRUE;
}

ISTBOOL InvertMatrix3by3(const float invIn[], float invOut[])
{
    float inv[9], det;
    int i;

    det = invIn[0] * invIn[4] * invIn[8]
          + invIn[1] * invIn[5] * invIn[6]
          + invIn[2] * invIn[3] * invIn[7]
          - invIn[0] * invIn[5] * invIn[7]
          - invIn[2] * invIn[4] * invIn[6]
          - invIn[1] * invIn[3] * invIn[8];

    if (det == 0)
        return IST_FALSE;

    det = ((float)1) / det;

    inv[0] = invIn[4] * invIn[8] - invIn[5] * invIn[7];
    inv[1] = invIn[2] * invIn[7] - invIn[1] * invIn[8];
    inv[2] = invIn[1] * invIn[5] - invIn[2] * invIn[4];
    inv[3] = invIn[5] * invIn[6] - invIn[3] * invIn[8];
    inv[4] = invIn[0] * invIn[8] - invIn[2] * invIn[6];
    inv[5] = invIn[2] * invIn[3] - invIn[0] * invIn[5];
    inv[6] = invIn[3] * invIn[7] - invIn[4] * invIn[6];
    inv[7] = invIn[1] * invIn[6] - invIn[0] * invIn[7];
    inv[8] = invIn[0] * invIn[4] - invIn[1] * invIn[3];

    for (i = 0; i < 9; i++)
        invOut[i] = inv[i] * det;

    return IST_TRUE;
}


float Get_Distance(const float x[3], const float y[3])
{
    int i = 0;
    float tmp = 0, r = 0;

    for (i = 0; i < 3; ++i) {
        tmp = (x[i] - y[i]);
        r = (r + pow(tmp, 2));
    }
    if ((r <= 0)) {
        return 0;
    }

    r = sqrt(r);
    return r;
}
