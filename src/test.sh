#!/bin/bash
assert(){
    excepted="$1"
    input="$2"

    ./tan-iCC "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$excepted" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected, but got $actual"
        exit 1
    fi
}

assert 0 0
assert 42 42

echo OK