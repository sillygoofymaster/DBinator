#include "structs.h"

void read_str(char s[], int maxlen) {
	char ch;
	int i;
	int chars_remain;

	i = 0;
	chars_remain = 1;
	while (chars_remain) {
		ch = getchar();
		if ((ch == '\n') || (ch == EOF)) {
			chars_remain = 0;
		}
		else if (i < maxlen - 1) {
			s[i] = ch;
			i++;
		}
	}
	s[i] = '\0';
	return;
}

int read_int(char* format, int* input) {
	if (scanf_s(format, input) != 1 || input < 0) {
		printf("> Invalid input. <\n");
		while (getchar() != '\n');
		return -1;
	}
	while (getchar() != '\n');
	return 1;
}

FILE* open_file(const char* filename, const char* mode) {
	FILE* file = fopen(filename, mode);
	if (file == NULL) {
		perror("Failed to open file");
		exit(1);
	}
	return file;
}

int compare_by_id(const void* a, const void* b) { // for qsort
	Index* index_a = (Index*)a;
	Index* index_b = (Index*)b;
	if (index_a->playerId < index_b->playerId) return -1;
	if (index_a->playerId > index_b->playerId) return 1;
	return 0;
}

int get_position(Index arr[], int low, int high, int32_t x)
{
	while (low <= high) {
		int mid = low + (high - low) / 2;

		if (arr[mid].playerId == x)
			return mid; 


		if (arr[mid].playerId < x)
			low = mid + 1;
		else
			high = mid - 1;
	}

	printf("> Invalid ID. <\n");
	return -1; 
}

void print_participation_header() {
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
	printf("|            |                    | Participation ID   | Player ID          | Tournament | Score      |\n");
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
}


void print_participation(TournamentParticipation* part) {
	printf("| %-10s | %-18s | %-18d | %-18d | %-10d | %-10d |\n",
		"          ", "                  ",
		part->participationId,
		part->playerId,
		part->tournamentId,
		part->score);
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
	return;
}

int32_t read_mindex_table(Index* trash, Index* index_table, char* filename) {	// get trash, maxindex and index_table, return maxid
	FILE* mindex_file = open_file(filename, "rb");

	Index buffindx;
	for (int i = 0; i < 2; i++) {
		if (fread(&buffindx, sizeof(Index), 1, mindex_file) != 1) {
			buffindx.playerId = -1; buffindx.position = -1;
		}
		trash[i] = buffindx;
	}

	int i = 0;
	int m_icount = 0;
	while (fread(&buffindx, sizeof(Index), 1, mindex_file)) {
		if (i >= 20) {
			fprintf(stderr, "Error reading index table from the file. The database currently supports up to 20 rows.\n");
			fclose(mindex_file);
			exit(1);											// exit if the table contains more than 20 rows
		}
		if (buffindx.playerId != -1) m_icount++;
		index_table[i] = buffindx;
		i++;
	}
	while (i <= 19) {
		index_table[i].playerId = -1;
		index_table[i].position = -1;							// fill empty slots with { -1, -1 }
		i++;
	}
	qsort(index_table, 20, sizeof(Index), compare_by_id);

	fclose(mindex_file);
	return m_icount;
}

void save_mindex_table(Index* trash, Index* index_table) {
	FILE* mindex_file = open_file("mi.bin", "wb");
	fwrite(&trash[0], sizeof(Index), 1, mindex_file);
	fwrite(&trash[1], sizeof(Index), 1, mindex_file);
	for (int i = 0; i < MAX_MASTER_FIELDS; i++) {
		fwrite(&index_table[i], sizeof(Index), 1, mindex_file);
	}
}

void print_player_header() {
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
	printf("| Player ID | Name               | Country            | Title              | Elo        | Total Games |\n");
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
}

void print_player(Player* player) {
	printf("| %-10d | %-18s | %-18s | %-18s | %-10d | %-10d |\n",
		player->playerId,
		player->name,
		player->country,
		player->title,
		player->elo,
		player->totalGames);
	printf("+------------+--------------------+--------------------+--------------------+------------+------------+\n");
}

void get_m(Player* buffer, long position) { // reads player from position
	FILE* master_file = open_file("m.bin", "rb");
	fseek(master_file, position, SEEK_SET);
	fread(buffer, sizeof(Player), 1, master_file);
	fclose(master_file);
}

