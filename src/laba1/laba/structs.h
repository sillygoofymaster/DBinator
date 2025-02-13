#ifndef STRUCTS_H
#define STRUCTS_H
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#define MAX_FIELD_LENGTH 40
#define MAX_MASTER_FIELDS 20
#define MAX_TRASH_FIELDS 2
#define DEFAULT_PARTICIPATION { -1, -1, -1, true, -1, -1 }
#define DEFAULT_PLAYER {-1, true, " ", " ", " ", -1, -1, -1, -1}

typedef struct {
	int32_t playerId;
	bool isDeleted;
	char name[MAX_FIELD_LENGTH];
	char country[MAX_FIELD_LENGTH];
	char title[MAX_FIELD_LENGTH];
	int32_t elo;
	int32_t totalGames;
	int32_t participationCount;
	long firstParticipation; // adress
} Player;

typedef struct {
	int32_t participationId;
	int32_t playerId;
	int32_t tournamentId;
	bool isDeleted;
	int32_t score;
	long nextParticipation; // adress
} TournamentParticipation;

typedef struct {
	int32_t playerId;
	long position;
} Index;
#endif 