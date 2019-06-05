# :star:starfish

## Introduction

A Chinese cheese engine written in C++, final project of Spring 2019 Artificial Intelligence at Tongji Univ.

Starfish is featured by its high-efficiency best move searching algorithm. We let the AI figure out the gaming dark magic during deep adversial searching, rather than providing much prior knowledge with it(~~Because none of us is good at playing Chinese cheese :)~~).

## Module Organization

* ucci.h/ucci.cpp
  * UCCI communication module

* tools.h
  * Basic tool functions

* book.h/book.cpp
  * Opening book processing module

* buffer.h
  * A fixed opening book extracted from BOOK.DAT in eleeye

* position.h/position.cpp
  * Move generating module

* movesort.h
  * Move sorting module

* search.h/search.cpp
  *  best move searching module

## Compiler Environment

MSVC 16.1.0

## Play with It

### Protocol

We support a simplified version of Universal Chinese Chess Protocol(UCCI), a text-based communication protocol between a chess interface and a chess engine.

The supported instructions are:

1. ucci
2. isready
3. position {fen <FEN string> | startpos} [moves < follow-up list>]
4. go time \<milliseconds>
5. quit

Please refer to <http://www.xqbase.com/protocol/cchess_ucci.htm> for details.

### For Human

starfish-debug.exe provides a simple graphical interface, have fun playing with it.

### For Silicon-Based Organism

You may have more common languages with starfish-release.exe.

## Why Starfish?
To pay tribute to our predecessors who developed xxfishes...

Since there are already stockfish, sunfish and moonfish, why not starfish?

![haixing](http://ww3.sinaimg.cn/large/9150e4e5ly1fn3sav37xbj203602qt8o.jpg)

## Reference

* Computer Cheese Step by Step(<http://www.xqbase.com/computer/stepbystep1.htm>)
* Chess Programming Wiki(<https://www.chessprogramming.org>)
* Chinese Chess AI eleeye(<https://github.com/xqbase/eleeye>)
* Chinese Chess AI moonfish(<https://github.com/walker8088/moonfish>)
* Chinese Chess AI XiangQi Wizard(<https://github.com/xqbase/xqwlight>)
* Chess AI sunfish(<https://github.com/thomasahle/sunfish>)
* Chess AI stockfish(<https://github.com/mcostalba/Stockfish>)
