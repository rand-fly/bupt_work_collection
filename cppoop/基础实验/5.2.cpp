#include <iostream>

class Point {
  public:
    Point(double x, double y)
        : x(x), y(y) {}

    void output() {
        std::cout << "(" << x << ", " << y << ")" << std::endl;
    }

    Point &operator++() {
        x += 1;
        y += 1;
        return *this;
    }

    Point operator++(int) {
        Point tmp = *this;
        operator++();
        return tmp;
    }

    Point &operator--() {
        x -= 1;
        y -= 1;
        return *this;
    }

    Point operator--(int) {
        Point tmp = *this;
        operator--();
        return tmp;
    }

  private:
    double x, y;
};

int main() {
    Point p(1, 2);
    (++p).output();
    (p++).output();
    p.output();
}