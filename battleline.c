#include <stdio.h>
#include <stdlib.h> // rand
#include <time.h>   // time
#include <assert.h> // assert

#define SUIT_NUM 2    // ����F6
#define NUMBER_NUM 10 // ����F10
#define MAX_CARD (SUIT_NUM * NUMBER_NUM)
#define MAX_LINE 9   // line�̐�
#define MAX_MEMBER 3 // 1���C���̏���l��
#define MAX_HAND 7   // ��D���
#define PLAYER1 0    // ����
#define PLAYER2 1    // ����
#define PLAYER_NUM 2
#define STRAIGHT_FIN_NUM 3
#define MULTI_FIN_NUM 5
#define MAX_COMMAND_LINE 10

/*--< ��� >--*/
typedef enum {
	Buta,
	Straight,
	Flash,
	ThreeCard,
	StraightFlush
} LineScore;

typedef struct {
	int suit; // 0�`5
	int num;  // 0�`9
	int id;   // for shuffle
} card_t;

card_t card[MAX_CARD];
card_t *card_ptr[MAX_CARD];
card_t blank = { 6, 10, 0}; // �w�`�̏������̍ۂɒu���󔒂�\���J�[�h
card_t *lines[PLAYER_NUM][MAX_LINE][MAX_MEMBER]; // ���w�ƓG�w�̏��
int lines_count[PLAYER_NUM][MAX_LINE]; // �e��ɉ��̃J�[�h���u����Ă��邩���L�^
card_t *hands[PLAYER_NUM][MAX_HAND];
int next_card = 0;       // ���̃J�[�h��deck���牽�Ԗڂ�������
int lines_v_d[MAX_LINE]; //�e��ł̏��s���L�^ 0:���ΐ� 1:PLAYER1 2:PLAYER2


/* �J�[�h�\���̒���suit�ϐ��ɑΉ����镶�����o�� */
char suit_to_mark(int suit){
	char mark[7] = "!#$%&?-"; // suit �̐����ɑΉ���������
	return mark[suit];
}


/* �J�[�h�\���̒���num�ϐ��ɑΉ����镶�����o�� */
char num_to_char(int num){
	char c_num[11] = "0123456789-";	// num �̐����ɑΉ���������
	return c_num[num];
}


/* �e���̏��s��\�� */
char print_v_d(int k){
	char ox[4]=" 12";
	return ox[k];
}


/* ��̏�Ԃ�\�� */
void print_field(void){
	int i, j;
	
	/* �G�w�̕\�� */
	for(j=0; j<MAX_MEMBER; j++){
		for(i=0; i<MAX_LINE; i++){
			printf("%c%c ", suit_to_mark( lines[PLAYER2][i][j]-> suit ), num_to_char( lines[PLAYER2][i][j]-> num) );
		}
		printf("\n");
	}
	
	for (i=0; i<MAX_LINE; i++){
		printf("%c  ", print_v_d( lines_v_d[i] ));
	}
	printf("\n");
	
	/* ���w�̕\�� */
	for(j=0; j<MAX_MEMBER; j++){
		for(i=0; i<MAX_LINE; i++){
			printf("%c%c ", suit_to_mark( lines[PLAYER1][i][j]-> suit ), num_to_char( lines[PLAYER1][i][j]-> num ) );
		}
		printf("\n");
	}
	
	/* �e��ɔԍ���\��(���F��UP) */
	for(i=0; i<MAX_LINE; i++){
		printf("%d  ", i);
	}
	printf("\n\n");
}


/* �f�b�N�̒��g�����ׂĕ\�� */
void print_deck(void){
	int i;
	
	for (i=0; i<MAX_CARD; i++){
		printf("%d: %d\n", card_ptr[i]-> suit, card_ptr[i]-> num);
	}
}


/* player�̎�D��\�� */
void print_hand(int player){
	int i;
	
	for(i=0; i<MAX_HAND; i++){
		printf("%c%c ", suit_to_mark( hands[player][i]-> suit ), num_to_char( hands[player][i]-> num ) );
	}
	printf("\n");
}


/* card�̓���ւ� */
void swap_card(card_t **c1, card_t **c2){
	card_t *tmp;
	
	tmp = *c1;
	*c1  = *c2;
	*c2  = tmp;
}


