
#include "definitions.hpp"

#include <algorithm>
#include <string>
#include <iostream>
#include <iomanip> 
#include <locale>



int main(int argc, char** argv)
{

	// initalization //////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	std::setlocale(LC_ALL, "");
	
	fillw::options 			opt;
	fillw::statistics 		stats;
	fillw::sentence_output  sout;
	fillw::output_map_type  output_map;

	std::wstring data;

	// set options from cmd parameters
	fillw::setOptions(argc, argv, opt);

	if (opt.help_only)
		return 0;

	// assign output sequences
	HIGHLIGHT_SEQ_CLOSE = (opt.color)?  RESET_SEQ_ANSI: 		RESET_SEQ_NON_ANSI;
	HIGHLIGHT_SEQ_OPEN 	= (opt.color)?  HIGHLIGHT_SEQ_ANSI: 	HIGHLIGHT_SEQ_NON_ANSI;
	SIGNAL_SEQ_OPEN 	= (opt.color)?  SIGNAL_SEQ_ANSI_OPEN: 	SIGNAL_SEQ_NON_ANSI_OPEN;
	SIGNAL_SEQ_CLOSE    = (opt.color)?  SIGNAL_SEQ_ANSI_CLOSE: 	SIGNAL_SEQ_NON_ANSI_CLOSE;

	sout.active = opt.print_sent;

	// processing /////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	// read from pipe
	for (std::wstring line; std::getline(std::wcin, line);)
		data += line + L"\n";
	
	stats.org_length = data.length();

	// get position of sentences
	fillw::getSentenceLines(data, stats);

	// convert whitespace characters to space
	std::replace_if(data.begin(), data.end(), ::iswspace, L' ');

	// remove multiple whitespaces 
	auto I = unique(data.begin(), data.end(), 
					[](auto lhs, auto rhs){ return lhs == L' ' && lhs == rhs; } );
	data.erase(I, data.end());

	// get expression occurrences
	// add leading and trailing space for word beginning/ending
	if (data.front() != L' ')
		data.insert(0, L" ");

	if (data.back() != L' ')
		data.append(L" ");
	
	fillw::getOccurrences(data, opt.word_list, stats, sout);

	// assign elements to vector
	for (auto& it : stats.occurrences)
		output_map.emplace_back(it);
	  
	//// Sort occurrence descending, but same number of occurrences alphabetically
	if (opt.sort_occur)
		sort(output_map.begin(), output_map.end(), [](auto &a, auto &b) 
				{return (a.second != b.second)? a.second > b.second: a.first < b.first;});

	// output /////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////
	
	// function for header printing
	auto make_header = [](std::wstring a){
		return std::wstring(L"\n") + std::wstring(HIGHLIGHT_SEQ_OPEN) + a 
						+ std::wstring(HIGHLIGHT_SEQ_CLOSE) + std::wstring(L"\n") 
						+ std::wstring(SEPARATOR);
	};

	// newline function for occurrence table
	auto occur_newline = [&](size_t pos){
		return (pos % 3 == 0 || pos == output_map.size())? L"\n" : L"   ";
	};

	// show statistics
	std::wcout 
	<< make_header(L"Statistics")
	<< stats.org_length 	<< " characters, " 
	<< stats.word_count 	<< " words, " 
	<< stats.fill_count 	<< " fill expressions\n"
	<< stats.sentence_num 	<< " sentences, " 
	<< stats.lines 			<< " lines\n\n\n";

	// word occurrences
	std::wcout << make_header(L"Fill Expression Occurrences");
	for (size_t pos = 0; auto &el : output_map)
		std::wcout << std::left  << std::setw(24) << (el.first + L":") 
				   << std::right << std::setw(5)  << el.second 
				   << occur_newline(++pos);

	// Sentence List
	if (opt.print_sent)
		std::wcout << make_header(L"\n\nSentence List") << sout.sentences.str();
	else
		std::wcout << "\n\nUse parameter -s for a sentence list." << std::endl;

	return 0;
}