void get_player_participations(Player* player, char* filename) {
	if (player->participationCount < 1) {
		printf("	> This player has no recorded tournament appearances. <\n");
		return;
	}

	TournamentParticipation participation;
	FILE* slave = open_file(filename, "rb");
	fseek(slave, player->firstParticipation, SEEK_SET);
	print_participation_header();
	while (fread(&participation, sizeof(TournamentParticipation), 1, slave)) {
		print_participation(&participation);
		if (participation.nextParticipation == -1) break;
		fseek(slave, participation.nextParticipation, SEEK_SET);
	}
	printf("	> This player has %d recorded tournament appearances. <\n", player->participationCount);
	return;
} 

int32_t get_player_editing_choice() {
	int32_t option;
	printf("> Editing options:\n");
	printf("	(1) Name; (2) Country;\n");
	printf("	(3) Title; (4) Elo;\n");
	printf("	(5) totalGames. \n");
	printf("	> Any other number aborts the operation.\n");
	printf("	> Pease enter the number of the command you wish to execute: ");
	read_int("%d", &option);
	return option;
}

void update_name(Player* player) {
	char cinput[MAX_FIELD_LENGTH];
	printf("	> Please enter new name: ");
	read_str(cinput, MAX_FIELD_LENGTH);
	snprintf(player->name, sizeof(player->name), "%s", cinput);
}

void update_country(Player* player) {
	char cinput[MAX_FIELD_LENGTH];
	printf("	> Please enter new country: ");
	read_str(cinput, MAX_FIELD_LENGTH);
	snprintf(player->country, sizeof(player->country), "%s", cinput);
}

void update_title(Player* player) {
	char cinput[MAX_FIELD_LENGTH];
	printf("	> Please enter new title: ");
	read_str(cinput, MAX_FIELD_LENGTH);
	snprintf(player->title, sizeof(player->title), "%s", cinput);
}

int update_elo(Player* player) {
	int32_t input;
	printf("	> Please enter new elo: ");
	if (read_int("%d", &input) == 1) {
		player->elo = input;
		return 1;
	}
	return -1;
}

int update_total_games(Player* player) {
	int32_t input;
	printf("	> Please enter updated amount of player's games: ");
	if (read_int("%d", &input) == 1) {
		player->totalGames = input;
		return 1;
	}
	return -1;
}

void update_m(long position, int32_t option, Player* player) { //index_table??
	char cinput[MAX_FIELD_LENGTH];
	int32_t input;

	switch (option) {
	case 1:
		update_name(player);
		break;

	case 2: 
		update_country(player);
		break;

	case 3: 
		update_title(player);
		break;

	case 4:
		if (update_elo(player) == -1) return;
		break;

	case 5:
		if (update_total_games(player) == -1) return;
		break;

	default:
		break;;
	}

	FILE* master_file = open_file("m.bin", "r+b");
	fseek(master_file, position, SEEK_SET);
	fwrite(player, sizeof(Player), 1, master_file);
	fclose(master_file);

	return;
}

void ut_m(char* filename) {
	FILE* master_file = open_file(filename, "rb");

	Player player;
	print_player_header();
	while (fread(&player, sizeof(Player), 1, master_file) == 1) {
		if (!player.isDeleted) print_player(&player);
	}

	fclose(master_file);
	return;
}

int32_t read_and_get_s_max_id(char* filename, bool ut_s) { // ut_s or retrieve max index, since slave index file is not maintained
	TournamentParticipation participation;
	FILE* slave_file = open_file(filename, "rb");
	int32_t maxid = -1;
	if (ut_s) print_participation_header();
	while (fread(&participation, sizeof(TournamentParticipation), 1, slave_file)) {
		if (participation.participationId > maxid) maxid = participation.participationId;
		if (ut_s && !participation.isDeleted) print_participation(&participation);
	}

	fclose(slave_file);
	return maxid;
}

long get_s(TournamentParticipation* buffer, Player* player, char* filename, int32_t tourid) { // returns participation's position

	FILE* slave = open_file(filename, "rb");
	long position = -1;
	fseek(slave, player->firstParticipation, SEEK_SET);
	for (int i = 0; i < player->participationCount; i++) {
		fread(buffer, sizeof(TournamentParticipation), 1, slave);
		if (!buffer->isDeleted && buffer->tournamentId == tourid) {
			position = ftell(slave) - sizeof(TournamentParticipation);
			fclose(slave);
			return position;
		}
		if (buffer->nextParticipation == -1) break;
		fseek(slave, buffer->nextParticipation, SEEK_SET);
	}
	fclose(slave);
	buffer->participationId = -1;
	position = -1;
	return position;
}

