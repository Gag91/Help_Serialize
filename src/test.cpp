#include "serialize.hpp"
#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <print>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct Point {
    int x;
    int y;
};

struct Person {
    std::string name;
    int age;
    bool active;
};

struct Line {
    Point start;
    Point end;
};

struct Polygon {
    std::string name;
    std::vector<Point> points;
};

enum class Color { Red,
                   Green,
                   Blue };
enum class Status { Ok,
                    Error,
                    Pending };

struct Pixel {
    Color color;
    Status status;
    int x;
    int y;
};

class Player {
  public:
    static constexpr bool hp_serializable = true;

    std::string get_name() const {
        return name_;
    }
    int get_level() const {
        return level_;
    }

  private:
    std::string name_;
    int level_ = 0;
};

static int g_pass = 0;
static int g_fail = 0;

void check(bool cond, const std::string &label) {
    if (cond) {
        ++g_pass;
        std::println("  [PASS] {}", label);
    } else {
        ++g_fail;
        std::println("  [FAIL] {}", label);
    }
}

template <typename T>
void check_eq(const T &a, const T &b, const std::string &label) {
    check(a == b, label);
}

void test_to_json_primitives() {
    std::println("=== to_json: primitives ===");
    check_eq(hp::ser::to_json(42), std::string("42"), "int 42");
    check_eq(hp::ser::to_json(-7), std::string("-7"), "int -7");
    check_eq(hp::ser::to_json(0), std::string("0"), "int 0");
    check_eq(hp::ser::to_json(3.14), std::string("3.14"), "double 3.14");
    check_eq(hp::ser::to_json(-2.5), std::string("-2.5"), "double -2.5");
    check_eq(hp::ser::to_json(true), std::string("true"), "bool true");
    check_eq(hp::ser::to_json(false), std::string("false"), "bool false");
    check_eq(hp::ser::to_json('a'), std::string("\"a\""), "char 'a'");
    check_eq(hp::ser::to_json('"'), std::string("\"\\\"\""), "char quote");
}

void test_to_json_strings() {
    std::println("=== to_json: strings ===");
    check_eq(hp::ser::to_json(std::string("hello")), std::string("\"hello\""), "plain string");
    check_eq(hp::ser::to_json(std::string("")), std::string("\"\""), "empty string");
    check_eq(hp::ser::to_json(std::string("say \"hi\"")), std::string("\"say \\\"hi\\\"\""), "escaped quote");
    check_eq(hp::ser::to_json(std::string("a\nb")), std::string("\"a\\nb\""), "newline escape");
    check_eq(hp::ser::to_json(std::string("a\tb")), std::string("\"a\\tb\""), "tab escape");
    check_eq(hp::ser::to_json(std::string("a\\b")), std::string("\"a\\\\b\""), "backslash escape");

    std::string_view sv = "view";
    check_eq(hp::ser::to_json(sv), std::string("\"view\""), "string_view");

    std::filesystem::path p = "C:/test/file.txt";
    check_eq(hp::ser::to_json(p), std::string("\"C:/test/file.txt\""), "filesystem::path");
}

void test_to_json_containers() {
    std::println("=== to_json: containers ===");
    check_eq(hp::ser::to_json(std::vector<int>{1, 2, 3}), std::string("[1, 2, 3]"), "vector<int>");
    check_eq(hp::ser::to_json(std::vector<int>{}), std::string("[]"), "empty vector");
    check_eq(hp::ser::to_json(std::vector<int>{42}), std::string("[42]"), "single element vector");
    check_eq(hp::ser::to_json(std::vector<std::string>{"a", "b"}), std::string("[\"a\", \"b\"]"), "vector<string>");

    std::vector<std::vector<int>> nested{{1, 2}, {3, 4}};
    check_eq(hp::ser::to_json(nested), std::string("[[1, 2], [3, 4]]"), "nested vector");

    check_eq(hp::ser::to_json(std::array<int, 3>{1, 2, 3}), std::string("[1, 2, 3]"), "array<int,3>");
    check_eq(hp::ser::to_json(std::set<int>{1, 2, 3}), std::string("[1, 2, 3]"), "set<int>");
    check_eq(hp::ser::to_json(std::pair<int, int>{1, 2}), std::string("[1, 2]"), "pair<int,int>");
    check_eq(hp::ser::to_json(std::pair<int, std::string>{42, "hi"}), std::string("[42, \"hi\"]"), "pair<int,string>");

    std::map<std::string, int> m{{"a", 1}, {"b", 2}};
    check_eq(hp::ser::to_json(m), std::string("{\"a\": 1, \"b\": 2}"), "map<string,int>");
}

