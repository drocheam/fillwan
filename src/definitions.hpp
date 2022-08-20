#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <algorithm>


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

constexpr std::wstring_view HIGHLIGHT_SEQ_NON_ANSI = 	L"";
constexpr std::wstring_view HIGHLIGHT_SEQ_ANSI = 		L"\033[1m";
constexpr std::wstring_view RESET_SEQ_NON_ANSI = 		L"";
constexpr std::wstring_view RESET_SEQ_ANSI =	 		L"\033[0m";

constexpr std::wstring_view SIGNAL_SEQ_NON_ANSI_OPEN = 	L"<<";
constexpr std::wstring_view SIGNAL_SEQ_NON_ANSI_CLOSE = L">>";
constexpr std::wstring_view SIGNAL_SEQ_ANSI_OPEN = 		L"\033[1;31m";
constexpr std::wstring_view SIGNAL_SEQ_ANSI_CLOSE = 	L"\033[0m";

constexpr std::wstring_view SEPARATOR = L"--------------------------------\n";

constexpr std::wstring_view punctuation_sent = L".:;?!¿⁇⁉⁈‽⸘";


// global variables for holding the color formatting strings
// (which strings are stored here is determined at runtime via program parameter)
inline std::wstring HIGHLIGHT_SEQ_CLOSE; 
inline std::wstring HIGHLIGHT_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_CLOSE;


// needed for inPipe below
#ifdef __unix__         
	#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32) 
	#include <io.h>
#endif


namespace fillw
{

	//// typedefs
	///////////////////////////////////////////////////////////////////////////////////////////////

	// slightly faster than std::towlower
	inline auto towlower = [](wchar_t c) -> wchar_t 
	{ 
		if (c > 64 && c < 91) // ASCII capitalized letters 
			return c + 32;
		else if (c > 127) // outside ASCII
			return std::towlower(c);
		else // inside ASCII, but no capitalized letter
			return c;
	};

	 //comparator for case independent comparison
	inline auto caseless_less = [](const std::wstring_view& lhs, const std::wstring_view &rhs) -> bool
	{
		for(size_t i = 0; i < lhs.length() && i < rhs.length(); i++)
			if (fillw::towlower(lhs.at(i)) != fillw::towlower(rhs.at(i)))
				return fillw::towlower(lhs.at(i)) < fillw::towlower(rhs.at(i));

		return (lhs.length() < rhs.length());
	};

	inline auto caseless_hash = [](const std::wstring_view &expr) -> size_t
	{
		std::wstring expr2(expr);
		std::transform(expr.begin(), expr.end(), expr2.begin(), fillw::towlower);

		return std::hash<std::wstring>()(expr2);
	};

	inline auto caseless_equal = []( const std::wstring_view& lhs, const std::wstring_view& rhs) -> bool
	{
		return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end(),
						  [](auto & a, auto & b) { return fillw::towlower(a) == fillw::towlower(b);} );
	};

	typedef std::vector<std::pair<std::wstring_view, size_t>> 					output_map_type;
	
	typedef std::unordered_map<std::wstring_view, 
							   size_t, 
							   decltype(caseless_hash),
							   decltype(caseless_equal)> 						occur_map_type;

	typedef const std::vector<std::unordered_set<std::wstring, 
												 decltype(caseless_hash),
												 decltype(caseless_equal)>> 	word_list_type;


	// InPipe Detection Function
	///////////////////////////////////////////////////////////////////////////////////////////////

	#ifdef __unix__         
		inline auto inPipe = []() -> bool { return !isatty(fileno(stdin)); };
	#elif defined(_WIN32) || defined(WIN32) 
		inline auto inPipe = []() -> bool { return !_isatty(_fileno(stdin)); };
	#endif

	//// Structs
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	struct options
	{
		bool dump,
			 sort_occur,
			 color,
			 help_only;
		
		std::string path;
		fillw::word_list_type *word_list;
	};

	struct statistics
	{
		size_t fill_count,
			   word_count,
			   length,
			   lines;

		fillw::occur_map_type occurrences;
	};

	//// Prototypes
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	void getHelp();

	void setOptions(int argc, char** argv, fillw::options &opt);
	
	int getText(const options &opt, std::wstring &text);
	
	size_t getLineCount(std::wstring_view data);

	void getOccurrences(std::wstring_view data, 
						const fillw::options &opt,
						fillw::statistics &stats,
						std::wostringstream &sout);

}


#endif