long find_earliest_deleted(FILE* slave) {
	fseek(slave, 0, SEEK_SET);
	long first_d_pos = 0;
	TournamentParticipation temp = DEFAULT_PARTICIPATION;

	while (fread(&temp, sizeof(TournamentParticipation), 1, slave) && !temp.isDeleted) {
		first_d_pos = ftell(slave);
	}

	return first_d_pos;
}

long insert_s(char* filename, int32_t maxid, Player* player, int32_t tourid) {
	int32_t input;

	TournamentParticipation participation;
	get_s(&participation, player, filename, tourid);
	if (participation.participationId != -1) {
		printf("	> This participation has been already recorded. <\n");
		return -1;
	}
	participation.tournamentId = tourid;

	printf("	> Please enter score: ");
	if (read_int("%d", &input) != 1) return -1;
	participation.score = input;

	participation.isDeleted = false;
	participation.nextParticipation = player->firstParticipation;

	participation.playerId = player->playerId;
	participation.participationId = maxid + 1;

	FILE* slave_file = open_file(filename, "r+b");
	long position = find_earliest_deleted(slave_file);
	fseek(slave_file, position, SEEK_SET);
	fwrite(&participation, sizeof(TournamentParticipation), 1, slave_file);
	fclose(slave_file);

	return position;
}

void update_s(TournamentParticipation* participation, long part_position, Player* player, char* filename) {
	FILE* slave = open_file(filename, "r+b");
	fseek(slave, part_position, SEEK_SET);
	fwrite(participation, sizeof(TournamentParticipation), 1, slave);
	fclose(slave);
	return;
}

long find_latest_player(FILE* master) {
	Player buff;
	fseek(master, 0, SEEK_END);
	long curr_pos = ftell(master) - sizeof(Player);
	fseek(master, curr_pos, SEEK_SET);
	while (fread(&buff, sizeof(Player), 1, master) && buff.isDeleted == true) {
		curr_pos -= sizeof(Player);
		fseek(master, curr_pos, SEEK_SET);
	}

	return curr_pos;
}

long find_earliest_deleted_player(FILE* master) {
	Player buff;
	fseek(master, 0, SEEK_SET);
	while (fread(&buff, sizeof(Player), 1, master)) {
		if (buff.isDeleted) return (ftell(master) - sizeof(Player));
	}
	fseek(master, 0, SEEK_END);
	return ftell(master);
}

int insert_m(char* filename, Index* index_table, int32_t maxid, Index*trash) {
	Player player;

	update_name(&player);
	update_country(&player);
	update_title(&player);
	if (update_elo(&player) == -1) return -1;
	if (update_total_games(&player) == -1) return -1;

	player.playerId = maxid + 1;
	player.participationCount = 0;
	player.firstParticipation = -1;
	player.isDeleted = false;

	long position = -1;
	FILE* master = open_file(filename, "r+b");
	fseek(master, 0, SEEK_END);

	if (trash[0].playerId != -1) {
		position = trash[0].position;
		trash[0].playerId = -1;
	}
	else if (trash[1].playerId != -1) {
		position = trash[1].position;
		trash[1].playerId = -1;
	}
	else position = find_earliest_deleted_player(master);

	fseek(master, position, SEEK_SET);
	fwrite(&player, sizeof(Player), 1, master);
	fclose(master);

	index_table[0].playerId = player.playerId;
	index_table[0].position = position;
	qsort(index_table, 20, sizeof(Index), compare_by_id);
	return 1;
}

int calc_s(char* filename) {
	FILE* slave = open_file(filename, "rb");
	int res = 0;
	TournamentParticipation part;

	while (fread(&part, sizeof(TournamentParticipation), 1, slave)) {
		if (!part.isDeleted) res++;
	}

	return res;
}

long get_player(Index* index_table, char* filename, Player* buffer) {
	int32_t input;
	long position = -1;

	printf("	> Please enter player ID: ");
	if (read_int("%d", &input) != 1) return position;
	int index = get_position(index_table, 0, 19, input);
	if (index == -1) return -1;
	position = index_table[index].position;
	get_m(buffer, position);
	

	return position;
}

