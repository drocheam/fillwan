
# fillwan 
## Fillword Analysis

This tool analyzes a given text for fill words and other potentially unwanted words and expressions in scientific writing.
Outputs are a list of found words with occurence count as well as a sentence list.


### Analyzed Words (English)

A list of analyzed categories with examples can be found below.

* first person perspective (I, my, we, our, ...)
* conjunctive (could, would, ...)
* transition words (but, therefore)
* colloquial terms (ok, nice, stuff, ...)
* fill words (indeed, really, very, ...)
* weakening expressions (quite, most, probably, ...)
* words and adjectives with little meaning (good, right, thing, ...)
* commonly overused expression (be seen, is shown, for example, ...)


### Analyzed Words (German)

* Personalpronomen (ich, mein, wir, uns, ...)
* Indefinitpronomen (man)
* Passivformen (wurde, werden, ...)
* Konjunktivformen (sei, wäre, könne, würde, ...)
* Füllwörter (auch, meist, wohl, ...)
* Konjunktionen und Satzverbindungen (außerdem, nichtdestotrotz, im nächsten Schritt, ...)
* häufig überbenutzte Formulierungen ("zu sehen ist", "es ergibt sich", "in dieser Arbeit", ...)
* Umgangssprachliche Formen (eh, halt, irgendwas, ...)
* ungenaue Adjektive oder Wörter mit wenig Aussagekraft (gut, schlecht, einigermaßen, oft, ...)


### Usage

The below program options are also displayed using the -h parameter.

```
usage: fillwan [options] [file]
reads from standard input if no file is provided

options:
-h | --help              display this help
-l | --lang (EN | DE)    language selection. English or German. Default: EN
-c | --color             enable colored output
-d | --dump              dump text with highlighted fill expressions
-a | --alpha             sort occurrence map alphabetically, not by count
```

Scroll through the text with the expressions being highlighted
```
cat text.txt | fillwan -cd | less -RN
```

Print all lines containing fill expressions with line numbers
```
cat text.txt | fillwan -d | grep -ne "<<[^>]\+>>"
```

### Building Instructions

The getopt.h library is needed, therefore this tool only builds on Unix-Systems.
No other external libraries are required.
This program builds using the C++20 standard.


### Word Sources
English:
https://www.indeed.com/career-advice/career-development/filler-word
https://www.myenglishpages.com/english/communication-lesson-conversation-fillers.php
http://languagelearningbase.com/88430/english-naturally-using-filler-phrases-hesitation-strategies
https://github.com/words/fillers/blob/main/data.txt
https://smartblogger.com/filler-words/
https://www.allpurposeguru.com/2021/08/filler-words-useless-phrases-you-can-cut-from-your-writing/
https://www.scribbr.com/academic-writing/transition-words/
https://blog.wordvice.com/common-transition-terms-used-in-academic-papers/
https://libguides.staffs.ac.uk/academic_writing/linking
https://writing.wisc.edu/handbook/style/transitions/

German:
https://web.archive.org/web/20210126114619/https://www.schreiblabor.com/fuellwoerter-test/

