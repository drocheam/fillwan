
#include "definitions.hpp"

#include "words_de.hpp"
#include "words_en.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>
#include <iostream>
#include <iomanip>
#include <array>


void fillw::getHelp() 
{
	using std::wcout, std::left, std::setw;

	wcout
	<< "usage: fillwan [options] [file]\n" 
	<< "reads from standard input if no file is provided\n\n"
	<< "options:\n" << left
	<< setw(25) << "-h | --help" 		   << "display this help\n"
	<< setw(25) << "-l | --lang (EN | DE)" << "language selection. English or German. Default: EN\n"
	<< setw(25) << "-c | --color"  		   << "enable colored output\n"
	<< setw(25) << "-d | --dump"	 	   << "dump text with highlighted fill expressions\n"
	<< setw(25) << "-a | --alpha "		   << "sort occurrence map alphabetically, not by count\n";
}


void fillw::setOptions(int argc, char** argv, options &opt) 
{
	// we could use getopt.h for POSIX systems instead, but this would made it platform dependent

	std::string lang, par;

	// default parameters
	opt.dump 		= false;
	opt.sort_occur 	= true;
	opt.color 		= false;
	opt.help_only	= false;
	opt.path		= "";
	opt.word_list 	= &word_list_en;

	std::vector<std::string> args;

	// this parts creates the args vector and expands parameters like -sc to -s -c
	for (int i = 1; i < argc; i++)
	{
		par = std::string(argv[i]);

		if (par.at(0) == '-' && par.length() > 2 && par.at(1) != '-')
			for (size_t j = 1; j < par.length(); j++)
				args.push_back(std::string("-") + par.at(j));
		else
			args.push_back(par);
	}

	// set options depending on args
	for (size_t i = 0; i < args.size(); i++)
	{
		par = args.at(i);

		if (par == "--alpha" || par == "-a")
			opt.sort_occur = false;

		else if (par == "--dump" || par == "-d")
			opt.dump = true;
		
		else if (par == "--color" || par == "-c")
			opt.color = true;

		else if (par == "--help" || par == "-h")
		{
			fillw::getHelp();
			opt.help_only = true;
		}
		else if (par == "--lang" || par == "-l")
		{
			if (i == size_t(args.size() - 1))
				throw std::runtime_error("Language parameter missing.");

			lang = args.at(++i);

			// enfore lower case
			std::transform(lang.begin(), lang.end(), lang.begin(), [](auto c){ return std::towlower(c); });

			if (lang == "de")			opt.word_list = &fillw::word_list_de;
			else if (lang == "en")		opt.word_list = &fillw::word_list_en;
			else						throw std::invalid_argument("Invalid language '" + lang + "'.");
		}
		else if (i == args.size() - 1)
			opt.path = args.at(i);
		else
			throw std::invalid_argument(par);
	}

	// assign output sequences
	HIGHLIGHT_SEQ_CLOSE = (opt.color)?  RESET_SEQ_ANSI: 		RESET_SEQ_NON_ANSI;
	HIGHLIGHT_SEQ_OPEN 	= (opt.color)?  HIGHLIGHT_SEQ_ANSI: 	HIGHLIGHT_SEQ_NON_ANSI;
	SIGNAL_SEQ_OPEN 	= (opt.color)?  SIGNAL_SEQ_ANSI_OPEN: 	SIGNAL_SEQ_NON_ANSI_OPEN;
	SIGNAL_SEQ_CLOSE    = (opt.color)?  SIGNAL_SEQ_ANSI_CLOSE: 	SIGNAL_SEQ_NON_ANSI_CLOSE;

}