/* �J�[�h�̑召��r */
/* 1:����1  0:����2 */
int is_card_greater(card_t *c1, card_t * c2){
	if (c1->suit > c2->suit){
		return 1;
	}
	
	else if( c1->suit == c2->suit){
		if( c1->num > c2->num){
			return 1;
		}
		else{
			return 0;
		}
	}
	
	else{
		return 0;
	}
	
}


/* �J�[�h���\�[�g���� */
void babble_sort(card_t *cards_ptr[], int size){
	int i, j;
	
	for (i=0; i<size-1; i++){
		for (j=i+1; j<size; j++){
			if (is_card_greater(cards_ptr[i], cards_ptr[j])){
				swap_card(&cards_ptr[i], &cards_ptr[j]);
			}
		}
	}
}


/* �d�������J�[�h���Ȃ����̌��� */
/*     0:�Ȃ�      1:����       */
int is_repeat_card(card_t *deck[]){
	int i, j;
	
	for (i=0; i<MAX_CARD-1; i++){
		for (j=i+1; j<MAX_CARD; j++){
			if (deck[i]-> suit == deck[j]-> suit){
				if (deck[i]-> num == deck[j]-> num){
					return 1;
				}
			}
		}
	}
	return 0;
}


/* ��𔻒�                       */
/* �X�g���[�g�t���b�V��->3�J�[�h  */
/* ->�t���b�V��->�X�g���[�g->�u�^ */
LineScore line_score(card_t *line_ptr[MAX_MEMBER]){
	int i;
	int first_suit;
	int first_num;
	
	first_suit = line_ptr[0]-> suit;
	first_num  = line_ptr[0]-> num;
	
	/* �X�g���[�g�t���b�V�� */
	if (line_ptr[1]-> suit == first_suit && line_ptr[1]-> num == (first_num + 1) ){
		if (line_ptr[2]-> suit == first_suit && line_ptr[2]-> num == (first_num + 2) ){
			return StraightFlush;
		}
	}
	
	/* 3�J�[�h */
	if (line_ptr[1]-> num == first_num ){
		if (line_ptr[2]-> num == first_num ){
			return ThreeCard;
		}
	}
	
	/* �t���b�V�� */
	if (line_ptr[1]-> suit == first_suit ){
		if (line_ptr[2]-> suit == first_suit ){
			return Flash;
		}
	}
	
	/* �X�g���[�g */
	if (line_ptr[1]-> num == (first_num + 1) ){
		if (line_ptr[2]-> num == (first_num + 2) ){
			return Straight;
		}
	}
	if (line_ptr[1]-> num == (first_num + 2) ){
		if (line_ptr[2]-> num == (first_num + 1) ){
			return Straight;
		}
	}
	if (line_ptr[1]-> num == (first_num - 1) ){
		if (line_ptr[2]-> num == (first_num + 1) ){
			return Straight;
		}
	}
	if (line_ptr[1]-> num == (first_num + 1) ){
		if (line_ptr[2]-> num == (first_num - 1) ){
			return Straight;
		}
	}
	if (line_ptr[1]-> num == (first_num - 2) ){
		if (line_ptr[2]-> num == (first_num - 1) ){
			return Straight;
		}
	}
	if (line_ptr[1]-> num == (first_num - 1) ){
		if (line_ptr[2]-> num == (first_num - 2) ){
			return Straight;
		}
	}
	
	/* ���Ȃ� */
	return Buta;
}


/* �ǂ���̐��̕����������̔��� */
/* ����1������2��苭�����ǂ���   */
/*   1:����1����    0:����2����   */
int is_line_greater(card_t *line_ptr1[MAX_MEMBER], card_t *line_ptr2[MAX_MEMBER], int player){
	LineScore score[PLAYER_NUM];
	int sum[PLAYER_NUM];
	int i;
	
	score[0] = line_score(line_ptr1);
	score[1] = line_score(line_ptr2);
	for (i=0; i<PLAYER_NUM; i++){
		sum[i] = 0;
	}
	
	
	if (score[0] > score[1]){
		return 1;
	}
	else if (score[0] == score[1]){
		for (i=0; i<MAX_MEMBER; i++){
			sum[0] += line_ptr1[i]-> num;
			sum[1] += line_ptr2[i]-> num;
		}
		if (sum[0] > sum[1]){
			return 1;
		}
		else if(sum[0] == sum[1]){
			return player; // �Ō�ɏo�����v���C���[��������
		}
		else{
			return 0;
		}
	}
	else {
		return 0;
	}
}