void test_to_json_optional() {
    std::println("=== to_json: optional ===");
    check_eq(hp::ser::to_json(std::optional<int>{42}), std::string("42"), "optional with value");
    check_eq(hp::ser::to_json(std::optional<int>{}), std::string("null"), "optional empty");
    check_eq(hp::ser::to_json(std::optional<std::string>{"hi"}), std::string("\"hi\""), "optional<string>");
}

void test_to_json_pointers() {
    std::println("=== to_json: pointers ===");
    auto up = std::make_unique<int>(42);
    check_eq(hp::ser::to_json(up), std::string("42"), "unique_ptr with value");

    auto sp = std::make_shared<int>(42);
    check_eq(hp::ser::to_json(sp), std::string("42"), "shared_ptr with value");

    std::unique_ptr<int> empty_up;
    check_eq(hp::ser::to_json(empty_up), std::string("null"), "empty unique_ptr");

    std::shared_ptr<int> empty_sp;
    check_eq(hp::ser::to_json(empty_sp), std::string("null"), "empty shared_ptr");

    auto sp_str = std::make_shared<std::string>("hello");
    check_eq(hp::ser::to_json(sp_str), std::string("\"hello\""), "shared_ptr<string>");
}

void test_to_json_enums() {
    std::println("=== to_json: enums ===");
    check_eq(hp::ser::to_json(Color::Red), std::string("\"Red\""), "Color::Red");
    check_eq(hp::ser::to_json(Color::Green), std::string("\"Green\""), "Color::Green");
    check_eq(hp::ser::to_json(Color::Blue), std::string("\"Blue\""), "Color::Blue");
    check_eq(hp::ser::to_json(Status::Ok), std::string("\"Ok\""), "Status::Ok");
    check_eq(hp::ser::to_json(Status::Error), std::string("\"Error\""), "Status::Error");
}

void test_to_json_structs() {
    std::println("=== to_json: structs ===");
    check_eq(hp::ser::to_json(Point{3, 4}), std::string("{\"x\": 3, \"y\": 4}"), "Point");
    check_eq(hp::ser::to_json(Person{"Xavi", 15, true}),
             std::string("{\"name\": \"Xavi\", \"age\": 15, \"active\": true}"), "Person");
    check_eq(hp::ser::to_json(Line{{1, 2}, {3, 4}}),
             std::string("{\"start\": {\"x\": 1, \"y\": 2}, \"end\": {\"x\": 3, \"y\": 4}}"), "Line nested");

    Polygon poly{"tri", {{0, 0}, {1, 0}, {0, 1}}};
    std::string poly_json = hp::ser::to_json(poly);
    check(poly_json.find("\"name\": \"tri\"") != std::string::npos, "Polygon has name");
    check(poly_json.find("\"points\":") != std::string::npos, "Polygon has points array");

    Pixel px{Color::Green, Status::Ok, 10, 20};
    check_eq(hp::ser::to_json(px),
             std::string("{\"color\": \"Green\", \"status\": \"Ok\", \"x\": 10, \"y\": 20}"), "Pixel with enums");
}

void test_to_json_classes() {
    std::println("=== to_json: classes ===");
    Player p;
    std::string json = hp::ser::to_json(p);
    check(json.find("name_") != std::string::npos, "Player has name_");
    check(json.find("level_") != std::string::npos, "Player has level_");
}

void test_from_json_primitives() {
    std::println("=== from_json: primitives ===");
    check_eq(hp::ser::from_json<int>("42"), 42, "int 42");
    check_eq(hp::ser::from_json<int>("-7"), -7, "int -7");
    check_eq(hp::ser::from_json<int>("0"), 0, "int 0");
    check_eq(hp::ser::from_json<double>("3.14"), 3.14, "double");
    check_eq(hp::ser::from_json<double>("-2.5"), -2.5, "double negative");
    check_eq(hp::ser::from_json<bool>("true"), true, "bool true");
    check_eq(hp::ser::from_json<bool>("false"), false, "bool false");
    check_eq(hp::ser::from_json<std::string>(R"("hello")"), std::string("hello"), "string");
    check_eq(hp::ser::from_json<std::string>(R"("")"), std::string(""), "empty string");
}

