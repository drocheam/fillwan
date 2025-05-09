#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

// strings
#include <string>  // std::wstring
#include <string_view>  // std::wstring_view
#include <sstream>  // std::wostringstream
#include <cwctype> // std::towlower

// other types
#include <unordered_map>  // std::unordered_set
#include <unordered_set>  // std::unordered_set
#include <vector>  // std::vector
#include <utility>  // std::pair
#include <cstddef> // size_t

// functionality
#include <algorithm>  // std::transform, std::equal
#include <functional>  // std::hash

// needed for inPipe below
#if defined(_WIN32) || defined(WIN32) // Windows
    #include <io.h>  // _isatty
    #include <stdio.h>  // _fileno
#else // UNIX
    #include <unistd.h>  // isatty
    #include <cstdio>  // fileno
#endif


// Definitions
///////////////////////////////////////////////////////////////////////////////////////////////
//
// Line: 
// Includes a newline character. 
// Last line of file is excluded if it consists only of the \n character.
//
// Word:
// one or more alphanumerical characters between punctuation marks or whitespaces
// 

// Formatting Strings
///////////////////////////////////////////////////////////////////////////////////////////////

// highlighting sequences (emphasizing things)
constexpr std::wstring_view HIGHLIGHT_SEQ_NON_ANSI =    L"";
constexpr std::wstring_view HIGHLIGHT_SEQ_ANSI =        L"\033[1m";
constexpr std::wstring_view RESET_SEQ_NON_ANSI =        L"";
constexpr std::wstring_view RESET_SEQ_ANSI =            L"\033[0m";

// signaling sequences (marking fill expressions)
constexpr std::wstring_view SIGNAL_SEQ_NON_ANSI_OPEN =    L"<<";
constexpr std::wstring_view SIGNAL_SEQ_NON_ANSI_CLOSE =   L">>";
constexpr std::wstring_view SIGNAL_SEQ_ANSI_OPEN =        L"\033[1;31m";
constexpr std::wstring_view SIGNAL_SEQ_ANSI_CLOSE =       L"\033[0m";

// visual separator string
constexpr std::wstring_view SEPARATOR = L"--------------------------------\n";

// punctuation for separating sentences:
constexpr std::wstring_view punctuation_sent = L".:;?!¿⁇⁉⁈‽⸘";

// global variables for holding the color formatting strings
// (which strings are stored here is determined at runtime by the program's parameters)
inline std::wstring HIGHLIGHT_SEQ_CLOSE; 
inline std::wstring HIGHLIGHT_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_CLOSE;


namespace fillw
{
    //// typedefs
    ///////////////////////////////////////////////////////////////////////////////////////////////

    // comparator for case-independent comparison
    inline auto caseless_less = [](const std::wstring_view& lhs, const std::wstring_view &rhs) -> bool
    {
        for(size_t i = 0; i < lhs.length() && i < rhs.length(); i++)
            if (std::towlower(lhs.at(i)) != std::towlower(rhs.at(i)))
                return std::towlower(lhs.at(i)) < std::towlower(rhs.at(i));

        return (lhs.length() < rhs.length());
    };

    // hashing function for case-independent comparison
    inline auto caseless_hash = [](const std::wstring_view &expr) -> size_t
    {
        std::wstring expr2(expr);
        std::transform(expr.begin(), expr.end(), expr2.begin(), std::towlower);

        return std::hash<std::wstring>()(expr2);
    };

    // equality function for case-independent comparison
    inline auto caseless_equal = [](const std::wstring_view& lhs, const std::wstring_view& rhs) -> bool
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
                          [](auto & a, auto & b) { return std::towlower(a) == std::towlower(b);} );
    };

    
    // type for word lists is a vector of sets. The first vector element holds all expressions with one word, 
    // the second with two and so on. Each element is an unordered set with wstrings
    // a set has an average constant time complexity for search, removal and insertion, 
    // making it suitable for our purpose
    // non-default hashing and comparator function to enable caseless comparisons
    typedef const std::vector<std::unordered_set<std::wstring, 
                                                 decltype(caseless_hash),
                                                 decltype(caseless_equal)>>     word_list_type;
    
    // the occurrence map is an unordered map of fill expression strings and numeric counts
    // custom hashing and comparator for caseless comparison
    // Average constant time complexity for search, removal and insertion
    // so searching and incrementing the count when updating the fill words should be fast
    typedef std::unordered_map<std::wstring_view, 
                               size_t, 
                               decltype(caseless_hash),
                               decltype(caseless_equal)>                        occur_map_type;
    
    // output vector consists of a pair of fill expression and count
    typedef std::vector<std::pair<std::wstring_view, size_t>>                   output_map_type;


    // inPipe Detection Function
    // needed to figure out if the program should read from file/parameter or pipe
    ///////////////////////////////////////////////////////////////////////////////////////////////

    #if defined(_WIN32) || defined(WIN32) // Windows 
        inline auto inPipe = []() -> bool { return _isatty(_fileno(stdin)) == 0; };
    #else  // UNIX
        inline auto inPipe = []() -> bool { return isatty(fileno(stdin)) == 0; };
    #endif

    //// Structs
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    // program options
    struct options
    {
        bool dump,
             sort_occur,
             color,
             help_only;
        
        std::string path;
        word_list_type *word_list;
    };

    // text statistics and fill words
    struct statistics
    {
        size_t fill_count,
               word_count,
               length,
               lines;

        occur_map_type occurrences;
    };

    //// Function Prototypes
    ///////////////////////////////////////////////////////////////////////////////////////////////
    
    void getHelp();

    void setOptions(int argc, char** argv, options &opt);
    
    int getText(const options &opt, std::wstring &text);
    
    size_t getLineCount(std::wstring_view data);

    void getOccurrences(std::wstring_view data, 
                        const options &opt,
                        statistics &stats,
                        std::wostringstream &sout);
}


#endif
