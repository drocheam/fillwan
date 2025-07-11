#!/usr/bin/bash

fillwan=./bin/fillwan

errors=0

assert() {
    ! eval "$1" > /dev/null && echo "Test failure in file $0, line ${BASH_LINENO}, condition: $1" 1>&2 && errors=$((errors+1))
}

# test text handling (words, characters, newlines, encoding)
assert "$fillwan ./tests/files/special_cases.txt | grep '49 words'"
assert "$fillwan ./tests/files/test_encoding.txt | grep '14 words'"
assert "$fillwan ./tests/files/test_encoding.txt | grep '82 characters'"
assert "$fillwan ./tests/files/lorem_ipsum_wc.txt | grep '2584 characters, 415 words, 14 lines, 4 fill expressions'"
assert "[[ $($fillwan -d ./tests/files/text_de_fill_words.txt | wc -l) == '17' ]]"

# test correct counting of fill expressions
assert "$fillwan -l de ./tests/files/text_de_fill_words.txt | grep '39 fill expressions'"
assert "$fillwan ./tests/files/text_en_fill_words.txt | grep '18 fill expressions'"

# compare dumped colored output
file=$(cat ./tests/files/correct_coloring_colored.txt)
output=$($fillwan -cd ./tests/files/correct_coloring.txt)
assert " [[ \"$output\" == \"$file\" ]] "

# compare dumped uncolored output
file=$(cat ./tests/files/correct_coloring_uncolored.txt)
output=$($fillwan -d ./tests/files/correct_coloring.txt)
assert " [[ \"$output\" == \"$file\" ]] "

# test sorting
assert "$fillwan -l de -a ./tests/files/text_de_fill_words.txt | grep 'aber.*anderen.*auch'"

# piping also works
assert "cat ./tests/files/text_en_fill_words.txt | $fillwan | grep '18 fill expressions'"

# file is preferred when both pipe and file option are present
assert "cat ./tests/files/text_en_fill_words.txt | $fillwan  ./tests/files/lorem_ipsum_wc.txt | grep '4 fill expressions'"

# info message if no pipe or file is present
assert "$fillwan 2>&1 | grep 'Pipe text into this program'"

# file does not exist
assert "$fillwan ./tests/abc 2>&1 | grep 'File not found.'"

# empty text
assert "echo -n '' | $fillwan 2>&1 | grep 'No text received.'"

# a file has at least one line
assert "echo -n ' ' | $fillwan | grep '1 lines'"
assert "echo ' ' | $fillwan | grep '1 lines'"

# exit with non zero status code when errors occurred
[[ $errors != 0 ]] && exit 1
exit 0