/* �z��� left ���� right �̗v�f�̃}�[�W�\�[�g���s�� */
void m_sort(card_t *deck[], int left, int right)
{
	int mid, i;
	int left_end, right_head;
	int tmp_pos = 0;
	card_t *temp[MAX_CARD];
	
	if (left >= right)           /* �z��̗v�f���ЂƂȂ� */
		return;
		
	mid = (left + right) / 2;
	m_sort(deck, left, mid);
	m_sort(deck, mid + 1, right);
	
	left_end = mid;
	right_head = mid + 1;
	while ( (left <= left_end) && (right_head <=right) ){
		if ( deck[left]-> id < deck[right_head]-> id){
			temp[tmp_pos] = deck[left];
			left++;
		}
		else {
			temp[tmp_pos] = deck[right_head];
			right_head++;
		}
		tmp_pos++;
	}
	/* �Е��̗v�f���Ȃ��Ȃ�΁C�c��͌��ɂ��� */
	while (left <= left_end){
		temp[tmp_pos] = deck[left];
		left++;	
		tmp_pos++;
	}
	while (right_head <=right){
		temp[tmp_pos] = deck[right_head];
		right_head++;
		tmp_pos++;
	}
	
	/* temp ����{���̊i�[�ꏊ�փR�s�[ */
	for (i = tmp_pos-1; 0<=i; i--){
		deck[right] = temp[i];
		right--;
	}
}


/* �J�[�h�̏��������ւ��� */
void MergeSort(card_t *deck[], int size){
	m_sort(deck, 0, size-1);
}


/* �g�p����J�[�h�ɐ��������蓖�Ă� */
void init_card(card_t *deck[]){
	int i, j, k;
	
	k = 0;
	for (i=0; i<SUIT_NUM; i++){
		for (j=0; j<NUMBER_NUM; j++){
			deck[k]-> suit = i;
			deck[k]-> num  = j;
			k++;
		}
	}
}

/* �e�J�[�h�ɗ�����U��C�����ɏ]���ă\�[�g���� */
void shuffle(card_t *deck[]){
	int i;
	
	srand( (unsigned)time(NULL) );
	for(i=0; i < MAX_CARD; i++){
		deck[i]-> id = rand();
	}
	MergeSort(deck, MAX_CARD);
}

/* �e�J�[�h�̍\���̂ւ̃|�C���^�[�z����쐬 */
void make_card_shadow(card_t deck[], card_t *shadow[], int size){
	int i;
	
	for( i=0; i < size; i++){
		shadow[i] = &deck[i];
	}
}

/* �������i�J�[�h�̐ݒ�ƃV���b�t���j */
void init_deck(card_t deck[]){
	make_card_shadow(deck, card_ptr, MAX_CARD);
	init_card(card_ptr);
	shuffle(card_ptr);
	if ( is_repeat_card(card_ptr) ){
		printf("�J�[�h���d�����Ă��܂�\n"); // for debug
	}
}

/* ��̏�Ԃ̏����� */
void init_field(void){
	int i,j;
	
	for(i=0; i<MAX_LINE; i++){
		for(j=0; j<MAX_MEMBER; j++){
			lines[PLAYER1][i][j] = &blank;
			lines[PLAYER2][i][j] = &blank;
		}
		lines_count[PLAYER1][i] = 0;
		lines_count[PLAYER2][i] = 0;
		lines_v_d[i] = 0;
	}
}


/* �e�v���C���[�ɏ�����D��z�z */
void init_hands(void){
	int i;
	
	for(i=0; i<MAX_HAND; i++){
		hands[PLAYER1][i] = card_ptr[next_card];
		next_card++;
	}
	
	for(i=0; i<MAX_HAND; i++){
		hands[PLAYER2][i] = card_ptr[next_card];
		next_card++;
	}
	
	/* �e���̎�D�̃\�[�g */
	babble_sort(hands[PLAYER1], MAX_HAND);
	babble_sort(hands[PLAYER2], MAX_HAND);
}


