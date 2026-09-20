#include "serialize.hpp"
#include <array>
#include <filesystem>
#include <iostream>
#include <print>
#include <set>
#include <string>
#include <vector>

struct Point {
    int x;
    int y;
};

struct Particle {
    std::string name;
    Point position;
    std::array<double, 3> velocity;
    bool active;
};

struct Grid {
    std::array<std::array<int, 3>, 3> cells;
    std::array<Point, 2> anchors;
    std::array<std::string, 2> labels;
    std::array<Particle, 2> particles;
};

enum class Color {
    Red,
    Green,
    Blue
};
enum class Status {
    Ok,
    Error,
    Pending
};

struct Pixel {
    Color color;
    Status status;
    int x;
    int y;
};

int main() {

    Pixel p{Color::Green, Status::Ok, 10, 20};
    std::println("{}", hp::ser::to_json(p));
}