//
// -- console_colors.h simple encapsulation of ASCI color codes for console output
//
//
// -- useage
//
//using CC = ConsoleColor;
//std::cout << CC::RED << "red text" << CC::RESET << std::endl;
//std::cout << CC::BRIGHT_BLUE << "blue text" << CC::RESET << std::endl;
// can be turned off at compile time by defining NO_CONSOLE_COLORS
//
// see also helper functionst end
//
#ifndef CONSOLE_COLOR_H
#define CONSOLE_COLOR_H

#ifdef NO_CONSOLE_COLORS
    #define ESC ""
    #define COLOR(code) ""
#else
    #define ESC "\033["
    #define COLOR(code) ESC code "m"
#endif

class ConsoleColor {
public:
    // Reset
    static constexpr const char* RESET = COLOR("0");

    // Styles
    static constexpr const char* BOLD      = COLOR("1");
    static constexpr const char* UNDERLINE = COLOR("4");
    static constexpr const char* INVERSE   = COLOR("7");

    // Foreground colors
    static constexpr const char* BLACK   = COLOR("30");
    static constexpr const char* RED     = COLOR("31");
    static constexpr const char* GREEN   = COLOR("32");
    static constexpr const char* YELLOW  = COLOR("33");
    static constexpr const char* BLUE    = COLOR("34");
    static constexpr const char* MAGENTA = COLOR("35");
    static constexpr const char* CYAN    = COLOR("36");
    static constexpr const char* WHITE   = COLOR("37");

    // Bright foreground colors
    static constexpr const char* BRIGHT_BLACK   = COLOR("90");
    static constexpr const char* BRIGHT_RED     = COLOR("91");
    static constexpr const char* BRIGHT_GREEN   = COLOR("92");
    static constexpr const char* BRIGHT_YELLOW  = COLOR("93");
    static constexpr const char* BRIGHT_BLUE    = COLOR("94");
    static constexpr const char* BRIGHT_MAGENTA = COLOR("95");
    static constexpr const char* BRIGHT_CYAN    = COLOR("96");
    static constexpr const char* BRIGHT_WHITE   = COLOR("97");

    // Background colors
    static constexpr const char* BG_BLACK   = COLOR("40");
    static constexpr const char* BG_RED     = COLOR("41");
    static constexpr const char* BG_GREEN   = COLOR("42");
    static constexpr const char* BG_YELLOW  = COLOR("43");
    static constexpr const char* BG_BLUE    = COLOR("44");
    static constexpr const char* BG_MAGENTA = COLOR("45");
    static constexpr const char* BG_CYAN    = COLOR("46");
    static constexpr const char* BG_WHITE   = COLOR("47");

    // Bright background colors
    static constexpr const char* BG_BRIGHT_BLACK   = COLOR("100");
    static constexpr const char* BG_BRIGHT_RED     = COLOR("101");
    static constexpr const char* BG_BRIGHT_GREEN   = COLOR("102");
    static constexpr const char* BG_BRIGHT_YELLOW  = COLOR("103");
    static constexpr const char* BG_BRIGHT_BLUE    = COLOR("104");
    static constexpr const char* BG_BRIGHT_MAGENTA = COLOR("105");
    static constexpr const char* BG_BRIGHT_CYAN    = COLOR("106");
    static constexpr const char* BG_BRIGHT_WHITE   = COLOR("107");

    // Composite styles
    static constexpr const char* BOLD_RED         = COLOR("1;31");
    static constexpr const char* UNDERLINE_BLUE   = COLOR("4;34");

    static inline std::string Paint(const std::string& text, const char* color = nullptr)
    {
        if (color == nullptr || *color == '\0') { return text; }
        return std::string(color) + text + RESET;
    }
    static inline std::string Paint(const std::string& text,const std::string& color)
    {
        if (color.empty()) {
            return text;
        }
        return color + text + RESET;
    }
    static inline std::string ForegroundRGB(int red, int green, int blue)
    {
    #ifdef NO_CONSOLE_COLORS
        return "";
    #else
        return "\033[38;2;" +
            std::to_string(red) + ";" +
            std::to_string(green) + ";" +
            std::to_string(blue) + "m";
    #endif
    }
};
//
// -- Global GLE specific colors
//
const std::string InputFileColor      = ConsoleColor::BRIGHT_BLUE;
const std::string OutputFileColor     = ConsoleColor::BRIGHT_GREEN;
const std::string GLECodeColor        = ConsoleColor::BRIGHT_GREEN;
const std::string LineNumberColor     = ConsoleColor::BRIGHT_CYAN;
const std::string ErrorMessageColor   = ConsoleColor::BRIGHT_RED;
const std::string WarningMessageColor = ConsoleColor::BRIGHT_YELLOW;

const std::string GLELogoColor =
    std::string(ConsoleColor::BRIGHT_RED) + "G" +
    std::string(ConsoleColor::BRIGHT_GREEN) + "L" +
    std::string(ConsoleColor::BRIGHT_BLUE) + "E" +
    std::string(ConsoleColor::RESET);

// helper functions
inline std::string ColorInputFile(const std::string& s) {
    return ConsoleColor::Paint(s, InputFileColor);
}
inline std::string ColorOutputFile(const std::string& s) {
    return ConsoleColor::Paint(s, OutputFileColor);
}
inline std::string ColorGLECode(const std::string& s) {
    return ConsoleColor::Paint(s, GLECodeColor);
}
inline std::string ColorLineNumber(int i) {
    return ConsoleColor::Paint(std::to_string(i), LineNumberColor);
}
inline std::string ColorErrorMessage(const std::string& s) {
    return ConsoleColor::Paint(s, ErrorMessageColor);
}
inline std::string ColorWarningMessage(const std::string& s) {
    return ConsoleColor::Paint(s, WarningMessageColor);
}
inline const char* GetConsoleColor(const std::string& color)
{
    // shdoul support all teh colors GLE supports ...
    if (color == "BLACK")          return ConsoleColor::BLACK;
    if (color == "RED")            return ConsoleColor::RED;
    if (color == "GREEN")          return ConsoleColor::GREEN;
    if (color == "YELLOW")         return ConsoleColor::YELLOW;
    if (color == "BLUE")           return ConsoleColor::BLUE;
    if (color == "MAGENTA")        return ConsoleColor::MAGENTA;
    if (color == "CYAN")           return ConsoleColor::CYAN;
    if (color == "WHITE")          return ConsoleColor::WHITE;

    if (color == "BRIGHT_BLACK")   return ConsoleColor::BRIGHT_BLACK;
    if (color == "BRIGHT_RED")     return ConsoleColor::BRIGHT_RED;
    if (color == "BRIGHT_GREEN")   return ConsoleColor::BRIGHT_GREEN;
    if (color == "BRIGHT_YELLOW")  return ConsoleColor::BRIGHT_YELLOW;
    if (color == "BRIGHT_BLUE")    return ConsoleColor::BRIGHT_BLUE;
    if (color == "BRIGHT_MAGENTA") return ConsoleColor::BRIGHT_MAGENTA;
    if (color == "BRIGHT_CYAN")    return ConsoleColor::BRIGHT_CYAN;
    if (color == "BRIGHT_WHITE")   return ConsoleColor::BRIGHT_WHITE;

    return nullptr;
}

#endif