/* ������ str �� sep �̕�������؂�ɕ������� */
/* ������̕����͍ő� max �܂�                */
/* �������ꂽ�e������̐擪�� ret �֊i�[      */
/* �Ԃ�l�͕�����                             */
int split_str(char *str, char *ret[], char sep, int max){
	int col = 0; // ������
	
	ret[col++] = str;
	while (*str && col< max){
		if (*str == sep){
			ret[col++] = str + 1;
		}
		++str;
	}
	
	return col;
}


/* �v���C���[�̓��͂����߂���                */
/* ���͂���I�����ꂽ�J�[�h�Ɨ��ǂݎ��    */
/* �ُ���͎���sele_cd, sele_ln �� -1 ������ */
void get_command(int *sele_cd, int *sele_ln){
	char line[MAX_COMMAND_LINE];
	char *ret[2];
	
	if (fgets(line, MAX_COMMAND_LINE, stdin) != NULL){
		if (split_str(line, ret, ':', 2) == 2){
			*sele_cd = atoi(ret[0]);
			*sele_ln = atoi(ret[1]);
		}
		else{
			/* error */
			*sele_cd = -1;
			*sele_ln = -1;
		}
	}
	else{
		/* error */
		*sele_cd = -1;
		*sele_ln = -1;
	}
}


/* sele_cd �� sele_ln �ɓ����Ă���l�͗L���Ȃ��̂� */
/*              �L���F1    �����F0                 */
int is_valid_command(int sele_cd, int sele_ln){
	if (sele_cd < 0 || MAX_HAND <= sele_cd)
		return 0; // error
	if (sele_ln < 0 || MAX_LINE <= sele_ln)
		return 0; // error
	
	return 1;
}


/* line�̃J�[�h����\�[�g */
void sort_line(card_t *line_ptr[MAX_MEMBER], int size){
	if (size < 2)	return;
	babble_sort(line_ptr, size);
}


/* �I�����ꂽ�J�[�h�� card �������\���̂ւ̎Q�Ƃ����Ă��邩 */
/* �܂�A�����J�[�h���Q�Ƃ��Ă��邩�̔���                 */
/*           �����F1      �قȂ�F0                         */
int is_the_card(int player, int sele_cd, card_t *card){
	if (hands[player][sele_cd] == card){
		return 1;
	}
	return 0;
}


/* ��D�̃J�[�h��I������ɏo�� */
/* �Ԃ�l�F�I�������J�[�h�ԍ�   */
int play_card(int player){
	int selected_card = -1;
	int selected_line = -1;
	int i;
	
	print_hand(player);
	do{
		printf("input [select_card : select_line]\n");
		get_command(&selected_card, &selected_line);
	} while ( !(is_valid_command(selected_card, selected_line)) 
				|| is_the_card(player, selected_card, &blank) );
	
	for (i=0; i<MAX_MEMBER; i++){
		/* �I��������ɋ󂫂�����Α}�� */
		if ( (lines[player][selected_line][i]-> suit == 6) && (lines[player][selected_line][i]-> num == 10) ){
			lines[player][selected_line][i] = hands[player][selected_card];
			++lines_count[player][selected_line];
			sort_line(lines[player][selected_line], lines_count[player][selected_line]);
			return selected_card;
		}
	}
	/* �󔒂��Ȃ������ꍇ */
	printf("ERROR:This line is full\n");
	return play_card(player);
}


/* �e���C���ɂ����ď��������܂������ǂ��� */
void v_d_card_line(int player){
	int i;
	int line;
	int w_l; // ���s(1:���� 0:����)
	
	for (i=0; i<MAX_LINE; i++){
		if ( lines_count[PLAYER1][i] == MAX_MEMBER && lines_count[PLAYER2][i] == MAX_MEMBER){
			if (lines_v_d[i] == 0){
				w_l = is_line_greater(lines[PLAYER1][i], lines[PLAYER2][i], player);
				if (w_l){ // PLAYER1���������Ȃ�
					lines_v_d[i] = 1;
				}
				else {
					lines_v_d[i] = 2;
				}
			}
		}
	}
}


