# Help_Serialize

A JSON serialization library for C++26, built from scratch using reflection.

`hp::ser::to_json(value)` and `hp::ser::from_json<T>(text)` work on any struct or class, no macros, no per-type code, no external dependencies.

## Example

```cpp
#include "serialize.hpp"

struct Item {
    std::string name;
    int value;
};

struct Player {
    std::string name;
    int level;
    std::vector<Item> inventory;
};

Player p{"Xavi", 15, {{"sword", 50}, {"potion", 10}}};

std::string json = hp::ser::to_json(p);
// {"name": "Xavi", "level": 15, "inventory": [{"name": "sword", "value": 50}, {"name": "potion", "value": 10}]}

Player loaded = hp::ser::from_json<Player>(json);
// loaded == p
```

## Supported types
- Primitives	int, double, bool, char
- Strings	    std::string, std::string_view, std::filesystem::path
- Containers	std::vector, std::array, std::set, std::map, std::pair
- Optional	    std::optional<T>
- Pointers	    std::unique_ptr<T>, std::shared_ptr<T>
- Enums	enum and enum class (serialized as the enumerator name)
- Structs/Classes	Any aggregate or class, including private members

std::weak_ptr is supported by to_json but not by from_json, if you believe this is an issue, may i ask why do you need weak_ptr ?

## How it works
### to_json
Uses C++26 reflection to iterate a struct's fields at compile time:

```cpp
template for (constexpr auto m : members) {
    result += std::format("\"{}\": ", std::meta::identifier_of(m));
    result += to_json(value.[:m:]);
}
```
members comes from std::meta::nonstatic_data_members_of(^^T). The spliced `value.[:m:]` reads each field. identifier_of(m) gets its name.

### from_json
Parses JSON into a JsonValue :

```cpp
template for (constexpr auto m : members) {
    std::string key(std::meta::identifier_of(m));
    auto it = obj.find(key);
    if (it == obj.end()) continue;
    using FieldType = std::remove_cvref_t<decltype(result.[:m:])>;
    result.[:m:] = from_json_value<FieldType>(*it->second);
}
```
Same reflection loop, opposite direction. 

## The JSON parser
`hp::ser::Parser` is a json parser:

- Lexer — tokenizes the tokens like in a programming language
- Parser — builds a tree of JsonValue
- to_string — prints the tree back to text
- No external JSON library. The parser is part of the project (too lazy to learn nhloman).

## Build
Requires:
- GCC 16.1+ or a compiler with C++26 reflection support (dont try to use clang pls)
-std=c++26 -freflection -lstdc++exp

### Built with Help_Make:
```cmd
hm -b
```

Or directly:
```cmd
g++ -std=c++26 -freflection src/*.cpp -o test.exe -Iinclude -lstdc++exp
```

## Status
148 tests passing
All supported types round-trip correctly
to_json and from_json are symmetric (except for weak_ptr ofc)

## Limitations
- weak_ptr not supported in from_json. A compile error explains why.
- No pretty printing yet. Output is compact.
- No std::variant, std::tuple, std::chrono support,std::set,etc...

## License
This program is licensed under the MIT License, see LICENSE file for more details.