int32_t get_tourid() {
	int32_t input;
	printf("	> Please enter tournament ID: ");
	if (read_int("%d", &input) != 1) return -1;
	return input;
}

void update_deleted_node_dependencies(TournamentParticipation* participation_deleted, long part_d_pos, Player* d_player, FILE* slave) {

	if (d_player->firstParticipation != part_d_pos) {
		TournamentParticipation prev, temp_part = DEFAULT_PARTICIPATION;
		long prev_pos2 = -1;
		fseek(slave, d_player->firstParticipation, SEEK_SET);

		while (fread(&temp_part, sizeof(TournamentParticipation), 1, slave) && temp_part.participationId != participation_deleted->participationId) {
			prev = temp_part;
			prev_pos2 = ftell(slave) - sizeof(TournamentParticipation);
			fseek(slave, temp_part.nextParticipation, SEEK_SET);
		}


		prev.nextParticipation = participation_deleted->nextParticipation;
		fseek(slave, prev_pos2, SEEK_SET);
		fwrite(&prev, sizeof(TournamentParticipation), 1, slave);

	}
	else {
		d_player->firstParticipation = participation_deleted->nextParticipation;
	}

	d_player->participationCount--;
	d_player->totalGames--;
	return;
}

void mark_participation_deleted(FILE* slave, long position) {
	fseek(slave, position, SEEK_SET);
	TournamentParticipation deleted_marker = DEFAULT_PARTICIPATION;
	fwrite(&deleted_marker, sizeof(TournamentParticipation), 1, slave);
}

int swap_participations(TournamentParticipation* participation2, long position1, long position2, Player* player2, FILE* slave) {

	TournamentParticipation temp_part = DEFAULT_PARTICIPATION, prev_part2;
	int res = 0;

	// find & update 2prevpart
	if (player2->firstParticipation != position2) {
		fseek(slave, player2->firstParticipation, SEEK_SET);

		while (fread(&temp_part, sizeof(TournamentParticipation), 1, slave)) {
			prev_part2 = temp_part;
			if (temp_part.nextParticipation == position2) break;
			fseek(slave, temp_part.nextParticipation, SEEK_SET);
		}

		prev_part2.nextParticipation = position1;
		long prev_pos2 = ftell(slave) - sizeof(TournamentParticipation);
		fseek(slave, prev_pos2, SEEK_SET);
		fwrite(&prev_part2, sizeof(TournamentParticipation), 1, slave);
	}
	else {
		player2->firstParticipation = position1;
		res = 1;
	}

	fseek(slave, position1, SEEK_SET);
	fwrite(participation2, sizeof(TournamentParticipation), 1, slave);

	mark_participation_deleted(slave, position2);

	return res;
}

void del_s(FILE* slave, Player* deleted_participation_player, TournamentParticipation* deleted_participation, Index* index_table, long deleted_part_pos, long del_p_position) {

	update_deleted_node_dependencies(deleted_participation, deleted_part_pos, deleted_participation_player, slave);
	update_m(del_p_position, 6, deleted_participation_player);

	Player moved_player;
	TournamentParticipation moved_participation = DEFAULT_PARTICIPATION;
	long moved_part_position = find_earliest_deleted(slave) - sizeof(TournamentParticipation);
	if (moved_part_position == deleted_part_pos) {
		mark_participation_deleted(slave, deleted_part_pos);
		return;
	}

	fseek(slave, moved_part_position, SEEK_SET);
	fread(&moved_participation, sizeof(TournamentParticipation), 1, slave);
	int index = get_position(index_table, 0, 19, moved_participation.playerId);
	long moved_player_position = index_table[index].position;
	get_m(&moved_player, moved_player_position);

	if (swap_participations(&moved_participation, deleted_part_pos, moved_part_position, &moved_player, slave) == 1) {
		update_m(moved_player_position, 6, &moved_player);
	}

	return;
}

void remove_del_pl_parts(Player* player, char* filename, long player_position, Index* index_table) {
	FILE* slave = open_file(filename, "r+b");
	TournamentParticipation temp;
	long cur_pos = -1;
	fseek(slave, player->firstParticipation, SEEK_SET);
	while (fread(&temp, sizeof(TournamentParticipation), 1, slave)) {
		cur_pos = ftell(slave) - sizeof(TournamentParticipation);
		del_s(slave, player, &temp, index_table, cur_pos, player_position);
		if (temp.nextParticipation == -1) break;
		fseek(slave, temp.nextParticipation, SEEK_SET);
	}

	fclose(slave);
	return;
}

