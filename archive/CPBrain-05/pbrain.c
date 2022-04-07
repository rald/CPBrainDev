#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define MEM_MAX 30000
#define PROC_MAX 256
#define STACK_MAX 1024

typedef struct Pos {
  size_t col;
  size_t row;
} Pos;

char *slurp(char *path) {
  char *source = NULL;

  FILE *fp = fopen(path, "rb");
  if(fp == NULL) {
    perror("Error fopen");
    return NULL;
  }

  if(fseek(fp, 0L, SEEK_END) != 0) {
    perror("Error fseek");
    goto terminate;
  }

  long bufsize = ftell(fp);
  if(bufsize == -1) {
    perror("Error ftell");
    goto terminate;
  }

  if(fseek(fp, 0L, SEEK_SET) != 0) {
    perror("Error fseek");
    goto terminate;
  }

  source = malloc(sizeof(char) * (bufsize + 1));
  if(source == NULL) {
    perror("Error malloc");
    goto terminate;
  }

  size_t newLen = fread(source, sizeof(char), bufsize, fp);
  if(newLen == 0) {
    free(source);
    source = NULL;
    perror("Error fread");
    goto terminate;
  }

  source[newLen] = '\0';

terminate:

  fclose(fp);

  return source;
}

Pos getpos(const char *code, size_t p) {
  Pos pos;

  pos.row=1;
  pos.col=1;

  size_t cp=0;

  if(code[cp]) {
    while(code[cp] && cp<=p) {
      if(code[cp]=='\n') {
        pos.row++;
        pos.col=1;
      } else {
        pos.col++;
      }
      cp++;
    }
    pos.col--;
  }

  return pos;
}

void error(const char *code, size_t p, const char *fmt, ...) {

  va_list args;

  Pos pos=getpos(code, p);

  fprintf(stderr, "Error Line %zu Column %zu: ", pos.row, pos.col);

  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);

  fprintf(stderr, "\n");

}

int run(const char *code) {

  unsigned char mem[MEM_MAX];
  ssize_t proc[PROC_MAX];
  size_t stack[STACK_MAX];
  size_t pstack[STACK_MAX];

  size_t cp = 0;
  size_t mp = 0;
  size_t sp = 0;
  ssize_t d = 0;

  for(size_t i = 0; i < MEM_MAX; i++)
    mem[i] = 0;

  for(size_t i = 0; i < PROC_MAX; i++)
    proc[i] = -1;

  while(code[cp]) {
    switch(code[cp]) {
    case '(':
    case '[':
      stack[sp]=code[cp];
      pstack[sp]=cp;
      if(sp<STACK_MAX) {
        sp++;
      } else {
        error(code, cp, "too deep");
      }
      break;
    case ')':
      if(sp>0) {
        sp--;
        if(stack[sp]!='(') {
          error(code, pstack[sp], "unbalanced %c", stack[sp]);
          return 1;
        }
      } else {
        error(code, cp, "unbalanced )");
        return 1;
      }
      break;
    case ']':
      if(sp>0) {
        sp--;
        if(stack[sp]!='[') {
          error(code, pstack[sp], "unbalanced %c", stack[sp]);
          return 1;
        }
      } else {
        error(code, cp, "unbalanced ]");
        return 1;
      }
      break;
    }
    cp++;
  }

  if(sp>0) {
    sp--;
    error(code, pstack[sp], "unbalanced %c", stack[sp]);
    return 1;
  }

  sp=0;
  cp=0;

  while(code[cp]) {
    switch(code[cp]) {
    case '.':
      putchar(mem[mp]);
      break;
    case ',': {
      int ch = getchar();
      if(ch == EOF)
        mem[mp] = 0;
      else
        mem[mp] = ch;
    }
    break;
    case '+':
      if(mem[mp] == 255)
        mem[mp] = 0;
      else
        mem[mp]++;
      break;
    case '-':
      if(mem[mp] == 0)
        mem[mp] = 255;
      else
        mem[mp]--;
      break;
    case '>':
      if(mp == 65535) {
        error(code, cp, "memory overflow");
        return 1;
      } else
        mp = mp + 1;
      break;
    case '<':
      if(mp == 0) {
        error(code, cp, "memory underflow");
        return 1;
      } else
        mp = mp - 1;
      break;
    case '[':
      if(mem[mp] == 0) {
        d = 1;
        while(code[cp] && d != 0) {
          cp++;
          d += (code[cp] == '[') - (code[cp] == ']');
        }
      }
      break;
    case ']':
      if(mem[mp] != 0) {
        d = 1;
        while(cp > 0 && d != 0) {
          cp--;
          d += (code[cp] == ']') - (code[cp] == '[');
        }
      }
      break;
    case '(':
      proc[mem[mp]] = cp;
      d = 1;
      while(code[cp] && d != 0) {
        cp++;
        d += (code[cp] == '(') - (code[cp] == ')');
      }
      break;
    case ')':
      if(sp > 0)
        cp = stack[--sp];
      else {
        error(code, cp, "stack underflow");
        return 1;
      }
      break;
    case ':':
      if(sp < 256)
        stack[sp++] = cp;
      else {
        error(code, cp, "stack overflow");
        return 1;
      }
      if(proc[mem[mp]] != -1)
        cp = proc[mem[mp]];
      else {
        error(code, cp, "calling undefined proc %d", mem[mp]);
        return 1;
      }
      break;
    case '#':
      cp++;
      while(code[cp] && code[cp] != '\n')
        cp++;
      break;
    case '/':
      cp++;
      if(code[cp] == '/') {
        while(code[cp] && code[cp] != '\n')
          cp++;
      } else if(code[cp] == '*') {
        while(code[cp] && (code[cp] != '*' || code[cp + 1] != '/'))
          cp++;
      }
      break;
    }
    cp++;
  }

  return 0;
}

int main(int argc, char **argv) {

  if(argc != 2) {
    fprintf(stderr, "Syntax: %s filename.pb\n", argv[0]);
    return 1;
  }

  char *code = slurp(argv[1]);

  if(code!=NULL) {
    run(code);
    free(code);
    code = NULL;
  }

  return 0;
}
