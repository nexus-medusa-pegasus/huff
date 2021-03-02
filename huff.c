#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define NUM_ASC 256

typedef struct _Heap_Node {
  char c;
  int freq;
  struct _Heap_Node *left;
  struct _Heap_Node *right;
} Node;

Node **heap;
int lastHeapIdx = 0;
char codeBuf[BUF_SIZE];
int codeBufIdx = -1;

int charFreq[NUM_ASC];
char *symCode[NUM_ASC];

void performEncoding(char *fName);
void performDecoding(char *fName);
void showCharFrequency();
int countNonZeroCharacters();
void addToHeap(Node *cur);
Node *deleteFromHeap();
void traverse(Node *cur, char c);

int main(int argc, char *argv[]) {

  performEncoding(argv[1]);
  performDecoding(argv[2]);

  return 0;
}

void performEncoding(char *fName) {
  FILE *fin, *fout;
  char buf[BUF_SIZE];
  char encodedFileName[BUF_SIZE];
  char writeBuf[BUF_SIZE];
  int len, i, cnt, numOfSym;
  Node *cur, *first, *second, *newOne;

  if (!(fin = fopen(fName, "rt"))) {
    fprintf(stderr, "Unable to open %s. Program terminated. \n", fName);
    exit(EXIT_FAILURE);
  }

  memset(charFreq, 0, NUM_ASC * sizeof(int));
  while (fgets(buf, BUF_SIZE, fin)) {
    len = strlen(buf);
    for (i = 0; i < len; i++) {
      charFreq[(int)buf[i]]++;
    }
  }

  cnt = countNonZeroCharacters();
  heap = (Node **)malloc(sizeof(Node *) * (cnt + 1));
  memset(heap, 0, sizeof(Node *) * (cnt + 1));

  for (i = 0; i < NUM_ASC; i++) {
    if (charFreq[i] > 0) {
      cur = (Node *)malloc(sizeof(Node));
      cur->c = (char)i;
      cur->freq = charFreq[i];
      cur->left = cur->right = NULL;
      addToHeap(cur);
    }
  }
  
  fclose(fin);

  while (1) {
    first = deleteFromHeap();
    second = deleteFromHeap();

    if (!second) {
      break;
    }

    newOne = (Node *)malloc(sizeof(Node));
    newOne->c = '\0';
    newOne->freq = first->freq + second->freq;
    newOne->left = first;
    newOne->right = second;

    addToHeap(newOne);
  }

  memset(symCode, 0, sizeof(symCode));

  traverse(first->left, '0');
  traverse(first->right, '1');

  for (i = 0; i < NUM_ASC; i++) {
      if (symCode[i]) {
        numOfSym++;
      }
  }

  strcpy(encodedFileName, fName);
  strcat(encodedFileName, ".encoded");

  if (!(fout = fopen("encoded", "wb"))) {
    fprintf(stderr, "Unable to open file: %s \n", encodedFileName);
    exit(EXIT_FAILURE);
  }

  fwrite(&numOfSym, sizeof(numOfSym), 1, fout);
  
  for (i = 0; i < NUM_ASC; i++) {
    if (symCode[i]) {
      writeBuf[0] = (char)i;
      writeBuf[1] = (char)strlen(symCode[i]);
      strcpy(&writeBuf[2], symCode[i]);
      fwrite(&writeBuf, sizeof(char), strlen(symCode[i]) + 2, fout);
    }
  }

  fclose(fout);
}

void justTraverse(Node *cur, char c) {
  char *hufCode;

  codeBufIdx++;
  codeBuf[codeBufIdx] = c;
  codeBuf[codeBufIdx + 1] = '\0';

  if (!(cur->left == 0 && cur->right == 0)) {
    traverse(cur->left, '0');
    traverse(cur->right, '1');
  }

  codeBuf[codeBufIdx] = '\0';
  codeBufIdx--;
}

