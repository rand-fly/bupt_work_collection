#include <cmath>
#include <iostream>
class Shape {
  public:
    double area() {
        return 0;
    }
};

class Rectangle : public Shape {
  public:
    Rectangle(double width, double height)
        : width(width), height(height) {
        std::cout << "Rectangle constructor called." << std::endl;
    }

    ~Rectangle() {
        std::cout << "Rectangle destructor called." << std::endl;
    }

    double area() {
        return width * height;
    }

  private:
    double width, height;
};

class Square : public Rectangle {
  public:
    Square(double length)
        : Rectangle(length, length) {
        std::cout << "Square constructor called." << std::endl;
    }
    ~Square() {
        std::cout << "Square destructor called." << std::endl;
    }
};

class Circle : public Shape {
  public:
    Circle(double radius)
        : radius(radius) {
        std::cout << "Circle constructor called." << std::endl;
    }

    ~Circle() {
        std::cout << "Circle destructor called." << std::endl;
    }

    double area() {
        return 3.14159265358979323846 * radius * radius;
    }

  private:
    double radius;
};

int main() {
    Rectangle s1(3, 2);
    std::cout << s1.area() << std::endl;
    Square s2(4);
    std::cout << s2.area() << std::endl;
    Circle s3(4);
    std::cout << s3.area() << std::endl;
}