#ifndef JTS2GD_UTILS
#define JTS2GD_UTILS


// built-in
#include <cstdint>
#include <sstream>
#include <iostream>
#include <cstdint>


struct SourceLocation
{
    const std::string* file_name;
    uint32_t line;
    uint32_t collum;

    template <class STREAM>
    friend STREAM& operator<<(STREAM& stream, const SourceLocation& sl)
    {
        stream << *sl.file_name << ':' << sl.line << ':' << sl.collum;
        return stream;
    }
};


enum class Color
{
    FG_DEFAULT        = 39,
    FG_BLACK          = 30,
    FG_RED            = 31,
    FG_GREEN          = 32,
    FG_YELLOW         = 33,
    FG_BLUE           = 34,
    FG_MAGENTA        = 35,
    FG_CYAN           = 36,
    FG_LIGHT_GRAY     = 37,
    FG_DARK_GRAY      = 90,
    FG_LIGHT_RED      = 91,
    FG_LIGHT_GREEN    = 92,
    FG_LIGHT_YELLOW   = 93,
    FG_LIGHT_BLUE     = 94,
    FG_LIGHT_MAGENTA  = 95,
    FG_LIGHT_CYAN     = 96,
    FG_WHITE          = 97,

    BG_RED            = 41,
    BG_GREEN          = 42,
    BG_BLUE           = 44,
    BG_DEFAULT        = 49
};


inline std::string get_color(Color md)
{
    #ifndef _WIN32
        std::string out {"\033["};
        out.append(std::to_string((uint32_t)md));
        out.append("m");
        return out;
    #else
        return "";
    #endif
}

[[noreturn]]
inline void panic(const std::string& msg)
{
    std::cout << '[' 
              << get_color(Color::FG_LIGHT_RED) << "ERROR" << get_color(Color::FG_DEFAULT)
              << "]: "
              <<  msg << std::endl;
    
    exit(1);
}

#endif