void mark_player_spot_deleted(long position, FILE* master) {
	Player dummy = DEFAULT_PLAYER;
	fseek(master, position, SEEK_SET);
	fwrite(&dummy, sizeof(Player), 1, master);
	return;
}

void upd_table_aft_del(Index* index_table, int id) {
	int index = get_position(index_table, 0, 19, id);
	index_table[index].playerId = -1; index_table[index].position = -1;
	qsort(index_table, 20, sizeof(Index), compare_by_id);
}

long swap_upper_player(long del_position, FILE* master, Index* trash, Index* index_table, int indx) {
	Player mov_player;
	long moved_pl_position = find_latest_player(master);


	fseek(master, moved_pl_position, SEEK_SET);
	fread(&mov_player, sizeof(Player), 1, master);

	mark_player_spot_deleted(moved_pl_position, master);
	fseek(master, del_position, SEEK_SET);
	fwrite(&mov_player, sizeof(Player), 1, master);

	if (indx == 1) trash[indx].position = moved_pl_position;
	int moved_index = get_position(index_table, 0, 19, mov_player.playerId);
	index_table[moved_index].position = del_position;

	return moved_pl_position;
}

void del_m(Player* del_player, long del_position, Index* trash, char* master_file_name, char* slave_file_name, Index* index_table) {
	if (del_player->participationCount > 0) remove_del_pl_parts(del_player, slave_file_name, del_position, index_table);
	if (trash[0].playerId == -1) {
		trash[0].playerId = del_player->playerId; 
		trash[0].position = del_position;

		FILE* master = open_file(master_file_name, "r+b");
		mark_player_spot_deleted(del_position, master);
		fclose(master);
	}
	else if (trash[1].playerId == -1) {
		trash[1].playerId = del_player->playerId;
		trash[1].position = del_position;

		FILE* master = open_file(master_file_name, "r+b");
		mark_player_spot_deleted(del_position, master);
		fclose(master);
	}
	else {
		Player moved_player;
		FILE* master = open_file(master_file_name, "r+b");


		long moved_pl_position1 = swap_upper_player(trash[0].position, master, trash, index_table, 0);
		long moved_pl_position2 = swap_upper_player(trash[1].position, master, trash, index_table, 1);
		if (moved_pl_position1 != del_position && moved_pl_position2 != del_position) {
			mark_player_spot_deleted(del_position, master);
			trash[0].position = del_position;
		}
		else trash[0].position = -1;
		fclose(master);
	}


	// update index_table 
	upd_table_aft_del(index_table, del_player->playerId);
}



void printoptions() {
	printf("Options:\n");
	printf("  (1) get-m; (2) get-s;\n");
	printf("  (3) del-m; (4) del-s;\n");
	printf("  (5) update-m; (6) update-s;\n");
	printf("  (7) insert-m; (8) insert-s;\n");
	printf("  (9) calc-m; (10) calc-s;\n");
	printf("  (11) ut-m; (12) ut-s;\n");
	printf("  (13) exit.\n");
}