/* �A����������Ƃ菟���������𔻒� */
int is_straight_win(int player){
	int i;
	int count = 0;
	
	for (i=0; i<MAX_LINE; i++){
		if (lines_v_d[i] == player+1){
			count++;
			if (count == STRAIGHT_FIN_NUM){
				return 1;
			}
		}
		else{
			count = 0;
		}
	}
	
	return 0;
}


/* �Q�[���̏��s�����܂������ǂ����𔻒� */
/* �e���C���ŏ����K�萔�̏������s��ꂽ�� */
int win_lose(void){
	int i;
	int flag_finish = 0;
	int win_count[PLAYER_NUM] = {0, 0};
	int s_fin[PLAYER_NUM] = {0, 0};
	
	for (i=0; i<MAX_LINE; i++){
		if (lines_v_d[i]==1){
			win_count[PLAYER1]++;
		}
		else if(lines_v_d[i]==2){
			win_count[PLAYER2]++;
		}
	}
	for (i=0; i<PLAYER_NUM; i++){
		s_fin[i] = is_straight_win(i);
	}
	
	if ( (win_count[PLAYER1] == MULTI_FIN_NUM) || s_fin[PLAYER1]){
		printf("\n----PLAYER1 WIN!!-------\n");
		flag_finish = 1;
	}
	else if ( (win_count[PLAYER2] == MULTI_FIN_NUM) || s_fin[PLAYER2]){
		printf("\n----PLAYER2 WIN!!-------\n");
		flag_finish = 1;
	}
	
	return flag_finish;
}


/* ����ɂ�鏟�s�ƃQ�[���̏��s�̏��� */
int victory_or_defeat(int player){
	int flag_finish = 0;
	
	v_d_card_line(player);    // �e���C���ɂ����ď��������܂������ǂ���
	flag_finish = win_lose(); // �Q�[���̏��s�����܂�����
	
	return flag_finish; // fix me
}


/* �J�[�h��1������ */
void draw_a_card(int player, int selected_card){
	if (next_card < MAX_CARD){
		hands[player][selected_card] = card_ptr[next_card];
		next_card++;
	}
	else {
		hands[player][selected_card] = &blank;
	}
	babble_sort( hands[player], MAX_HAND);
}


/* �Ԃ�l�Ɏ��̃v���C���[�̐�����Ԃ� */
int turn_chenge(int player){
	return 1-player;
}


/* ���C���ƂȂ�Q�[���̒��g */
void play_game(void){
	int selected_card = 0;
	int player = PLAYER1;
	int flag_finish = 0; // geme�̏��s�������Ȃ� 1 �ɂȂ�
	char player_name[PLAYER_NUM][10] = {"PLAYER1", "PLAYER2"};
	
	while(!flag_finish){
		printf("�y%s's turn�z\n", player_name[player]);
		print_field();
		selected_card = play_card(player);       // �J�[�h���v���C����
		flag_finish = victory_or_defeat(player); // �킢�ŏ��s�̊m�F�Ə���
		draw_a_card(player, selected_card);      // ��D�̕�[
		player = turn_chenge(player);            // ��Ԃ̌��
	}
	print_field();
}


/* �N���C�A���g�Ƃ̐ڑ�                   */
/* �w��l���̃v���C���[���ڑ�����܂őҋ@ */
void begin_program(void){ // fix me
	
}


/* �ڑ����Ă���N���C�A���g�ɏI�����郁�b�Z�[�W�� */
/* �\�P�b�g�̏I�����s���֐�                       */
void end_program(void){ // fix me
	
}


void test(void){
	assert( is_valid_command(-1,-1) == 0 );
	assert( is_valid_command(0,8) == 1 );
	assert( is_valid_command(6,0) == 1 );
	assert( is_valid_command(6,8) == 1 );
	assert( is_valid_command(0,9) == 0 );
	assert( is_valid_command(7,0) == 0 );
	assert( is_valid_command(7,9) == 0 );
}


int main2(void){
	test();
	printf("DEBUG SUCCESS\n");
}


int main(void)
{
	//begin_program(); // ���쐬
	init_deck(card);
	init_field();
	
	init_hands();
	play_game();
	//end_program(); // ���쐬
	return 0;
}