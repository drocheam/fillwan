#include "definitions.hpp"

// word lists
#include "words_de.hpp"
#include "words_en.hpp"

// strings
#include <string>  // std::string, std::wstring
#include <string_view>  // std::wstring_view
#include <sstream>  // std::wostringstream

// other types
#include <vector>  // std::vector
#include <iostream>  // std::wcout, std::wcerr
#include <array>  // std::array

// functionality
#include <iomanip>  // std::setw
#include <algorithm>  // std::transform, std::find_if, std::find_if_not
#include <cstdio>  // std::fopen, std::fclose


void fillw::getHelp() 
{
    using std::wcout, std::left, std::setw;

    wcout
    << "usage: fillwan [options] [file]\n" 
    << "reads from standard input if no file is provided\n\n"
    << "options:\n" << left
    << setw(25) << "-h | --help"           << "display this help\n"
    << setw(25) << "-l | --lang (EN | DE)" << "language selection. English or German. Default: EN\n"
    << setw(25) << "-c | --color"          << "enable colored output\n"
    << setw(25) << "-d | --dump"           << "dump text with highlighted fill expressions\n"
    << setw(25) << "-a | --alpha "         << "sort occurrence map alphabetically, not by count\n";
}


void fillw::setOptions(int argc, char** argv, options &opt) 
{
    // we could use getopt.h for POSIX systems instead, but this would make it platform dependent

    std::string lang, par;

    // default parameters
    opt.dump        = false;
    opt.sort_occur  = true;
    opt.color       = false;
    opt.help_only   = false;
    opt.path        = "";
    opt.word_list   = &word_list_en;

	// argument vector
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
			// no language provided
            if (i == size_t(args.size() - 1))
			{
                getHelp();
                exit(2);
            }

            // enforce lower case
            lang = args.at(++i);
            std::transform(lang.begin(), lang.end(), lang.begin(), 
                           [](auto c){ return std::towlower(c); });

			// assign correct language

            if (lang == "de")            
                opt.word_list = &fillw::word_list_de;

            else if (lang == "en")        
                opt.word_list = &fillw::word_list_en;

			// unknown language
            else
			{
                getHelp();
                exit(2);
            }
        }
		// file path
        else if (i == args.size() - 1 && args.at(i).at(0) != '-')
            opt.path = args.at(i);

		// invalid parameter
        else
		{
            getHelp();
            exit(2);
        }
    }

    // assign output sequences
    HIGHLIGHT_SEQ_CLOSE = (opt.color)?  RESET_SEQ_ANSI:          RESET_SEQ_NON_ANSI;
    HIGHLIGHT_SEQ_OPEN  = (opt.color)?  HIGHLIGHT_SEQ_ANSI:      HIGHLIGHT_SEQ_NON_ANSI;
    SIGNAL_SEQ_OPEN     = (opt.color)?  SIGNAL_SEQ_ANSI_OPEN:    SIGNAL_SEQ_NON_ANSI_OPEN;
    SIGNAL_SEQ_CLOSE    = (opt.color)?  SIGNAL_SEQ_ANSI_CLOSE:   SIGNAL_SEQ_NON_ANSI_CLOSE;
}