/*
[m/s]index_file:
			2x trashIndex: Index
			18x [m/s]Index: Index
*/
int main() {


	//Player Bob = { 0, false, "Bob", "Cloud Cuckooland", "IM", 120, 5, 0, -1 };
	//Player Alice = { 1, false, "Alice", "Cloud Cuckooland", "GM", 5000, 89, 0, -1 };
	//Player AnonymousPamphleteer = { 2, false, "Anonymous Pamphleteer", "Questionable", "FM", 7000, 4758329, 0, -1 };
	//Player Jakjxjk = { 3, false, "Kjabxkj", "Questionable", "Untitled", 98, 328588, 0, -1 };
	//Player  Ehhhh = { 4, false, "Ehhhh", "????", "FM", 3, 2, 0, -1 };
	//FILE* master_file = open_file("m.bin", "wb");
	//fwrite(&Bob, sizeof(Player), 1, master_file);
	//fwrite(&Alice, sizeof(Player), 1, master_file);
	//fwrite(&AnonymousPamphleteer, sizeof(Player), 1, master_file);
	//fwrite(&Jakjxjk, sizeof(Player), 1, master_file);
	//fwrite(&Ehhhh, sizeof(Player), 1, master_file);
	//fclose(master_file);
	//Index trash = { -1, -1 }, ind;
	//FILE* index_file = open_file("mi.bin", "wb");
	//fwrite(&trash, sizeof(Index), 1, index_file);
	//fwrite(&trash, sizeof(Index), 1, index_file);
	//for (int i = 0; i < 5; i++) {
	//	ind.playerId = i; ind.position = i * sizeof(Player);
	//	fwrite(&ind, sizeof(Index), 1, index_file);
	//}
	//for (int i = 5; i < 20; i++) {
	//	fwrite(&trash, sizeof(Index), 1, index_file);
	//}
	//exit(1);

	char mindex_file[MAX_FIELD_LENGTH] = "mi.bin";
	char m_file[MAX_FIELD_LENGTH] = "m.bin";
	char s_file[MAX_FIELD_LENGTH] = "s.bin";

	Index m_trash[MAX_TRASH_FIELDS], m_index_table[MAX_MASTER_FIELDS];
	int32_t m_icount = read_mindex_table(m_trash, m_index_table, mindex_file);
	int32_t s_maxid = read_and_get_s_max_id(s_file, false);
	int32_t m_maxid = m_index_table[19].playerId;

	printoptions();
	while (true) {

		printf("Pease enter the number of the command you wish to execute: ");
		int32_t input;
		Player player;
		long position;
		TournamentParticipation participation;
		if (read_int("%d", &input) != 1) continue;

		switch (input) {
		case 1:														// get-m
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			print_player_header();
			print_player(&player);
			get_player_participations(&player, s_file);
			
			break;

		case 2:														// get-s
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			input = get_tourid();
			if (input == -1) break;

			if (player.participationCount == 0) {
				printf("	> This player has no recorded tournament appearances. <");
				break;
			}
			get_s(&participation, &player, s_file, input);
			if (participation.participationId != -1) {
				print_participation_header();
				print_participation(&participation);
			}
			else printf("	> This player did not participate in this tournament. <");

			break;
		case 3:														// del-m
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			del_m(&player, position, m_trash, m_file, s_file, m_index_table);

			m_icount--;
			break;

		case 4:														// del-s
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			input = get_tourid();
			if (input == -1) break;

			long participation_location = get_s(&participation , &player, s_file, input);
			if (participation_location == -1) {
				printf("	> Invalid participation. <\n");
				break;
			}

			FILE* slave = open_file(s_file, "r+b");
			del_s(slave, &player, &participation, m_index_table, participation_location, position);
			fclose(slave);
			break;

		case 5:														// update-m
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) return;
			int choice = get_player_editing_choice();

			update_m(position, choice, &player);

			break;

		case 6:														// update-s
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			input = get_tourid();
			if (input == -1) break;

			long part_position = get_s(&participation , &player, s_file, input);
			if (part_position != -1 && !participation.isDeleted) {
				printf("	> Please enter new score: ");
				if (read_int("%d", &input) != 1) return;
				participation.score = input;
				update_s(&participation, part_position, &player, s_file);
			}
			else printf("	> Invalid participation. <\n");

			break;
		case 7:														// insert-m
			if (m_index_table[0].playerId != -1) {
				printf("	> Table is already full. <\n");
				break;
			}
			int res = insert_m(m_file, m_index_table, m_maxid, m_trash);
			if (res == 1) {
				printf("	> Player inserted successfully. <\n");
				m_icount++;
				m_maxid++;
			}
			else printf("	> Error: player wasn't inserted. <\n");
			break;

		case 8:														// insert-s
			position = get_player(m_index_table, m_file, &player);
			if (position == -1) break;

			input = get_tourid();
			if (input == -1) break;

			long partposition = insert_s(s_file, s_maxid, &player, input);
			if (partposition == -1) break;

			player.firstParticipation = partposition;
			player.participationCount++;
			player.totalGames++;
			update_m(position, 6, &player);
			s_maxid++;

			break;

		case 9:														// calc-m
			printf("	> The table presently contains %d players.\n", m_icount);
			break;

		case 10:													// calc-s
			printf("	> The table presently contains %d participations.\n", calc_s(s_file));
			break;

		case 11:													// ut-m
			ut_m(m_file);
			break;

		case 12:													// ut-s
			read_and_get_s_max_id(s_file, true);
			break;

		case 13:													// exit
			printf("> Quitting.\n");
			save_mindex_table(m_trash, m_index_table);
			exit(0);

		default:
			printf("> Invalid input. <\n");
		}

		printf("\n");
	}
}