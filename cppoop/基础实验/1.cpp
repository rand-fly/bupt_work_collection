#include <iostream>
const int row = 4, col = 5;

void input(int *mat) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            std::cin >> mat[i * col + j];
        }
    }
}

void output(int *mat) {
    for (int i = 0; i < row; i++) {
        std::cout << mat[i * col];
        for (int j = 1; j < col; j++) {
            std::cout << " " << mat[i * col + j];
        }
        std::cout << std::endl;
    }
}

void add(int *dest, int *mat1, int *mat2) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            dest[i * col + j] = mat1[i * col + j] + mat2[i * col + j];
        }
    }
}

void sub(int *dest, int *mat1, int *mat2) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            dest[i * col + j] = mat1[i * col + j] - mat2[i * col + j];
        }
    }
}

int main() {
    int *A1 = new int[row * col];
    int *A2 = new int[row * col];
    int *A3 = new int[row * col];
    input(A1);
    input(A2);
    add(A3, A1, A2);
    output(A3);
    sub(A3, A1, A2);
    output(A3);
    delete[] A1;
    delete[] A2;
    delete[] A3;
}