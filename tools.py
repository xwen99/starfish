import itertools
import re

from moonfish import *

################################################################################
# This module contains functions used by test.py and xboard.py.
# Nothing from here is imported into moonfish.py which is entirely self-sufficient
################################################################################

FEN_INITIAL = 'rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1'

################################################################################
# Parse and Render moves
################################################################################

def gen_legal_moves(pos):
    ''' pos.gen_moves(), but without those that leaves us in check.
        Also the position after moving is included. '''
    for move in pos.gen_moves():
        pos1 = pos.move(move)
        # If we just checked for opponent moves capturing the king, we would miss
        # captures in case of illegal castling.
        if not any(pos1.value(m) >= MATE_LOWER for m in pos1.gen_moves()):
            yield move, pos1

def mrender(pos, m):
    p = ''
    m = m if pos.move_color == RED else (181-m[0], 181-m[1])
    return internal2iccs(m[0]) + internal2iccs(m[1]) + p

def mparse(color, move):
    m = (iccs2internal(move[0:2]), iccs2internal(move[2:4]))
    return m if color == RED else (181-m[0], 181-m[1])

###############################################################################
def load_pad_table(file):
    with open(file, 'r', encoding = 'utf-8') as f:
        lines = f.readlines()
    index = 0    
    while True:
        if index >= len(lines):
            return True
        piece = lines[index].strip()        
        if piece not in pst:
            print("load pad table error at %d %s" % (index+1, piece))
            return False
        table = pst[piece]    
        for i in range(10):
            values = [int(x) for x in lines[index+i+1].strip().split()]
            for j in range(9):
                table[(i+3)*16+j+3] = values[j]
            #print(values)
            #print(table[(i+3)*16:(i+4)*16])            
        index += 11
    return True
    
def load_from_qcb(qcb_file):
        with open(qcb_file) as f:
            lines = f.readlines()
        
        books = []
        
        for line in lines:
            if line.startswith("*") :
                continue
            items = line.strip()
            books.append(items)
        
        return books    
        
###############################################################################
    
def print_pos(pos):
    print()
    print(u'   红方走' if pos.move_color == 0 else u'   黑方走')
    #print('     9  8  7  6  5  4  3  2  1')
    for i, row in enumerate(pos.board.split()):
        print('   %d'%(9-i), u' '.join(uni_pieces.get(p, p) for p in row))
        #print(' %X %d'%(i+3, 9-i), ' '.join(uni_pieces.get(p, p) for p in row))
        #print(' %X  '%(i+3), ' '.join(uni_pieces.get(p, p) for p in row))
    print('      a  b  c  d  e  f  g  h  i')
    #print('      3  4  5  6  7  8  9  A  B')
    print()  
    
################################################################################
# Parse and Render positions
################################################################################

def parseFEN(fen):
    """ Parses a string in FEN into a Position """
    board, color = fen.split()[:2]
    board = re.sub(r'\d', (lambda m: '.'*int(m.group(0))), board)
    
    init_board = list(LINE_WITH*LINE_HEIGHT*' ')
    init_board[12::13] = ['\n']*14
    for i, row in enumerate(board.split('/')):
        for j, ch in enumerate(row):
            init_board[(i+2) * LINE_WITH + j+2] = ch
    init_board = ''.join(init_board)
    
    score = sum(pst[p][i] for i,p in enumerate(init_board) if p.isupper())
    score += sum(piece[p] for i,p in enumerate(init_board) if p.isupper())
    score -= sum(pst[p.upper()][181 - i] for i,p in enumerate(init_board) if p.islower())
    score -= sum(piece[p.upper()] for i,p in enumerate(init_board) if p.islower())
    
    pos = Position(init_board, RED, score)
    return pos if color == 'w' else pos.rotate()
    
################################################################################
# Pretty print
################################################################################

def pv(searcher, pos, include_scores=True):
    res = []
    seen_pos = set()
    color = pos.move_color
    origc = color
    if include_scores:
        res.append(str(pos.score))
    while True:
        move = searcher.tp_move.get(pos)
        if move is None:
            break
        res.append(mrender(pos, move))
        pos, color = pos.move(move), 1-color
        if pos in seen_pos:
            res.append('loop')
            break
        seen_pos.add(pos)
        if include_scores:
            res.append(str(pos.score if color==origc else -pos.score))
    return ' '.join(res)

################################################################################
# Bulk move generation
################################################################################

def expand_position(pos):
    ''' Yiels a tree of generators [p, [p, [...], ...], ...] rooted at pos '''
    yield pos
    for _, pos1 in gen_legal_moves(pos):
        yield expand_position(pos1)

def collect_tree_depth(tree, depth):
    ''' Yields positions exactly at depth '''
    root = next(tree)
    if depth == 0:
        yield root
    else:
        for subtree in tree:
            for pos in collect_tree_depth(subtree, depth-1):
                yield pos

def flatten_tree(tree, depth):
    ''' Yields positions exactly at less than depth '''
    if depth == 0:
        return
    yield next(tree)
    for subtree in tree:
        for pos in flatten_tree(subtree, depth-1):
            yield pos