void fillw::getOccurrences(std::wstring_view data, const fillw::options &opt,
						   fillw::statistics &stats, std::wostringstream &sout)
{
	using std::wstring, std::find_if, std::find_if_not, std::reverse_iterator;
				
	bool prem_break; // premature break, punctuation mark ends the sentence
	bool succ;

	size_t ws_e, start_wd, end_wd;

	std::vector<size_t> ws_pos;
	std::wstring_view sent, word;
	std::wstring word_s;

	// pre-reserve for speedup. a whitespace every 5 chars is a guess
	ws_pos.reserve(size_t(data.length()/5));

	if (!std::iswspace(data.at(0)))
		ws_pos.push_back(0);

    for(size_t pos = 0; pos < data.length(); pos++)
	{
		pos = std::find_if(data.begin()+pos, data.end(), std::iswspace) - data.begin();
		ws_pos.push_back(pos);
		pos = std::find_if_not(data.begin()+pos, data.end(), std::iswspace) - data.begin();
	}

	if (!std::iswspace(data.at(data.length()-1)))
		ws_pos.push_back(data.length());
	
	// init stats
	stats.fill_count = 0; 
	stats.word_count = 0;

	for(size_t i = 0; i < ws_pos.size()-1; i++)
	{
		prem_break = false;
		succ = false;

		ws_e = std::find_if_not(data.begin()+ws_pos.at(i), data.end(), std::iswspace) - data.begin();
		start_wd = std::find_if_not(data.begin()+ws_e, data.begin()+ws_pos.at(i+1), ::iswpunct) - data.begin();

		if (start_wd < ws_pos.at(i+1))
		{
			for(size_t j = 0; j < opt.word_list->size(); j++)
			{
				if (i+j+1 >= ws_pos.size() || prem_break)
					break;

				word = data.substr(start_wd, ws_pos.at(i+j+1)-start_wd);

				// exclude punctuation marks at the end of the word
				auto end_it = find_if_not(reverse_iterator(word.end()), 
										  reverse_iterator(word.begin()), ::iswpunct);
				end_wd = word.length() - (end_it - reverse_iterator(word.end()));
			
				if (end_wd != word.length()){
					// exclude mark, exit loop on next iteration
					word = word.substr(0, end_wd);
					prem_break = true;
				}
				else if (end_wd == 0)
					break;

				word_s = std::wstring(word);
				
				if (j > 0)
				{
					// convert whitespaces to space, remove multiple ones
					std::replace_if(word_s.begin(), word_s.end(), ::iswspace, L' ');
					auto I = unique(word_s.begin(), word_s.end(), 
									[](auto& lhs, auto& rhs){ return lhs == L' ' && lhs == rhs; } );
					word_s.erase(I, word_s.end());
				}

				auto &wn = opt.word_list->at(j);
				auto it = wn.find(word_s);

				// first word iteration, add word count
				// on the way we excluded cases where there are only punctuation marks between whitespaes
				if (j == 0) 
					stats.word_count++;

				if (it != wn.end())
				{
					if (opt.dump)
						sout << data.substr(ws_pos.at(i), start_wd-ws_pos.at(i))
							 << SIGNAL_SEQ_OPEN 
							 << data.substr(start_wd, end_wd)
							 << SIGNAL_SEQ_CLOSE 
							 << data.substr(start_wd + end_wd, ws_pos.at(i+j+1)-(start_wd+end_wd));
					else
						// increments the element and inserts one if needed
						stats.occurrences[*it]++;

					i += j; // skip search for next words, since they already belong 
							// to a fill word expression
					stats.fill_count++;
					stats.word_count += j; // add skipped words to word count
					succ = true;
					break;
				}
			}
		}
		if (!succ && opt.dump)
			sout << data.substr(ws_pos.at(i), ws_pos.at(i+1)-ws_pos.at(i)); 
	}
	if (opt.dump)
		sout << data.substr(ws_pos.at(ws_pos.size()-2), data.length()); 
}


int fillw::getText(const options &opt, std::wstring &text)
{
	// exit if not connected to a pipe
	if (opt.path == "" && !fillw::inPipe())
	{
		std::wcerr << "Pipe text into this program or specify a file path." << std::endl;
		return 1;
	}
	else
	{
		FILE * file;

		if (opt.path != "")
		{
 			file =  fopen(opt.path.c_str(), "r");
			if (file == NULL)
			{
				std::wcerr << "File Not Found";
				return -1;
			}
		}
		else
			file = stdin;

		// buffered read is much faster than getline etc.
		std::array<wchar_t, 2 << 12> buffer;
		while(fgetws(buffer.data(), buffer.size(), file) != NULL)
			text.append(buffer.data());

		if (text.length() == 0)
		{
			std::wcerr << "No text received." << std::endl;
			return 1;
		}
	}
	return 0;
}


size_t fillw::getLineCount(std::wstring_view data)
{
	size_t count = 0;
	
	for(auto &c : data)
		if (c == L'\n')
			count++;

	return (count)? count: count + 1;
}


size_t fillw::getSentenceCount(std::wstring_view data)
{
	size_t pos=0, count=0;

	while (pos < data.length())
	{
		if (pos > 0)
			pos = std::find_if(data.begin()+pos, data.end(), std::iswspace) - data.begin();
		pos = std::find_if(data.begin()+pos, data.end(), std::iswalnum) - data.begin();

		if (pos < data.length())
		{
			pos = data.find_first_of(punctuation_sent, pos);
			count++;
		}
	}
	return count;
}

