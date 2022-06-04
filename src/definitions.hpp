#ifndef DEFINITIONS_HPP
#define DEFINITIONS_HPP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <sstream>


// Definitions
//
// Line: Includes newline character. 
// Last line of file is excluded if it consists only of the \n character.
//
// Sentence: At least one whitespace + punctuation mark.
// Special case at end of stream: character that is not whitespace 
// or punctuation mark + end of file/stream
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

constexpr std::wstring_view punctuation_sent = L".:;?!¿⁇⁉⁈‽⸘؟·჻";


// global variables for holding the color formatting strings
// (which strings are stored here is determined at runtime via program parameter)
inline std::wstring HIGHLIGHT_SEQ_CLOSE; 
inline std::wstring HIGHLIGHT_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_OPEN;
inline std::wstring SIGNAL_SEQ_CLOSE;




namespace fillw
{


	//// typedefs
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	// comparator for case independent comparison
	inline auto caseless_cmp = [](const std::wstring& lhs, const std::wstring &rhs) -> bool
	{
		for(size_t i = 0; i < lhs.length() && i < rhs.length(); i++)
			if(std::towlower(lhs[i]) != std::towlower(rhs[i]))
				return std::towlower(lhs[i]) < std::towlower(rhs[i]);

		return (lhs.length() < rhs.length());
	};

	typedef decltype(caseless_cmp) cmp_decl;

	typedef std::vector<std::pair<std::wstring, size_t>> 			output_map_type;
	typedef std::map<std::wstring, size_t, cmp_decl> 				occur_map_type;
	typedef const std::vector<std::set<std::wstring, cmp_decl>> 	word_list_type;


	//// Structs
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	struct options
	{
		bool print_sent,
			 sort_occur,
			 color,
			 help_only;

		fillw::word_list_type *word_list;
	};

	struct statistics
	{
		size_t fill_count,
			   word_count,
			   sentence_num,
			   org_length,
			   lines;

		fillw::occur_map_type occurrences;

		std::vector<size_t> sentence_lines;
	};


	struct sentence_output
	{
		bool active;
		std::wostringstream sentences;
	};


	//// Prototypes
	///////////////////////////////////////////////////////////////////////////////////////////////
	
	void getHelp();
	void setOptions(int argc, char** argv, fillw::options &opt);
	
	void getSentenceLines(const std::wstring &data, fillw::statistics &stats);


	void getOccurrences(const std::wstring &data, 
						fillw::word_list_type *word_list,
						fillw::statistics &stats,
						fillw::sentence_output &sout);


}



#endif
