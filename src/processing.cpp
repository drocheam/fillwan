
#include "definitions.hpp"

#include "words_de.hpp"
#include "words_en.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <getopt.h>



void fillw::getHelp() 
{
	using std::wcout, std::left, std::setw;

	wcout
	<< "usage: fillwan [options]\n" 
	<< "options:\n" << left
	<< setw(25) << "-h | --help" 			<< "display this help\n"
	<< setw(25) << "-l | --lang (EN | DE)" 	<< "language selection. Default: EN\n"
	<< setw(25) << "-c | --color"  			<< "colored output\n"
	<< setw(25) << "-s | --sentences" 		<< "print sentences with occurrences\n"
	<< setw(25) << "-a | --alpha "			<< "sort occurrence map alphabetically, not by count\n";
}


void fillw::setOptions(int argc, char** argv, options &opt) 
{

	int c = 0,
		option_index = 0;
	std::string lang;

	// default parameters
	opt.print_sent 	= false;
	opt.sort_occur 	= true;
	opt.color 		= false;
	opt.help_only	= false;
	opt.word_list 	= &word_list_en;

	// list for available options
 	option long_options[] = 
	{
		{"help", 		no_argument, 		 0, 'h'},
		{"color", 		no_argument, 		 0, 'c'},
		{"sentences", 	no_argument, 		 0, 's'},
		{"alpha", 		no_argument, 		 0, 'a'},
		{"lang", 		required_argument, 	 0, 'l'},
	};


	for (;;) 
	{
		// read next parameter
		c = getopt_long(argc, argv, "l:csah", long_options, &option_index);

		// handle parameter
		switch(c) 
		{
			case -1: 							return;
			case  0:							break;
			case 'c': opt.color = true; 		break;
			case 's': opt.print_sent = true; 	break;
			case 'a': opt.sort_occur = false;	break;

			case 'l':
				lang = std::string(optarg);

				for (auto &c : lang)
					c = std::tolower(c);

				if (lang == "de")			opt.word_list = &fillw::word_list_de;
				else if (lang == "en")		opt.word_list = &fillw::word_list_en;
				else						throw std::invalid_argument("Invalid language");

				break;

			default: // -h for help or invalid parameter 
				fillw::getHelp();
				opt.help_only = true;
		}
	}
}


void fillw::getOccurrences(const std::wstring &data, fillw::word_list_type *word_list,
						   fillw::statistics &stats, fillw::sentence_output &sout)
{
	using std::wstring, std::find_if, std::find_if_not, std::reverse_iterator;
				
	bool prem_break, // premature break, punctuation mark ends the sentence
		 sent_add; // flag for when sentence ends in this iteration 

	size_t dot_fw, dot_bw, word_start, start_wd, end_wd;

	std::vector<size_t> ws_pos;
	std::wstring sent, word;


	// store position of whitespaces
    for(size_t pos = 0; pos != wstring::npos; pos = data.find(L' ', pos+1))
		ws_pos.push_back(pos);

	// init stats
	stats.fill_count = 0; 
	stats.sentence_num = 0; 
	stats.word_count = ws_pos.size()-1;
	

	for(size_t i = 0, j = 0; i < ws_pos.size()-1; i++)
	{
		j = 0;
		prem_break = false;
		sent_add = false;

		for(auto &wn : *word_list)
		{
			if (i+j+1 >= ws_pos.size() || prem_break)
				break;

			word = data.substr(ws_pos.at(i)+1, ws_pos.at(i+j+1)-ws_pos.at(i)-1);

			// increment sentence number if last word before sentence (j=0)
			// and sentence ending punctuation mark in word
			if ((j == 0 && word.find_last_of(punctuation_sent) != wstring::npos)
					|| (j == 0 && i+1 == ws_pos.size()-1))
			{
				sent_add = true;
				stats.sentence_num++;
			}
			
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

			// exclude punctuation marks at the start of the word
			start_wd = find_if_not(word.begin(), word.end(), ::iswpunct) - word.begin();

			if (start_wd == word.length())
				break;
			else if (start_wd != 0)
				word = word.substr(start_wd, word.length() - start_wd + 1);
			
			auto it = wn.find(word);

			if (it != wn.end())
			{
				if (stats.occurrences.find(*it) != stats.occurrences.end())
					stats.occurrences.at(*it)++;
				else
					stats.occurrences.emplace(*it, 1);


				if (sout.active)
				{
					// find sentence ending and beginning. Add -1 to search radius
					// to include punctuation mark belonging appended/prepended to word
					dot_fw = data.find_first_of(punctuation_sent, ws_pos.at(i+j+1)-1);
					dot_fw = data.find_first_not_of(punctuation_sent, dot_fw);
					dot_bw = data.find_last_of(punctuation_sent, ws_pos.at(i));
				
					// handle npos
					dot_bw = (dot_bw == wstring::npos)? 0: 				dot_bw+1;
					dot_fw = (dot_fw == wstring::npos)? data.length():	dot_fw;

					// extract sentence	
					sent = data.substr(dot_bw, dot_fw-dot_bw+1);

					// insert signaling around word
					word_start = ws_pos.at(i) + 1 - dot_bw + start_wd; 
					sent.insert(word_start + it->length(), SIGNAL_SEQ_CLOSE);
					sent.insert(word_start, SIGNAL_SEQ_OPEN);
				
					// print sentence to stringstream. Subtract sent_add in index,
					// since the sentence number was incremented in this iteration
					sout.sentences << HIGHLIGHT_SEQ_OPEN << "Line " 
						<< stats.sentence_lines.at(stats.sentence_num - sent_add) << ": "
						<< HIGHLIGHT_SEQ_CLOSE << sent << "\n\n";
				}

				i += j; // skip search for next words, since they already belong 
						// to a fill word expression
				stats.fill_count++;
				break;
			}

			j++;
		}
	}
}


void fillw::getSentenceLines(const std::wstring &data, fillw::statistics &stats)
{
	using std::find_if, std::count, std::find_if_not, std::wstring;

	auto next = data.begin();
	size_t sentend_pos = 0;

	stats.lines = 0;

	for(auto pos = data.begin(); pos < data.end(); pos = next)
	{
		sentend_pos = data.find_first_of(punctuation_sent, pos-data.begin());

		if (sentend_pos != wstring::npos)
		{
			// store sentence line
			stats.sentence_lines.push_back(stats.lines+1);
			
			// find sentence ending and start of next sentence
			auto sentend = data.begin() + sentend_pos;
			sentend = find_if(sentend, data.end(), ::iswspace);
			next = find_if_not(sentend, data.end(), ::iswspace);
			
			// count lines in sentence
			stats.lines += count(pos, next, L'\n');
		}
		else 
		{
			auto last_ch = find_if_not(pos, data.end(), ::iswspace);
			if (last_ch != data.end())
				stats.sentence_lines.push_back(stats.lines+1);

			// count lines to the end
			stats.lines += count(pos, data.end(), L'\n');
			return;
		}
	}
}