void test_from_json_containers() {
    std::println("=== from_json: containers ===");

    auto v = hp::ser::from_json<std::vector<int>>("[1, 2, 3]");
    check_eq(v, std::vector<int>{1, 2, 3}, "vector<int>");

    auto empty_v = hp::ser::from_json<std::vector<int>>("[]");
    check_eq(empty_v.size(), std::size_t{0}, "empty vector");

    auto single_v = hp::ser::from_json<std::vector<int>>("[42]");
    check_eq(single_v.size(), std::size_t{1}, "single element size");
    check_eq(single_v[0], 42, "single element value");

    auto vs = hp::ser::from_json<std::vector<std::string>>(R"(["a", "b", "c"])");
    check_eq(vs.size(), std::size_t{3}, "vector<string> size");
    check_eq(vs[0], std::string("a"), "vector<string>[0]");
    check_eq(vs[2], std::string("c"), "vector<string>[2]");

    auto nested = hp::ser::from_json<std::vector<std::vector<int>>>("[[1, 2], [3, 4]]");
    check_eq(nested.size(), std::size_t{2}, "nested vector outer size");
    check_eq(nested[0].size(), std::size_t{2}, "nested vector inner size");
    check_eq(nested[0][0], 1, "nested[0][0]");
    check_eq(nested[1][1], 4, "nested[1][1]");

    auto m = hp::ser::from_json<std::map<std::string, int>>(R"({"a": 1, "b": 2})");
    check_eq(m.size(), std::size_t{2}, "map size");
    check_eq(m["a"], 1, "map[a]");
    check_eq(m["b"], 2, "map[b]");

    auto empty_m = hp::ser::from_json<std::map<std::string, int>>("{}");
    check_eq(empty_m.size(), std::size_t{0}, "empty map");
}

void test_from_json_optional() {
    std::println("=== from_json: optional ===");

    auto o1 = hp::ser::from_json<std::optional<int>>("42");
    check(o1.has_value(), "optional has value");
    check_eq(*o1, 42, "optional value is 42");

    auto o2 = hp::ser::from_json<std::optional<int>>("null");
    check(!o2.has_value(), "optional empty");

    auto o3 = hp::ser::from_json<std::optional<std::string>>(R"("hi")");
    check(o3.has_value(), "optional<string> has value");
    check_eq(*o3, std::string("hi"), "optional<string> value");
}

void test_from_json_pointers() {
    std::println("=== from_json: pointers ===");

    auto up = hp::ser::from_json<std::unique_ptr<int>>("42");
    check(up != nullptr, "unique_ptr not null");
    check_eq(*up, 42, "unique_ptr value");

    auto sp = hp::ser::from_json<std::shared_ptr<int>>("42");
    check(sp != nullptr, "shared_ptr not null");
    check_eq(*sp, 42, "shared_ptr value");

    auto null_up = hp::ser::from_json<std::unique_ptr<int>>("null");
    check(null_up == nullptr, "null unique_ptr");

    auto null_sp = hp::ser::from_json<std::shared_ptr<int>>("null");
    check(null_sp == nullptr, "null shared_ptr");

    auto sp_str = hp::ser::from_json<std::shared_ptr<std::string>>(R"("hello")");
    check(sp_str != nullptr, "shared_ptr<string> not null");
    check_eq(*sp_str, std::string("hello"), "shared_ptr<string> value");
}

