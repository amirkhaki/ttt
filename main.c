// game algorithm: https://stackoverflow.com/a/66405791/17649624
// minimax algorithm:
// https://sisoog.com/2021/10/15/%D9%BE%DB%8C%D8%A7%D8%AF%D9%87-%D8%B3%D8%A7%D8%B2%DB%8C-%D9%87%D9%88%D8%B4-%D9%85%D8%B5%D9%86%D9%88%D8%B9%DB%8C-%D8%B4%D8%B7%D8%B1%D9%86%D8%AC-%D9%82%D8%B3%D9%85%D8%AA-%D8%AF%D9%88%D9%85-2/
// presumption: O is minimizer and X is maximizer

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct User {
  char username[40];
  int win_count, lose_count, draw_count;
};

int user_get_score(struct User u) { return u.win_count - u.lose_count; }

int user_compare(const void *v1, const void *v2) {
  const struct User *u1 = (struct User *)v1;
  const struct User *u2 = (struct User *)v2;
  int cmp;
  if (user_get_score(*u1) > user_get_score(*u2)) {
    return -1;
  } else if (user_get_score(*u1) < user_get_score(*u2)) {
    return 1;
  } else if ((cmp = strcmp(u1->username, u2->username)) > 0) {
    return 1;
  } else if (cmp < 0) {
    return -1;
  }

  return 0;
}

struct Users {
  struct User *array;
  size_t used;
  size_t size;
};

// struct Users users;

void users_remove_by_index(struct Users *u, int index) {
  for (int i = index; i < u->used - 1; i++) {
    u->array[i] = u->array[i + 1];
  }
  u->used--;
}

struct User users_pop_user(struct Users *u, struct User user) {
  for (int i = 0; i < u->used; ++i) {
    if (strcmp(u->array[i].username, user.username) == 0) {
      user = u->array[i];
      users_remove_by_index(u, i);
      return user;
    }
  }
  return user;
}

void users_init(struct Users *a, size_t initial_size) {
  a->array = malloc(initial_size * sizeof(struct User));
  a->used = 0;
  a->size = initial_size;
}

void users_insert(struct Users *u, struct User user) {
  if (u->used == u->size) {
    u->size *= 2;
    u->array = realloc(u->array, u->size * sizeof(struct User));
  }
  u->array[u->used++] = user;
}

int users_write_to_file(struct Users *u, FILE *fp) {
  int wc = fwrite(&u->used, sizeof(u->used), 1, fp);
  if (wc != 1) {
    perror("unable to write Users.used");
    return -1;
  }
  wc = fwrite(u->array, sizeof(struct User), u->used, fp);
  if (wc != u->used) {
    perror("unable to write Users.array");
    return -1;
  }
  return 0;
}

int users_read_from_file(struct Users *u, FILE *fp) {
  int used;
  int rc = fread(&used, sizeof(u->used), 1, fp);
  if (rc != 1) {
    perror("unable to read Users.used");
    return -1;
  }
  users_init(u, used);
  rc = fread(u->array, sizeof(struct User), used, fp);
  if (rc != used) {
    perror("unable to read Users.array");
    return -1;
  }
  u->used = used;
  return 0;
}

void users_free(struct Users *u) {
  free(u->array);
  u->array = NULL;
  u->used = u->size = 0;
}

void users_sort(struct Users *u) {
  qsort(u->array, u->used, sizeof(struct User), user_compare);
}

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
  printf("\033[H\033[2J\r\n\tTic Tac Toe\r\n\r\n");
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
  if (p == X_PLAYER) {
    g->x_board |= c;
    return;
  }
  if (p == O_PLAYER) {
    g->o_board |= c;
    return;
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
const cell cells[9] = {a1, a2, a3, b1, b2, b3, c1, c2, c3};

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
    for (int i = 0; i < 9; ++i) {
      if (game_is_move_playable(g, cells[i])) {
        tmp = g;
        tmp.x_board |= cells[i];
        best = max(minimax(tmp, depth + 1, !isMax), best);
      }
    }
    return best;
  } else {
    int best = 1000;
    for (int i = 0; i < 9; ++i) {
      if (game_is_move_playable(g, cells[i])) {
        tmp = g;
        tmp.o_board |= cells[i];
        best = min(minimax(tmp, depth + 1, !isMax), best);
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
  for (int i = 0; i < 9; ++i) {
    if (game_is_move_playable(g, cells[i])) {
      tmp = g;
      tmp.x_board |= cells[i];
      move_val = minimax(tmp, 0, false);
      if (move_val > best_val) {
        best_val = move_val;
        move = cells[i];
      }
    }
  }
  return move;
}

cell get_user_move(enum player p) {
  int u_code = 0;
  do {
    printf("Enter your move %c (1~9):", p == X_PLAYER ? 'X' : 'O');
    scanf("%d", &u_code);
    if (!(u_code > 0 && u_code < 10))
      printf("you must be entered value between 1~9\r\n");

  } while (!(u_code > 0 && u_code < 10));
  u_code--;
  return cells[u_code];
}

void play_1v1(struct Users *users) {
  struct User x = {.username = {0}};
  struct User o = {.username = {0}};

  printf("player O enter your name (max length is 39): ");
  scanf("%s", o.username);

  printf("player X enter your name (max length is 39): ");
  scanf("%s", x.username);

  x = users_pop_user(users, x);
  o = users_pop_user(users, o);

  struct Game g = {0, 0};
  int game_score = 0;
  int i = 0;
  print_board(g);
  do {
    enum player p = i % 2 == 0 ? O_PLAYER : X_PLAYER;
    cell u_move = get_user_move(p);
    if (!game_is_move_playable(g, u_move)) {
      if (!game_is_any_move_left(g)) {
        break;
      }
      printf("You can't use this location\r\nThis location has already been "
             "used!\r\n");
      continue;
    }
    game_play_move(&g, u_move, p);
    print_board(g);
    ++i;
  } while ((game_score = game_evaluate_score(g)) == 0);

  print_board(g);

  if (game_score == 0) {
    printf("draw\n");
    x.draw_count++;
    o.draw_count++;
  } else if (game_score > 0) {
    printf("X won\n");
    x.win_count++;
    o.lose_count++;
  } else {
    printf("O won\n");
    x.lose_count++;
    o.win_count++;
  }
  users_insert(users, x);
  users_insert(users, o);
  users_sort(users);
}
void play_with_ai(void) {
  struct Game g = {0, 0};
  int evaluate = 0;
  print_board(g);
  do {
    cell u_move = get_user_move(O_PLAYER);
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

void print_scoreboard(const struct Users *u) {
  printf("username\twin_count\tlose_count\tdraw_count\n");
  for (int i = 0; i < u->used; ++i) {
    printf("%40s\t%5d\t%5d\t%5d\n", u->array[i].username, u->array[i].win_count,
           u->array[i].lose_count, u->array[i].draw_count);
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: ./a.out /path/to/file.db\n");
    return EXIT_FAILURE;
  }
  struct Users users;
  FILE *fp = NULL;
  if ((fp = fopen(argv[1], "r")) != NULL) {
    if (users_read_from_file(&users, fp) == -1) {
      fclose(fp);
      printf("error reading from file\n");
      return EXIT_FAILURE;
    }
  } else {
    users_init(&users, 2);
  }

  play_1v1(&users);
  print_scoreboard(&users);

  if ((fp = fopen(argv[1], "w")) != NULL) {
    if (users_write_to_file(&users, fp) == -1) {
      printf("error writing to file\n");
      fclose(fp);
      return EXIT_FAILURE;
    }
  } else {
    printf("unable to write file\n");
    fclose(fp);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
