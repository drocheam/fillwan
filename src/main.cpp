
#include "definitions.hpp"

#include <algorithm>
#include <string>
#include <iostream>
#include <iomanip> 
#include <locale>


int main(int argc, char** argv)
{
	// initialization /////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	std::setlocale(LC_ALL, "");
	
	fillw::options 			opt;
	fillw::statistics 		stats;
	fillw::output_map_type  output_map;

	std::wostringstream sout;
	std::wstring text;
	int status;

	// set options from cmd parameters
	fillw::setOptions(argc, argv, opt);

	if (opt.help_only)
		return 0;

	// processing /////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////

	status = fillw::getText(opt, text);

	if (status)
		return status;
	
	stats.length = text.length();
	stats.sentences = fillw::getSentenceCount(text);
	stats.lines = fillw::getLineCount(text);

	fillw::getOccurrences(text, opt, stats, sout);

	// assign elements to vector
	for (auto& it : stats.occurrences)
		output_map.emplace_back(it);
	  
	//// sort occurrence descending, but same number of occurrences alphabetically
	if (opt.sort_occur)
		sort(output_map.begin(), output_map.end(), [](auto &a, auto &b)
				     { return (a.second != b.second)? a.second > b.second: a.first < b.first; });
	else
		sort(output_map.begin(), output_map.end(), [](auto &a, auto &b) 
												   {return fillw::caseless_less(a.first, b.first);});


	// output /////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	// function for header printing
	auto make_header = [](std::wstring a){
		return L"\n" + HIGHLIGHT_SEQ_OPEN + a + HIGHLIGHT_SEQ_CLOSE + L"\n" 
			   														+ std::wstring(SEPARATOR);
	};

	// newline function for occurrence table
	auto occur_newline = [&](size_t pos){
		return (pos % 3 == 0 || pos == output_map.size())? L"\n" : L"   ";
	};

	if (opt.dump)
	{
		std::wcout << sout.str();
		return 0;
	}

	// show statistics
	std::wcout << make_header(L"Statistics")
			   << stats.length        << " characters, " 
			   << stats.word_count    << " words, " 
			   << stats.fill_count    << " fill expressions\n"
			   << stats.sentences     << " sentences, " 
			   << stats.lines 	      << " lines\n\n\n";

	if (output_map.size())
	{
		std::wcout << make_header(L"Fill Expression Occurrences");
		for (size_t pos = 0; auto &el : output_map)
			std::wcout << std::left  << std::setw(24) << (el.first + L":") 
					   << std::right << std::setw(5)  << el.second 
					   << occur_newline(++pos);
	}
	else
		std::wcout << HIGHLIGHT_SEQ_OPEN + L"No Fill Expressions Found." + HIGHLIGHT_SEQ_CLOSE 
				   << std::endl << std::endl;

	return 0;
}
