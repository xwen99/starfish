# -*- coding: utf-8 -*-
import re, sys, time

from itertools import count
from moonfish import *
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

###############################################################################
    
def print_pos(pos):
    print()
    #print('     9  8  7  6  5  4  3  2  1')
    for i, row in enumerate(pos.board.split()):
        print('   %d'%(9-i), u' '.join(uni_pieces.get(p, p) for p in row))
        #print(' %X %d'%(i+3, 9-i), ' '.join(uni_pieces.get(p, p) for p in row))
        #print(' %X  '%(i+3), ' '.join(uni_pieces.get(p, p) for p in row))
    print('      a  b  c  d  e  f  g  h  i')
    #print('      3  4  5  6  7  8  9  A  B')
    print()
    print(u'   红方走' if pos.move_color == 0 else u'   黑方走')
    print()

###############################################################################
# User interface
###############################################################################

###############################################################################
def do_game(fen):

    #load_pad_table('pad_table.txt')
    pos = Position(board_initial, RED, 0)
    #pos = parseFEN('1C1a5/4k4/3a5/4N4/9/5R3/7r1/5p3/4p4/5K3 w')
    #pos = parseFEN('1C3k3/3C2P2/b1N1b4/4p4/9/7r1/R5n2/3n5/2p1p4/3K5 w') #Failed
    checker = RuleChecker()
    
    print(fen)
    #pos = parseFEN(fen)
    
    searcher = Searcher()
    print_pos(pos)
    
    while True:
        
        '''
        if pos.is_checked():
            print(u'红方' if pos.move_color else '黑方', "将军!")
            if pos.is_dead():
                print(u'黑方' if pos.move_color else '红方', "被将死!")
                break
        # We query the user until she enters a (pseudo) legal move.
        #for move in pos.gen_moves():
            #print("%s %X:%X" % (pos.board[move[0]], move[0], move[1]))
        #    print(move_to_zh(pos.board, move))        
        
        move = None
        while move not in pos.gen_moves():
            match = re.match('([a-i][0-9])'*2, input('Your move: '))
            if match:
                move = iccs2internal(match.group(1)), iccs2internal(match.group(2))
                #print(hex(move[0]),hex(move[1]))
                #print(render2(*move))
            else:
                # Inform the user when invalid input (e.g. "help") is entered
                print("Please enter a move like h2e2")
        print(move_to_zh(pos.board, move))        
        pos = pos.move(move)
        # After our move we rotate the board and print it again.
        # This allows us to see the effect of our move.
        print_pos(pos.rotate_board())
         
        if pos.is_checked():
            print(u'红方' if pos.move_color else '黑方', "将军!")
            if pos.is_dead():
                print(u'黑方' if pos.move_color else '红方', "被将死!")
                break
                
        if pos.score <= -MATE_LOWER:
            print(u"你赢了")
            break
        '''
        
        # Fire up the engine to look for a move.
        ban = checker.check_ban_move(pos)
        if ban : 
           print(ban) 
           return 0
        
        move, score, depth = searcher.search(pos, max_depth = 8, ban_move = ban, secs = 20 )
        checker.append_move(pos, move)     
        print("Move: %s(%s) score: %d depth: %d" % (render2(move, pos.move_color), move_to_zh(pos.board, move), score, depth))
         
        pos = pos.move(move)
        
        if pos.move_color == RED:
            print_pos(pos)
        else:
            print_pos(pos.rotate_board())
        
        checked, dead = pos.is_checked_dead()
        if dead:
           print(u'黑方' if pos.move_color else '红方', "被将死!")
           return pos.move_color
        elif checked:
           print(u'红方' if pos.move_color else '黑方', "将军!") 
           
def main():
    bad_games = []
    books = load_from_qcb('end_games.qcb')
    #books = load_from_qcb('hard_game.txt')
    for i, fen in enumerate(books[1636:]):
        try:
            ret = do_game(fen)
            print(i+1)
            if ret == RED:
                bad_games.append(fen)
                break
        except Exception as e:
            print(e)
            time.sleep(3)
            break                
    with open('bad_games.txt', 'a+') as f:
        f.writelines(bad_games)
        
if __name__ == '__main__':
    main()

