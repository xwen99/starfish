# -*- coding: utf-8 -*-
import importlib
import re
import sys
import time

from moonfish import *
import tools


def main():
    pos = tools.parseFEN(tools.FEN_INITIAL)
    searcher = Searcher()
    #  default options
    color = RED
    #max_time = -1
    max_time = 1000
    #max_depth = 1000
    max_depth = 8
    show_thinking = False

    stack = []
    while True:
        if stack:
            smove = stack.pop()
        else:
            smove = input()

        if smove == 'quit':
            print('bye')
            sys.stdout.flush()
            break

        elif smove == 'isready':
            sys.stdout.flush()
            print('readyok')

#        elif smove == 'newgame':
#            stack.append('position fen ' + tools.FEN_INITIAL)

        elif smove.startswith('position'):
            params = smove.split(' ', 2)
            if params[1] == 'fen':
                fen = params[2]
                pos = tools.parseFEN(fen)
                color = RED if fen.split()[1] == 'w' else BLACK
                start = time.time()
                for s_score in searcher._search(pos):
                    moves = tools.pv(searcher, pos, include_scores = False)

                    if show_thinking:
                        entry = searcher.tp_score.get(
                            (pos, searcher.depth, True))
                        score = int(round((entry.lower + entry.upper)/2))
                        usedtime = int((time.time() - start) * 1000)
                        moves_str = moves  # if len(moves) < 15 else ''
                        print('info depth {} score {} time {} nodes {} pv {}'.format(
                            searcher.depth, score, usedtime, searcher.nodes, moves_str))

                    if max_time > 0 and (time.time() - start) * 1000 > max_time:
                        break
                    
                    if searcher.depth >= max_depth:
                        break

                moves = moves.split(' ')
                print('bestmove ' + moves[0])
                sys.stdout.flush()
        else:
            pass


if __name__ == '__main__':
    main()
