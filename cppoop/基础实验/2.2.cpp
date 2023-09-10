#include <cassert>
#include <iostream>

class Matrix {
  public:
    Matrix(int lines, int rows)
        : lines(lines), rows(rows) {
        data = new int[lines * rows];
    }

    ~Matrix() {
        delete[] data;
    }

    Matrix(const Matrix &a) {
        lines = a.lines;
        rows = a.rows;
        data = new int[lines * rows];
        for (int i = 0; i < lines * rows; i++) {
            data[i] = a.data[i];
        }
    }

    Matrix(Matrix &&a) {
        data = a.data;
        a.data = nullptr;
    }

    Matrix &operator=(const Matrix &a) {
        if (this == &a) {
            return *this;
        }
        delete[] data;
        lines = a.lines;
        rows = a.rows;
        data = new int[lines * rows];
        for (int i = 0; i < lines * rows; i++) {
            data[i] = a.data[i];
        }
        return *this;
    }

    Matrix &operator=(Matrix &&a) {
        if (this == &a) {
            return *this;
        }
        delete[] data;
        lines = a.lines;
        rows = a.rows;
        data = a.data;
        a.data = nullptr;
        return *this;
    }

    void input() {
        for (int i = 0; i < lines; i++) {
            for (int j = 0; j < rows; j++) {
                std::cin >> data[i * rows + j];
            }
        }
    }

    void output() {
        for (int i = 0; i < lines; i++) {
            std::cout << data[i * rows];
            for (int j = 1; j < rows; j++) {
                std::cout << " " << data[i * rows + j];
            }
            std::cout << std::endl;
        }
    }

    Matrix operator+(const Matrix &a) {
        assert(lines == a.lines && rows == a.rows);
        Matrix ans(lines, rows);
        for (int i = 0; i < lines * rows; i++) {
            ans.data[i] = data[i] + a.data[i];
        }
        return ans;
    }

    Matrix operator-(const Matrix &a) {
        assert(lines == a.lines && rows == a.rows);
        Matrix ans(lines, rows);
        for (int i = 0; i < lines * rows; i++) {
            ans.data[i] = data[i] - a.data[i];
        }
        return ans;
    }

  private:
    int lines, rows;
    int *data;
};

int main() {
    Matrix A1(4, 5), A2(4, 5), A3(4, 5);
    A1.input();
    A2.input();
    A3 = A1 + A2;
    A3.output();
    A3 = A1 - A2;
    A3.output();

    Matrix *pA1 = new Matrix(4, 5), *pA2 = new Matrix(4, 5), *pA3 = new Matrix(4, 5);
    pA1->input();
    pA2->input();
    *pA3 = *pA1 + *pA2;
    pA3->output();
    *pA3 = *pA1 - *pA2;
    pA3->output();
    delete pA1;
    delete pA2;
    delete pA3;
}