void traverse(Node *cur, char c) {
  char *hufCode;

  codeBufIdx++;
  codeBuf[codeBufIdx] = c;
  codeBuf[codeBufIdx + 1] = '\0';

  if (cur->left == 0 && cur->right == 0) {
    hufCode = (char *)malloc(sizeof(char) * (strlen(codeBuf) + 1));
    strcpy(hufCode, codeBuf);
    symCode[(int)cur->c] = hufCode;
  } else {
    traverse(cur->left, '0');
    traverse(cur->right, '1');
  }

  codeBuf[codeBufIdx] = '\0';
  codeBufIdx--;
}

Node *deleteFromHeap() {
  Node *retVal = heap[1];
  int parent = 1;
  int left = parent * 2;
  int right = left + 1;
  int smaller;
  Node *temp;

  if (lastHeapIdx <= 0) {
    return NULL;
  }

  heap[1] = heap[lastHeapIdx];
  lastHeapIdx--;

  while (1) {
    if (left > lastHeapIdx) {
      break;
    } else if (right > lastHeapIdx) {
      if (heap[left]->freq < heap[parent]->freq) {
        temp = heap[left];
        heap[left] = heap[parent];
        heap[parent] = temp;

        parent = left;
        left = parent * 2;
        right = left + 1;
      } else {
        break;
      }
    } else {
      smaller = (heap[left]->freq <= heap[right]->freq) ? left : right;

      if (heap[smaller]->freq < heap[parent]->freq) {
        temp = heap[smaller];
        heap[smaller] = heap[parent];
        heap[parent] = temp;

        parent = smaller;
        left = parent * 2;
        right = left + 1;
      } else {
        break;
      }
    }
  }

  return retVal;
}

void addToHeap(Node *cur) {
  lastHeapIdx++;
  heap[lastHeapIdx] = cur;

  int currentIdx = lastHeapIdx;
  int parentIdx = currentIdx / 2;
  Node *temp;

  while (parentIdx >= 1) {
    if (heap[parentIdx]->freq > heap[currentIdx]->freq) {
      temp = heap[parentIdx];
      heap[parentIdx] = heap[currentIdx];
      heap[currentIdx] = temp;

      currentIdx = parentIdx;
      parentIdx = currentIdx / 2;
    } else {
      break;
    }
  }
}

int countNonZeroCharacters() {
  int cnt, i;

  for (cnt = i = 0; i < NUM_ASC; i++) {
    if (charFreq[i] > 0) {
      cnt++;
    }
  }

  return cnt;
}

void performDecoding(char *fName) {
  FILE *fin;
  int numOfSym, i;
  char symbolAndLen[2];
  char buf[BUF_SIZE];
  Node *huffRoot, *cur;

  if (!(fin = fopen(fName, "rb"))) {
    fprintf(stderr, "Unable to open file: %s \n", fName);
    exit(EXIT_FAILURE);
  }

  numOfSym = 0;
  fread(&numOfSym, sizeof(int), 1, fin);

  huffRoot = (Node *)malloc(sizeof(Node));
  huffRoot->left = huffRoot->right = NULL;
  cur = huffRoot;

  for (i = 0; i < numOfSym; i++) {
    fread(symbolAndLen, 2, 1, fin);
    fread(buf, 1, (int)symbolAndLen[1], fin);
    buf[(int)symbolAndLen[1]] = '\0';

    cur = huffRoot;
    for (i = 0; i < (int)symbolAndLen[1]; i++) {
      if (buf[i] == '0') {
        if (cur->left == 0) {
          cur->left = (Node *)malloc(sizeof(Node));
          cur->left->left = cur->left->right = NULL;
        }
        cur = cur->left;
      } else if (buf[i] == '1') {
        if (cur->right == 0) {
          cur->right = (Node *)malloc(sizeof(Node));
          cur->right->left = cur->right->right = NULL;
        }
        cur = cur->right;
      } else {
        fprintf(stderr, "Cannot decode file \n");
        exit(EXIT_FAILURE);
      }
    }
    cur->c = symbolAndLen[0];
  }

  codeBufIdx--;
  justTraverse(huffRoot->left, '0');
  justTraverse(huffRoot->right, '1');

  fclose(fin);
}

