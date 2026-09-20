#pragma once

#include <array>
#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <meta>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace hp::ser {

    template <typename T>
    std::string to_json(const std::unique_ptr<T> &ptr);

    template <typename T>
    std::string to_json(const std::shared_ptr<T> &ptr);

    template <typename T>
    std::string to_json(const std::weak_ptr<T> &ptr);

    template <typename T, typename U>
    std::string to_json(const std::map<T, U> &map);

    template <typename T, typename U>
    std::string to_json(const std::pair<T, U> &pair);

    template <typename T>
    std::string to_json(const std::vector<T> &vec);

    template <typename T>
    std::string to_json(const std::set<T> &vec);

    template <typename T, std::size_t U>
    std::string to_json(const std::array<T, U> &arr);

    template <typename T>
    std::string to_json(const std::optional<T> &value);

    template <typename T>
        requires std::is_enum_v<T>
    std::string to_json(T value);

    template <typename T>
        requires(!std::is_enum_v<T>)
    std::string to_json(const T &value);

    inline std::string to_json(const int &value);
    inline std::string to_json(const double &value);
    inline std::string to_json(const bool &value);
    inline std::string to_json(const char &value);
    inline std::string to_json(const std::string &value);
    inline std::string to_json(const std::string_view &value);
    inline std::string to_json(const std::filesystem::path &path);

    inline std::string to_json(const int &value) {
        return std::format("{}", value);
    }

    inline std::string to_json(const double &value) {
        return std::format("{}", value);
    }

    inline std::string to_json(const bool &value) {
        return value ? "true" : "false";
    }

    inline std::string to_json(const std::filesystem::path &path) {
        return to_json(path.generic_string());
    }

    template <typename T>
    inline std::string to_json(const std::optional<T> &value) {
        std::string result;
        if (!value.has_value()) {
            return "null";
        }
        return to_json(value);
    }

    inline std::string to_json(const std::string &value) {
        std::string result = "\"";
        for (char c : value) {
            switch (c) {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                default:
                    result += c;
            }
        }
        result += "\"";
        return result;
    }

    inline std::string to_json(const std::string_view &value) {
        return to_json(std::string(value));
    }

    inline std::string to_json(const char &value) {
        return to_json(std::string(1, value));
    }

    template <typename T>
    std::string to_json(const std::vector<T> &vec) {
        std::string result = "[";
        bool first = true;
        for (const auto &v : vec) {
            if (!first)
                result += ", ";
            first = false;
            result += to_json(v);
        }
        result += "]";
        return result;
    }

    template <typename T>
    std::string to_json(const std::set<T> &vec) {
        std::string result = "[";
        bool first = true;
        for (const auto &v : vec) {
            if (!first)
                result += ", ";
            first = false;
            result += to_json(v);
        }
        result += "]";
        return result;
    }

    template <typename T, std::size_t U>
    std::string to_json(const std::array<T, U> &arr) {
        std::string result = "[";
        bool first = true;
        for (const auto &a : arr) {
            if (!first) {
                result += ", ";
            }
            first = false;
            result += to_json(a);
        }
        return result += "]";
    }

    template <typename T>
    std::string to_json(const std::unique_ptr<T> &ptr) {
        if (!ptr)
            return "null";
        return to_json(*ptr);
    }

    template <typename T>
    std::string to_json(const std::shared_ptr<T> &ptr) {
        if (!ptr)
            return "null";
        return to_json(*ptr);
    }

    template <typename T>
    std::string to_json(const std::weak_ptr<T> &p) {
        if (auto sp = p.lock()) {
            return to_json(*sp);
        }
        return "null";
    }

    template <typename T, typename U>
    std::string to_json(const std::map<T, U> &map) {
        std::string result;
        bool first = true;
        result += "{";
        for (const auto &[Key, Value] : map) {
            if (!first)
                result += ", ";
            first = false;
            result += to_json(Key);
            result += ": ";
            result += to_json(Value);
        }

        result += "}";
        return result;
    }

    template <typename T, typename U>
    std::string to_json(const std::pair<T, U> &pair) {
        std::string result = "[";

        result += to_json(pair.first);
        result += ", ";
        result += to_json(pair.second);

        return result += "]";
    }

    template <typename T>
        requires std::is_enum_v<T>
    std::string to_json(T value) {
        static constexpr auto enumerators = std::define_static_array(
            std::meta::enumerators_of(^^T));

        template for (constexpr auto e : enumerators) {
            if (value == [:e:]) {
                return std::format("\"{}\"", std::meta::identifier_of(e));
            }
        }
        return "null";
    }

    template <typename T>
        requires(!std::is_enum_v<T>)
    std::string to_json(const T &value) {
        static constexpr auto members = std::define_static_array(
            std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::unchecked()));

        std::string result = "{";
        bool first = true;

        template for (constexpr auto m : members) {
            if (!first)
                result += ", ";
            first = false;
            result += std::format("\"{}\": ", std::meta::identifier_of(m));
            result += to_json(value.[:m:]);
        }

        result += "}";
        return result;
    }

} // namespace hp::ser