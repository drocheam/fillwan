#include "definitions.hpp"

#include <algorithm>  // std::sort
#include <string>  // std::wstring
#include <sstream>  // std::wostringstream
#include <iostream>  // std::wcout
#include <iomanip> // std::setw
#include <clocale>  // std::setlocale


int main(int argc, char** argv)
{
    // initialization /////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    std::setlocale(LC_ALL, "");
    
    fillw::options opt;
    fillw::statistics stats;
    fillw::output_map_type output_map;

    std::wostringstream sout;
    std::wstring text;
    int status;

    // set options from cmd line parameters
    fillw::setOptions(argc, argv, opt);

    if (opt.help_only)
        return 0;

    // processing /////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////

    // try reading the text and return status code
    status = fillw::getText(opt, text);

    // return error code
    if (status)
        return status;

    fillw::getOccurrences(text, opt, stats, sout);

    // generate occurrence vector and stats
    // only needed if not in dump mode
    if (!opt.dump)
    {
        // get length and line count
        stats.length = text.length();
        stats.lines = fillw::getLineCount(text);

        // assign elements to vector
        for (auto& it : stats.occurrences)
            output_map.emplace_back(it);
          
        // sort occurrence descending, but same number of occurrences alphabetically
        if (opt.sort_occur)
            std::sort(output_map.begin(), output_map.end(), [](auto &a, auto &b)
                     { return (a.second != b.second)? a.second > b.second: a.first < b.first; });

        // sort alphabetically
        else
            std::sort(output_map.begin(), output_map.end(), [](auto &a, auto &b) 
                     {return fillw::caseless_less(a.first, b.first);});
    }

    // output /////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////
    
    // dump and exit when in dump mode
    if (opt.dump)
    {
        std::wcout << sout.str();
        return 0;
    }
    
    // show statistics
    std::wcout << HIGHLIGHT_SEQ_OPEN << L"\nStatistics\n" << HIGHLIGHT_SEQ_CLOSE
               << SEPARATOR 
               << stats.length       << " characters, " 
               << stats.word_count   << " words, " 
               << stats.lines        << " lines, "
               << stats.fill_count   << " fill expressions\n\n\n";

    // list fill expressions
    if (output_map.size())
    {
        std::wcout << HIGHLIGHT_SEQ_OPEN << L"\nFill Expression Occurrences\n" 
                   << HIGHLIGHT_SEQ_CLOSE << SEPARATOR;

        for (size_t pos = 0; auto &el : output_map)
            std::wcout << std::left  << std::setw(24) << el.first 
                       << std::right << std::setw(5)  << el.second 
                       << ((++pos % 3 == 0 || pos == output_map.size())? L"\n" : L"   ");
    }
    else
        std::wcout << HIGHLIGHT_SEQ_OPEN + L"No Fill Expressions Found.\n\n" + HIGHLIGHT_SEQ_CLOSE;

    return 0;
}