void test_from_json_structs() {
    std::println("=== from_json: structs ===");

    auto p = hp::ser::from_json<Point>(R"({"x": 3, "y": 4})");
    check_eq(p.x, 3, "Point.x");
    check_eq(p.y, 4, "Point.y");

    auto person = hp::ser::from_json<Person>(R"({"name": "Xavi", "age": 15, "active": true})");
    check_eq(person.name, std::string("Xavi"), "Person.name");
    check_eq(person.age, 15, "Person.age");
    check_eq(person.active, true, "Person.active");

    auto line = hp::ser::from_json<Line>(R"({"start": {"x": 1, "y": 2}, "end": {"x": 3, "y": 4}})");
    check_eq(line.start.x, 1, "Line.start.x");
    check_eq(line.start.y, 2, "Line.start.y");
    check_eq(line.end.x, 3, "Line.end.x");
    check_eq(line.end.y, 4, "Line.end.y");

    auto px = hp::ser::from_json<Pixel>(R"({"color": "Green", "status": "Ok", "x": 10, "y": 20})");
    check_eq(px.x, 10, "Pixel.x");
    check_eq(px.y, 20, "Pixel.y");
}

void test_from_json_classes() {
    std::println("=== from_json: classes ===");
    auto p = hp::ser::from_json<Player>(R"({"name_": "Alice", "level_": 42})");
    check_eq(p.get_name(), std::string("Alice"), "Player.name");
    check_eq(p.get_level(), 42, "Player.level");
}

void test_from_json_missing_fields() {
    std::println("=== from_json: missing fields ===");
    auto p = hp::ser::from_json<Point>(R"({"x": 3})");
    check_eq(p.x, 3, "Point.x present");
    check_eq(p.y, 0, "Point.y defaulted to 0");

    auto p2 = hp::ser::from_json<Point>("{}");
    check_eq(p2.x, 0, "empty object x defaults");
    check_eq(p2.y, 0, "empty object y defaults");
}

void test_from_json_extra_fields() {
    std::println("=== from_json: extra fields ===");
    auto p = hp::ser::from_json<Point>(R"({"x": 3, "y": 4, "z": 99, "w": "ignored"})");
    check_eq(p.x, 3, "Point.x with extra fields");
    check_eq(p.y, 4, "Point.y with extra fields");
}

void test_roundtrip_primitives() {
    std::println("=== round-trip: primitives ===");

    check_eq(hp::ser::from_json<int>(hp::ser::to_json(42)), 42, "int round-trip");
    check_eq(hp::ser::from_json<int>(hp::ser::to_json(-7)), -7, "negative int round-trip");
    check_eq(hp::ser::from_json<double>(hp::ser::to_json(3.14)), 3.14, "double round-trip");
    check_eq(hp::ser::from_json<bool>(hp::ser::to_json(true)), true, "true round-trip");
    check_eq(hp::ser::from_json<bool>(hp::ser::to_json(false)), false, "false round-trip");
    check_eq(hp::ser::from_json<std::string>(hp::ser::to_json(std::string("hello"))),
             std::string("hello"), "string round-trip");
    check_eq(hp::ser::from_json<std::string>(hp::ser::to_json(std::string("say \"hi\""))),
             std::string("say \"hi\""), "escaped string round-trip");
}

void test_roundtrip_containers() {
    std::println("=== round-trip: containers ===");

    std::vector<int> vi{1, 2, 3};
    check_eq(hp::ser::from_json<std::vector<int>>(hp::ser::to_json(vi)), vi, "vector<int>");

    std::vector<std::string> vs{"a", "b", "c"};
    check_eq(hp::ser::from_json<std::vector<std::string>>(hp::ser::to_json(vs)), vs, "vector<string>");

    std::vector<std::vector<int>> vn{{1, 2}, {3, 4}};
    check_eq(hp::ser::from_json<std::vector<std::vector<int>>>(hp::ser::to_json(vn)), vn, "nested vector");

    std::map<std::string, int> m{{"a", 1}, {"b", 2}};
    check_eq(hp::ser::from_json<std::map<std::string, int>>(hp::ser::to_json(m)), m, "map");

    std::map<std::string, std::vector<int>> mv{{"a", {1, 2}}, {"b", {3, 4}}};
    check_eq(hp::ser::from_json<std::map<std::string, std::vector<int>>>(hp::ser::to_json(mv)), mv, "map of vectors");
}

void test_roundtrip_optional() {
    std::println("=== round-trip: optional ===");

    auto o1 = hp::ser::from_json<std::optional<int>>(hp::ser::to_json(std::optional<int>{42}));
    check(o1.has_value(), "optional<int> has value");
    check_eq(*o1, 42, "optional<int> value");

    auto o2 = hp::ser::from_json<std::optional<int>>(hp::ser::to_json(std::optional<int>{}));
    check(!o2.has_value(), "empty optional stays empty");
}

