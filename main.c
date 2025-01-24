// video linki: https://www.youtube.com/watch?v=LL9q3bY4DS4

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STATE_MENU 0
#define STATE_NEW_GAME 1
#define STATE_PLAY 2
#define STATE_GAME_OVER 3

#define PLAYER_COUNT 2
#define MAX_SIZE 20
#define NUM_PIECE_TYPE 5
#define STACK_SIZE 5

typedef struct {
  int x, y;
} Position;

typedef struct {
  int x1, y1, x2, y2;
  char captured_piece;
} Move;

typedef struct {
  Move moves[STACK_SIZE];
  int top;
} MoveStack;

typedef struct {
  int boyut;
  char **data;
} GameBoard;

typedef struct {
  int gameMode;
  int turn;
  int score[PLAYER_COUNT][NUM_PIECE_TYPE];
  int moveMade;
  Position movedPiecePosition;
  GameBoard *board;
  MoveStack undo_stack;
  MoveStack redo_stack;
  int complete_sets[PLAYER_COUNT]; // Tam setleri takip etmek için
} Info;

int IsMiddleOfBoard(int N, int x, int y) {
  if (N % 2 == 0) {
    return (x == N / 2 || x == N / 2 - 1) && (y == N / 2 || y == N / 2 - 1);
  } else {
    return (x <= N / 2 + 1 && x >= N / 2 - 1) &&
           (y <= N / 2 + 1 && y >= N / 2 - 1);
  }
}

void randomPlacement(
    GameBoard *board) { // taşların tahtaya rastgele yerleşmesini sağlamak için
  int i, j;
  int totalPieceCount = board->boyut * board->boyut - 4;
  int pieceCountPerType = totalPieceCount / 5;
  int remainder = totalPieceCount % 5;

  int pieceCount[NUM_PIECE_TYPE] = {pieceCountPerType, pieceCountPerType,
                                    pieceCountPerType, pieceCountPerType,
                                    pieceCountPerType};

  for (i = 0; i < remainder; i++) {
    pieceCount[i]++;
  }

  srand(time(NULL));

  for (i = 0; i < board->boyut; i++) {
    for (j = 0; j < board->boyut; j++) {
      if (IsMiddleOfBoard(board->boyut, i, j)) {
        continue;
      }

      int pieceType = rand() % NUM_PIECE_TYPE;
      while (pieceCount[pieceType] == 0) {
        pieceType = (pieceType + 1) % NUM_PIECE_TYPE;
      }

      pieceCount[pieceType]--;

      board->data[i][j] = 'A' + pieceType;
    }
  }
}

GameBoard *createBoard(int boyut) { // tahta oluştur
  int i, j;
  if (boyut > MAX_SIZE) {
    boyut = MAX_SIZE;
  }
  GameBoard *board = (GameBoard *)malloc(sizeof(GameBoard));
  board->boyut = boyut;
  board->data = (char **)malloc(sizeof(char *) * boyut);
  for (i = 0; i < boyut; i++) {
    board->data[i] = (char *)malloc(sizeof(char) * boyut);
    for (j = 0; j < boyut; j++) {
      board->data[i][j] = '.';
    }
  }
  return board;
}

void InitializeInfo(Info *info, int gameMode, int size) {
  int i, j;
  for (i = 0; i < PLAYER_COUNT; i++) {
    for (j = 0; j < NUM_PIECE_TYPE; j++) {
      info->score[i][j] = 0;
    }
    info->complete_sets[i] = 0; // Tam setleri sıfırla
  }

  info->turn = 0;
  info->gameMode = gameMode;

  info->moveMade = 0;

  info->board = createBoard(size);
  randomPlacement(info->board);
  info->undo_stack.top = -1;
  info->redo_stack.top = -1;
}

void printBoard(GameBoard *board) { // tahtayı yazdır
  int i, j;
  printf("\n  ");
  for (i = 0; i < board->boyut; i++) {
    printf("%d ", i);
  }
  printf("\n");
  for (i = 0; i < board->boyut; i++) {
    printf("%d ", i);
    for (j = 0; j < board->boyut; j++) {
      printf("%c ", board->data[i][j]);
    }
    printf("\n");
  }
}

