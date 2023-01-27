// game algorithm: https://stackoverflow.com/a/66405791/17649624
// minimax algorithm:
// https://sisoog.com/2021/10/15/%D9%BE%DB%8C%D8%A7%D8%AF%D9%87-%D8%B3%D8%A7%D8%B2%DB%8C-%D9%87%D9%88%D8%B4-%D9%85%D8%B5%D9%86%D9%88%D8%B9%DB%8C-%D8%B4%D8%B7%D8%B1%D9%86%D8%AC-%D9%82%D8%B3%D9%85%D8%AA-%D8%AF%D9%88%D9%85-2/
// presumption: O is minimizer and X is maximizer

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXIMIZER_WIN_SCORE 10
#define MINIMIZER_WIN_SCORE -10

#define max(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

#define min(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })

typedef int32_t cell;

struct Game {
  int32_t o_board, x_board;
};

enum player { X_PLAYER, O_PLAYER };

const cell a1 = 0x80080080;
const cell a2 = 0x40008000;
const cell a3 = 0x20000808;
const cell b1 = 0x08040000;
const cell b2 = 0x04004044;
const cell b3 = 0x02000400;
const cell c1 = 0x00820002;
const cell c2 = 0x00402000;
const cell c3 = 0x00200220;

char game_cell_char(struct Game g, cell c, char default_char) {
  return (g.x_board & c) ? 'X' : (g.o_board & c ? 'O' : default_char);
}

void print_board(struct Game g) {
  printf("\033[1J\r\n\tTic Tac Toe\r\n\r\n");
  printf("Player 1 (X)  -  Player 2 (O)\r\n\r\n\r\n");
  printf("     |     |     \n");
  printf("  %c  |  %c  |  %c \n", game_cell_char(g, a1, '1'),
         game_cell_char(g, a2, '2'), game_cell_char(g, a3, '3'));
  printf("_____|_____|_____\n");
  printf("     |     |     \n");
  printf("  %c  |  %c  |  %c \n", game_cell_char(g, b1, '4'),
         game_cell_char(g, b2, '5'), game_cell_char(g, b3, '6'));
  printf("_____|_____|_____\n");
  printf("     |     |     \n");
  printf("  %c  |  %c  |  %c \n", game_cell_char(g, c1, '7'),
         game_cell_char(g, c2, '8'), game_cell_char(g, c3, '9'));
  printf("     |     |     \n\n");
}

_Bool game_is_move_playable(struct Game g, cell c) {
  return ((g.o_board | g.x_board) & c) == 0;
}

void game_play_move(struct Game *g, cell c, enum player p) {
  switch (p) {
  case X_PLAYER:
    g->x_board |= c;
    break;
  case O_PLAYER:
    g->o_board |= c;
    break;
  }
}

_Bool game_is_any_move_left(struct Game g) {
  return (g.x_board | g.o_board) !=
         (a1 | a2 | a3 | b1 | b2 | b3 | c1 | c2 | c3);
}

int game_evaluate_score(struct Game g) {
  return (g.o_board & (g.o_board << 1) & (g.o_board >> 1)) != 0
             ? MINIMIZER_WIN_SCORE
             : ((g.x_board & (g.x_board << 1) & (g.x_board >> 1)) != 0
                    ? MAXIMIZER_WIN_SCORE
                    : 0);
}
const cell cells[3][3] = {{a1, a2, a3}, {b1, b2, b3}, {c1, c2, c3}};

int minimax(struct Game g, int depth, _Bool isMax) {
  int score = game_evaluate_score(g);

  if (score == MAXIMIZER_WIN_SCORE) {
    return score - depth;
  } else if (score == MINIMIZER_WIN_SCORE) {
    return score + depth;
  }

  if (!game_is_any_move_left(g)) {
    return 0;
  }
  struct Game tmp;
  if (isMax) {
    int best = -1000;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (game_is_move_playable(g, cells[i][j])) {
          tmp = g;
          tmp.x_board |= cells[i][j];
          best = max(minimax(tmp, depth + 1, !isMax), best);
        }
      }
    }
    return best;
  } else {
    int best = 1000;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        if (game_is_move_playable(g, cells[i][j])) {
          tmp = g;
          tmp.o_board |= cells[i][j];
          best = min(minimax(tmp, depth + 1, !isMax), best);
        }
      }
    }
    return best;
  }
}

cell game_find_best_move(struct Game g, enum player p) {
  cell move = -1;
  int best_val = -100000, move_val;
  struct Game tmp;
  if (p == O_PLAYER) {
    // assumed that X is maximizer, so if wanna find best move for O_PLAYER, we
    // should swap
    int32_t t = g.x_board;
    g.x_board = g.o_board;
    g.o_board = t;
  }
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (game_is_move_playable(g, cells[i][j])) {
        tmp = g;
        tmp.x_board |= cells[i][j];
        move_val = minimax(tmp, 0, false);
        if (move_val > best_val) {
          best_val = move_val;
          move = cells[i][j];
        }
      }
    }
  }
  return move;
}

cell get_user_move() {
  int u_code = 0;
  do {
    printf("Enter your move (1~9):");
    scanf("%d", &u_code);
    if (!(u_code > 0 && u_code < 10))
      printf("you must be entered value between 1~9\r\n");

  } while (!(u_code > 0 && u_code < 10));
  u_code--;
  cell *moves = cells;
  return moves[u_code];
}

void play_with_ai(void) {
  struct Game g = {0, 0};
  int evaluate = 0;
  print_board(g);
  do {
    cell u_move = get_user_move();
    if (!game_is_move_playable(g, u_move)) {
      if (!game_is_any_move_left(g)) {
        break;
      }
      printf("You can't use this location\r\nThis location has already been "
             "used!\r\n");
      continue;
    }
    game_play_move(&g, u_move, O_PLAYER);
    cell ai_move = game_find_best_move(g, X_PLAYER);
    if (ai_move == -1) {
      break;
    }
    game_play_move(&g, ai_move, X_PLAYER);
    print_board(g);
  } while ((evaluate = game_evaluate_score(g)) == 0);

  print_board(g);

  if (evaluate == 0) {
    printf("draw\n");
  } else if (evaluate > 0) {
    printf("X won\n");
  } else {
    printf("O won\n");
  }
}

int main(int argc, char *argv[]) {
  play_with_ai();
  return EXIT_SUCCESS;
}