void fillw::getOccurrences(std::wstring_view data, const fillw::options &opt,
                           fillw::statistics &stats, std::wostringstream &sout)
{
    using std::wstring, std::find_if, std::find_if_not, std::reverse_iterator;
                
    bool prem_break; // premature break, punctuation mark ends the sentence
    bool succ;  // fill expression found in the current word iteration

    size_t ws_e, start_wd, end_wd;

    std::vector<size_t> ws_pos;  // white space position
    std::wstring_view word;  // fill expression
    std::wstring word_s;  // wstring version of word above

    // pre-reserve for speedup. a word length of 5 is a guess
    ws_pos.reserve(size_t(data.length()/5));

	// always treat text beginning as whitespace
    if (!std::iswspace(data.at(0)))
        ws_pos.push_back(0);

	// find whitespace groups
	// multiple consecutive whitespaces are treated as one
    for(size_t pos = 0; pos < data.length(); pos++)
    {
        pos = std::find_if(data.begin()+pos, data.end(), std::iswspace) - data.begin();
        ws_pos.push_back(pos);
        pos = std::find_if_not(data.begin()+pos, data.end(), std::iswspace) - data.begin();
    }

	// always treat text ending as whitespace
    if (!std::iswspace(data.at(data.length()-1)))
        ws_pos.push_back(data.length());
    
    // init stats
    stats.fill_count = 0;  // number of fill expressions
    stats.word_count = 0;  // number of words

	// iterate over all whitespace positions (=possible word boundaries)
    for(size_t i = 0; i < ws_pos.size()-1; i++)
    {
        prem_break = false;  // break after next j-th iteration below
        succ = false; // expression found

		// find next non-whitespace character. Then find next not-punctuation character
        ws_e = std::find_if_not(data.begin()+ws_pos.at(i), data.end(), std::iswspace) - data.begin();
        start_wd = std::find_if_not(data.begin()+ws_e, data.begin()+ws_pos.at(i+1), ::iswpunct) - data.begin();

		// when the found position is smaller than next whitespace position, this should be a word beginning
		// otherwise we found a group of punctuation marks between whitespaces
        if (start_wd < ws_pos.at(i+1))
        {
			// from this position on, search to 1., 2., ... Nth whitespace, 
			// with N being the maximum word length of a fill expression
            for(size_t j = 0; j < opt.word_list->size(); j++)
            {
				// skip to i+1 iteration when at the end of text or prem_break (=invalid word)
                if (i+j+1 >= ws_pos.size() || prem_break)
                    break;

				// "word"/expression goes from initial whitespace at i to the i+j-th one in the j-th iteration
                word = data.substr(start_wd, ws_pos.at(i+j+1)-start_wd);

                // exclude punctuation marks at the end of the word/expression
                auto end_it = find_if_not(reverse_iterator(word.end()), 
                                          reverse_iterator(word.begin()), ::iswpunct);
                end_wd = word.length() - (end_it - reverse_iterator(word.end()));
            
				// marks found 
				// if the current j-th iteration fails, there is no fill expression here
				// since fill expressions have no punctuation marks inside them
                if (end_wd != word.length()){
                    // exclude mark at the end of the word
					// continue to i+1-th word after this j-th iteration
                    word = word.substr(0, end_wd);
                    prem_break = true;
                }
				// i+j-th "word" consists of only punctuation marks
				// in this case the search is ended for this i-iteration
                else if (end_wd == 0)
                    break;

				// create string from string_view
                word_s = std::wstring(word);
                
				// we're checking for multiple words, 
				// in this case we need to convert all whitespaces to a normal space and join multiple ones
				// so we can compare to our fill expressions
                if (j > 0)
                {
                    // convert whitespaces to space, remove multiple ones
                    std::replace_if(word_s.begin(), word_s.end(), ::iswspace, L' ');
                    auto I = std::unique(word_s.begin(), word_s.end(), 
                                        [](auto& lhs, auto& rhs){ return lhs == L' ' && lhs == rhs; } );
                    word_s.erase(I, word_s.end());
                }

                auto &wn = opt.word_list->at(j);  // get fill expression set with word length of j
                auto it = wn.find(word_s);  // try finding our current expression

                // on first word iteration (j=0), add word count
                // if we got this far we already excluded cases where there are only punctuation marks between whitespaces
				// which wouldn't count as words
                if (j == 0) 
                    stats.word_count++;

				// expression found successfully
                if (it != wn.end())
                {
					// dump mode: insert highlighting sequence around expression
                    if (opt.dump)
                        sout << data.substr(ws_pos.at(i), start_wd-ws_pos.at(i))
                             << SIGNAL_SEQ_OPEN 
                             << data.substr(start_wd, end_wd)
                             << SIGNAL_SEQ_CLOSE 
                             << data.substr(start_wd + end_wd, ws_pos.at(i+j+1)-(start_wd+end_wd));
					// normal mode: increment expression counter
                    else
                        stats.occurrences[*it]++;

                    i += j; // skip search for next words, since they already belong 
                            // to a fill word expression
                    stats.fill_count++; // increment global fill word counter
                    stats.word_count += j; // add skipped words to word count
                    succ = true;  // expression found
                    break;
                }
            }
        }
		// in dump mode add to dump even if no expressions are found
        if (!succ && opt.dump)
            sout << data.substr(ws_pos.at(i), ws_pos.at(i+1)-ws_pos.at(i)); 
    }
    // add remaining text with no fill expressions to dump
    if (opt.dump)
        sout << data.substr(ws_pos.at(ws_pos.size()-1), data.length()); 
}


int fillw::getText(const options &opt, std::wstring &text)
{
    // exit if not connected to a pipe and no file is provided
    if (opt.path == "" && !fillw::inPipe())
    {
        std::wcerr << "Pipe text into this program or specify a file path." << std::endl;
        return 1;
    }
    else
    {
        FILE * file;

		// read from file
        if (opt.path != "")
        {
            file =  fopen(opt.path.c_str(), "r");
            if (file == NULL)
            {
                std::wcerr << "File Not Found" << std::endl;
                return -1;
            }
        }
		// read from pipe (=stdin)
        else
            file = stdin;

        // buffered read is much faster than getline etc.
        std::array<wchar_t, 2 << 12> buffer;
        while(fgetws(buffer.data(), buffer.size(), file) != NULL)
            text.append(buffer.data());

		// empty pipe
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

    // file has always at least one line
    return (count)? count: count + 1;
}