void printScore(Info *info) { // skoru yazdır
  int i, j;
  printf("\nSkorlar:\n");
  for (i = 0; i < PLAYER_COUNT; i++) {
    printf("Oyuncu %d: ", i + 1);
    for (j = 0; j < NUM_PIECE_TYPE; j++) {
      printf("%c: %d ", 'A' + j, info->score[i][j]);
    }
    printf("Tam setler: %d\n",
           info->complete_sets[i]); // Tam setleri yazdır
    printf("\n");
  }
}

int savetofile(Info *info) { // dosyaya kaydetme
  int i, j;

  FILE *fp = fopen("save.txt", "w");
  if (fp == NULL) {
    return 0;
  }

  fprintf(fp, "%d\n", info->board->boyut);
  for (i = 0; i < info->board->boyut; i++) {
    for (j = 0; j < info->board->boyut; j++) {
      fprintf(fp, "%c", info->board->data[i][j]);
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "%d %d %d %d %d\n", info->gameMode, info->turn, info->moveMade,
          info->movedPiecePosition.x, info->movedPiecePosition.y);
  for (i = 0; i < PLAYER_COUNT; i++) {
    for (j = 0; j < NUM_PIECE_TYPE; j++) {
      fprintf(fp, "%d ", info->score[i][j]);
    }
    fprintf(fp, "%d ", info->complete_sets[i]); // Tam setleri dosyaya yaz
    fprintf(fp, "\n");
  }
  fclose(fp);

  return 1;
}

int readfromfile(
    Info *info) { // kayıtlı oyunu oynamak için dosyadan okuma yapan fonksiyon
  int i, j;
  int boyut;

  FILE *fp = fopen("save.txt", "r");
  if (fp == NULL) {
    return 0;
  }

  fscanf(fp, "%d\n", &boyut);

  GameBoard *board = createBoard(boyut);
  for (i = 0; i < boyut; i++) {
    for (j = 0; j < boyut; j++) {
      fscanf(fp, " %c", &board->data[i][j]);
    }
  }

  info->board = board;

  fscanf(fp, "%d %d %d %d %d\n", &info->gameMode, &info->turn, &info->moveMade,
         &info->movedPiecePosition.x, &info->movedPiecePosition.y);
  for (i = 0; i < PLAYER_COUNT; i++) {
    for (j = 0; j < NUM_PIECE_TYPE; j++) {
      fscanf(fp, "%d", &info->score[i][j]);
    }
    fscanf(fp, "%d", &info->complete_sets[i]); // Tam setleri dosyadan oku
  }
  fclose(fp);

  return 1;
}

void pushMove(MoveStack *stack, Move move) {
  if (stack->top < STACK_SIZE - 1) {
    stack->moves[++stack->top] = move;
  }
}

Move popMove(MoveStack *stack) {
  if (stack->top >= 0) {
    return stack->moves[stack->top--];
  }
  Move empty_move = {0};
  return empty_move;
}

int isValidMove(GameBoard *board, int x1, int y1, int x2, int y2) {
  if (board->data[x1][y1] == '.' || board->data[x2][y2] != '.') {
    return 0;
  }
  int abs_x = abs(x2 - x1);
  int abs_y = abs(y2 - y1);
  if (!((abs_x == 2 && abs_y == 0) || (abs_x == 0 && abs_y == 2))) {
    return 0;
  }
  int avg_x = (x1 + x2) / 2;
  int avg_y = (y1 + y2) / 2;
  if (board->data[avg_x][avg_y] == '.') {
    return 0;
  }
  return 1;
}

int makeMove(Info *info, Move *move) { // hamleyi yaptıran fonksiyon
  int i;
  if (!isValidMove(info->board, move->x1, move->y1, move->x2, move->y2)) {
    printf("İlk yer boş %c / son yer dolu %c\n",
           info->board->data[move->x1][move->y1],
           info->board->data[move->x2][move->y2]);
    return 0;
  }

  int avg_x = (move->x1 + move->x2) / 2;
  int avg_y = (move->y1 + move->y2) / 2;

  move->captured_piece = info->board->data[avg_x][avg_y];

  info->board->data[move->x2][move->y2] = info->board->data[move->x1][move->y1];
  info->board->data[move->x1][move->y1] = '.';
  info->board->data[avg_x][avg_y] = '.';

  pushMove(&info->undo_stack, *move);
  info->redo_stack.top = -1;

  char pieceType = move->captured_piece;
  info->score[info->turn][pieceType - 'A']++;
  info->moveMade++;

  // Tam set kontrolü
  int set_complete = 1;
  for (i = 0; i < NUM_PIECE_TYPE; i++) {
    if (info->score[info->turn][i] == 0) {
      set_complete = 0;
      break;
    }
  }
  if (set_complete) {
    info->complete_sets[info->turn]++;
    for (i = 0; i < NUM_PIECE_TYPE; i++) {
      info->score[info->turn][i]--;
    } // tamamlanmış setleri doğru sayabilmek için seti artırdıktan sonra tüm
      // taşların skorlarını bir azaltıyoruz
  }

  return 1;
}

void undoMove(Info *info) { // yapılan hamleyi geri alan
  int i;
  if (info->undo_stack.top < 0) {
    printf("Geri alacak hamle yok.\n");
    return;
  }
  Move move = popMove(&info->undo_stack);
  int mid_x = (move.x1 + move.x2) / 2;
  int mid_y = (move.y1 + move.y2) / 2;

  info->board->data[move.x1][move.y1] = info->board->data[move.x2][move.y2];
  info->board->data[move.x2][move.y2] = '.';
  info->board->data[mid_x][mid_y] = move.captured_piece;

  info->score[1 - info->turn][move.captured_piece - 'A']--;

  pushMove(&info->redo_stack, move);
  info->moveMade--;

  int set_complete = 1;
  for (i = 0; i < NUM_PIECE_TYPE; i++) {
    if (info->score[1 - info->turn][i] == 0) {
      set_complete = 0;
      break;
    }
  }
  if (set_complete) {
    info->complete_sets[1 - info->turn]--;
  }
}

void redoMove(Info *info) {
  if (info->redo_stack.top < 0) {
    printf("İleri alacak hamle yok.\n");
    return;
  }
  Move move = popMove(&info->redo_stack);
  makeMove(info, &move);
}

Move findBestMove(GameBoard *board) {
  // bilgisayarın algoritması
  int i, j, x1, x2, y2, y1;
  Move bestMove = {0};
  for (x1 = 0; x1 < board->boyut; x1++) {
    for (y1 = 0; y1 < board->boyut; y1++) {
      if (board->data[x1][y1] != '.') {
        int possible_moves[4][2] = {
            {x1 + 2, y1}, {x1 - 2, y1}, {x1, y1 + 2}, {x1, y1 - 2}};
        for (i = 0; i < 4; i++) {
          x2 = possible_moves[i][0];
          y2 = possible_moves[i][1];
          if (x2 >= 0 && x2 < board->boyut && y2 >= 0 && y2 < board->boyut) {
            if (isValidMove(board, x1, y1, x2, y2)) {
              bestMove.x1 = x1;
              bestMove.y1 = y1;
              bestMove.x2 = x2;
              bestMove.y2 = y2;
              return bestMove;
            }
          }
        }
      }
    }
  }
  return bestMove;
}

void computerMove(Info *info) { // Bilgisayarın hamle yapmasını sağlar
  Move bestMove = findBestMove(info->board);
  if (makeMove(info, &bestMove)) {
    printf("Bilgisayar hamle yaptı: (%d, %d) -> (%d, %d)\n", bestMove.x1,
           bestMove.y1, bestMove.x2, bestMove.y2);
  } else {
    printf("Bilgisayar geçerli bir hamle yapamadı.\n");
  }
  info->turn = 1 - info->turn;
}

int isGameOver(Info *info) { // Oyunun bitip bitmediğini kontrol eden fonksiyon
  int i, j, k;
  for (i = 0; i < info->board->boyut; i++) {
    for (j = 0; j < info->board->boyut; j++) {
      if (info->board->data[i][j] != '.') {
        int possible_moves[4][2] = {
            {i + 2, j}, {i - 2, j}, {i, j + 2}, {i, j - 2}};
        for (k = 0; k < 4; k++) {
          int x2 = possible_moves[k][0];
          int y2 = possible_moves[k][1];
          if (x2 >= 0 && x2 < info->board->boyut && y2 >= 0 &&
              y2 < info->board->boyut) {
            if (isValidMove(info->board, i, j, x2, y2)) {
              return 0; // Hamle yapılabiliyor, Oyun bitmez
            }
          }
        }
      }
    }
  }
  return 1; // Yapılabilecek hamle kalmadı, oyun bitti
}

int main(void) {
  Move move;
  Info info;
  int boyut;
  int oyunModu, secim;
  int state = STATE_MENU;

  while (1) {
    switch (state) {
    case STATE_MENU: {
      printf("Yeni Oyuna Başla: 0\nKayıtlı Oyuna Devam Et: 1\nÇıkış yap: 2\n");
      scanf("%d", &secim);
      getchar();
      if (secim == 0) {
        state = STATE_NEW_GAME;
      } else if (secim == 1) {
        if (!readfromfile(&info)) {
          printf("Okuma yapılamadı\n");
        } else {
          state = STATE_PLAY;
        }
      } else if (secim == 2) {
        exit(0);
      }
    } break;

    case STATE_NEW_GAME: {
      do {
        printf("Oyun tahtasinin boyutunu girin: 6-20\n");
        scanf("%d", &boyut);
      } while (boyut < 6 || boyut > 20);

      do {
        printf("Oyun Modu Seçiniz:\n 1. İki Kişilik 2. Bilgisayara Karşı\n");
        scanf("%d", &oyunModu);
      } while (oyunModu != 1 && oyunModu != 2);

      InitializeInfo(&info, oyunModu, boyut);

      state = STATE_PLAY;
    } break;

    case STATE_PLAY: {
      printBoard(info.board);
      printScore(&info);
      printf(
          "Oyunu kaydedip çıkmak için X'e,\nTuru pas geçmek için P'ye "
          "basın\nDevam etmek için hamle yapılacak taşın ve hamle yapılacak "
          "bölmenin koordinatlarını giriniz (x1 y1 x2 y2)\nYapılan hamleyi "
          "geri almak için U,\nGeri alınan hamleyi yinelemek için R basın.\n");
      char inputBuffer[101] = {0};
      fgets(inputBuffer, 100, stdin);

      if (inputBuffer[0] == 'X' || inputBuffer[0] == 'x') {
        if (!savetofile(&info)) {
          printf("Dosya kaydedilemedi\n");
        }
        state = STATE_MENU;
      } else if (inputBuffer[0] == 'P' || inputBuffer[0] == 'p') {
        info.turn = 1 - info.turn;
        if (info.gameMode == 2 && info.turn == 1) {
          computerMove(&info);
        }
      } else if (inputBuffer[0] == 'U' || inputBuffer[0] == 'u') {
        undoMove(&info);
      } else if (inputBuffer[0] == 'R' || inputBuffer[0] == 'r') {
        redoMove(&info);
      } else if (sscanf(inputBuffer, "%d %d %d %d", &move.x1, &move.y1,
                        &move.x2, &move.y2) == 4) {
        if (!makeMove(&info, &move)) {
          printf("Geçersiz hamle\n");
        } else if (info.gameMode == 2 && info.turn == 1) {
          computerMove(&info);
        }
      } else {
        printf("Geçersiz girdi\n");
      }

      if (isGameOver(&info)) {
        printf("Oyun bitti!\n");
        printScore(&info);
        state = STATE_MENU;
      }
    } break;
    }
  }

  return 0;
}