void test_roundtrip_structs() {
    std::println("=== round-trip: structs ===");

    Point p{3, 4};
    auto loaded_p = hp::ser::from_json<Point>(hp::ser::to_json(p));
    check_eq(loaded_p.x, p.x, "Point.x");
    check_eq(loaded_p.y, p.y, "Point.y");

    Person person{"Xavi", 15, true};
    auto loaded_person = hp::ser::from_json<Person>(hp::ser::to_json(person));
    check_eq(loaded_person.name, person.name, "Person.name");
    check_eq(loaded_person.age, person.age, "Person.age");
    check_eq(loaded_person.active, person.active, "Person.active");

    Line line{{1, 2}, {3, 4}};
    auto loaded_line = hp::ser::from_json<Line>(hp::ser::to_json(line));
    check_eq(loaded_line.start.x, line.start.x, "Line.start.x");
    check_eq(loaded_line.end.y, line.end.y, "Line.end.y");

    Pixel px{Color::Blue, Status::Pending, 5, 10};
    auto loaded_px = hp::ser::from_json<Pixel>(hp::ser::to_json(px));
    check_eq(loaded_px.x, px.x, "Pixel.x");
    check_eq(loaded_px.y, px.y, "Pixel.y");
}

void test_roundtrip_nested() {
    std::println("=== round-trip: nested ===");

    Polygon poly{"triangle", {{0, 0}, {1, 0}, {0, 1}}};
    auto loaded = hp::ser::from_json<Polygon>(hp::ser::to_json(poly));
    check_eq(loaded.name, poly.name, "Polygon.name");
    check_eq(loaded.points.size(), poly.points.size(), "Polygon.points size");
    for (std::size_t i = 0; i < loaded.points.size(); ++i) {
        check_eq(loaded.points[i].x, poly.points[i].x, "point[" + std::to_string(i) + "].x");
        check_eq(loaded.points[i].y, poly.points[i].y, "point[" + std::to_string(i) + "].y");
    }

    std::vector<Point> points{{1, 2}, {3, 4}, {5, 6}};
    auto loaded_points = hp::ser::from_json<std::vector<Point>>(hp::ser::to_json(points));
    check_eq(loaded_points.size(), points.size(), "vector<Point> size");
    for (std::size_t i = 0; i < loaded_points.size(); ++i) {
        check_eq(loaded_points[i].x, points[i].x, "point[" + std::to_string(i) + "].x");
        check_eq(loaded_points[i].y, points[i].y, "point[" + std::to_string(i) + "].y");
    }
}

void test_roundtrip_deep() {
    std::println("=== round-trip: deep nesting ===");

    std::map<std::string, std::vector<Point>> data{
        {"group1", {{1, 2}, {3, 4}}},
        {"group2", {{5, 6}}},
    };
    auto loaded = hp::ser::from_json<std::map<std::string, std::vector<Point>>>(hp::ser::to_json(data));
    check_eq(loaded.size(), data.size(), "map size");
    check_eq(loaded["group1"].size(), std::size_t{2}, "group1 size");
    check_eq(loaded["group2"].size(), std::size_t{1}, "group2 size");
    check_eq(loaded["group1"][0].x, 1, "group1[0].x");
    check_eq(loaded["group2"][0].y, 6, "group2[0].y");
}

int main() {
    test_to_json_primitives();
    test_to_json_strings();
    test_to_json_containers();
    test_to_json_optional();
    test_to_json_pointers();
    test_to_json_enums();
    test_to_json_structs();
    test_to_json_classes();

    test_from_json_primitives();
    test_from_json_containers();
    test_from_json_optional();
    test_from_json_pointers();
    test_from_json_structs();
    test_from_json_classes();
    test_from_json_missing_fields();
    test_from_json_extra_fields();

    test_roundtrip_primitives();
    test_roundtrip_containers();
    test_roundtrip_optional();
    test_roundtrip_structs();
    test_roundtrip_nested();
    test_roundtrip_deep();

    std::println("\n==============================");
    std::println("Passed: {}   Failed: {}", g_pass, g_fail);
    std::println("==============================");

    return g_fail == 0 ? 0 : 1;
}