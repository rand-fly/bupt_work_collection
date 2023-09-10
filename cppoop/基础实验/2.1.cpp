#include <cmath>
#include <iostream>

class Point {
  public:
    Point(double x = 0, double y = 0)
        : x(x), y(y) {
        std::cout << "Point constructor called." << std::endl;
    }

    ~Point() {
        std::cout << "Point destructor called." << std::endl;
    }

    double dis(Point a) {
        double dx = x - a.x;
        double dy = y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

  private:
    double x, y;
};

class Circle {
  public:
    Circle(Point center, double radius)
        : center(center), radius(radius) {
        std::cout << "Circle constructor called." << std::endl;
    }

    ~Circle() {
        std::cout << "Circle destructor called." << std::endl;
    }

    bool intersect(Circle a) {
        int dis = center.dis(a.center);
        return dis <= radius + a.radius && dis >= std::abs(radius - a.radius);
    }

  private:
    Point center;
    double radius;
};

int main() {
    double x1, y1, r1, x2, y2, r2;
    std::cout << "Please input the center of circle 1: ";
    std::cin >> x1 >> y1;
    std::cout << "Please input the radius of circle 1: ";
    std::cin >> r1;
    std::cout << "Please input the center of circle 2: ";
    std::cin >> x2 >> y2;
    std::cout << "Please input the radius of circle 2: ";
    std::cin >> r2;

    Circle c1 = Circle(Point(x1, y1), r1);
    Circle c2 = Circle(Point(x2, y2), r2);

    if (c1.intersect(c2)) {
        std::cout << "Circle 1 intersects with circle 2." << std::endl;
    } else {
        std::cout << "Circle 1 does not intersect with circle 2." << std::endl;
    }
}