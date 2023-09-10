#include <cstdlib>
#include <ctime>
#include <iostream>
#include <limits>

// https://zh.cppreference.com/w/cpp/io/basic_istream/ignore
const long long max_size = std::numeric_limits<std::streamsize>::max();

int main() {
    std::srand(std::time(0));
    int num = std::rand() % 1000 + 1;
    int guess;
    do {
        std::cout << "Make a guess: ";
        if (!(std::cin >> guess)) {
            std::cout << "Wrong input." << std::endl;
            std::cin.clear();
            std::cin.ignore(max_size, '\n');
        } else if (guess > num) {
            std::cout << "The price you guess is too high." << std::endl;
        } else if (guess < num) {
            std::cout << "The price you guess is too low." << std::endl;
        }
    } while (guess != num);
    std::cout << "You are right!" << std::endl;
}