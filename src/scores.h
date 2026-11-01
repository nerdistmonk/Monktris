// src/scores.h
#ifndef SCORES_H
#define SCORES_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char name[4];
    uint32_t score;
} HighScoreEntry;

void scores_init(void);
bool scores_qualifies(uint32_t score);
void scores_insert(const char* name, uint32_t score);
const HighScoreEntry* scores_get(int index);

#endif // SCORES_H