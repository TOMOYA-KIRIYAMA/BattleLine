#include <stdio.h>
#include <stdlib.h> // rand
#include <time.h>   // time
#include <assert.h> // assert

#define SUIT_NUM 2    // 上限：6
#define NUMBER_NUM 10 // 上限：10
#define MAX_CARD (SUIT_NUM * NUMBER_NUM)
#define MAX_LINE 9   // lineの数
#define MAX_MEMBER 3 // 1ラインの上限人数
#define MAX_HAND 7   // 手札上限
#define PLAYER1 0    // 自分
#define PLAYER2 1    // 相手
#define PLAYER_NUM 2
#define STRAIGHT_FIN_NUM 3
#define MULTI_FIN_NUM 5
#define MAX_COMMAND_LINE 10

/*--< 手役 >--*/
typedef enum {
	Buta,
	Straight,
	Flash,
	ThreeCard,
	StraightFlush
} LineScore;

typedef struct {
	int suit; // 0～5
	int num;  // 0～9
	int id;   // for shuffle
} card_t;

card_t card[MAX_CARD];
card_t *card_ptr[MAX_CARD];
card_t blank = { 6, 10, 0}; // 陣形の初期化の際に置く空白を表すカード
card_t *lines[PLAYER_NUM][MAX_LINE][MAX_MEMBER]; // 自陣と敵陣の状態
int lines_count[PLAYER_NUM][MAX_LINE]; // 各列に何個のカードが置かれているかを記録
card_t *hands[PLAYER_NUM][MAX_HAND];
int next_card = 0;       // 次のカードがdeckから何番目かを示す
int lines_v_d[MAX_LINE]; //各列での勝敗を記録 0:未対戦 1:PLAYER1 2:PLAYER2


/* カード構造体中のsuit変数に対応する文字を出力 */
char suit_to_mark(int suit){
	char mark[7] = "!#$%&?-"; // suit の数字に対応した文字
	return mark[suit];
}


/* カード構造体中のnum変数に対応する文字を出力 */
char num_to_char(int num){
	char c_num[11] = "0123456789-";	// num の数字に対応した文字
	return c_num[num];
}


/* 各戦列の勝敗を表示 */
char print_v_d(int k){
	char ox[4]=" 12";
	return ox[k];
}


/* 場の状態を表示 */
void print_field(void){
	int i, j;
	
	/* 敵陣の表示 */
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
	
	/* 自陣の表示 */
	for(j=0; j<MAX_MEMBER; j++){
		for(i=0; i<MAX_LINE; i++){
			printf("%c%c ", suit_to_mark( lines[PLAYER1][i][j]-> suit ), num_to_char( lines[PLAYER1][i][j]-> num ) );
		}
		printf("\n");
	}
	
	/* 各列に番号を表示(視認性UP) */
	for(i=0; i<MAX_LINE; i++){
		printf("%d  ", i);
	}
	printf("\n\n");
}


/* デックの中身をすべて表示 */
void print_deck(void){
	int i;
	
	for (i=0; i<MAX_CARD; i++){
		printf("%d: %d\n", card_ptr[i]-> suit, card_ptr[i]-> num);
	}
}


/* playerの手札を表示 */
void print_hand(int player){
	int i;
	
	for(i=0; i<MAX_HAND; i++){
		printf("%c%c ", suit_to_mark( hands[player][i]-> suit ), num_to_char( hands[player][i]-> num ) );
	}
	printf("\n");
}


/* cardの入れ替え */
void swap_card(card_t **c1, card_t **c2){
	card_t *tmp;
	
	tmp = *c1;
	*c1  = *c2;
	*c2  = tmp;
}


/* カードの大小比較 */
/* 1:引数1  0:引数2 */
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


/* カードをソートする */
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


/* 重複したカードがないかの検査 */
/*     0:なし      1:あり       */
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


/* 手役判定                       */
/* ストレートフラッシュ->3カード  */
/* ->フラッシュ->ストレート->ブタ */
LineScore line_score(card_t *line_ptr[MAX_MEMBER]){
	int i;
	int first_suit;
	int first_num;
	
	first_suit = line_ptr[0]-> suit;
	first_num  = line_ptr[0]-> num;
	
	/* ストレートフラッシュ */
	if (line_ptr[1]-> suit == first_suit && line_ptr[1]-> num == (first_num + 1) ){
		if (line_ptr[2]-> suit == first_suit && line_ptr[2]-> num == (first_num + 2) ){
			return StraightFlush;
		}
	}
	
	/* 3カード */
	if (line_ptr[1]-> num == first_num ){
		if (line_ptr[2]-> num == first_num ){
			return ThreeCard;
		}
	}
	
	/* フラッシュ */
	if (line_ptr[1]-> suit == first_suit ){
		if (line_ptr[2]-> suit == first_suit ){
			return Flash;
		}
	}
	
	/* ストレート */
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
	
	/* 役なし */
	return Buta;
}


/* どちらの戦列の方が強いかの判定 */
/* 引数1が引数2より強いかどうか   */
/*   1:引数1が大    0:引数2が大   */
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
			return player; // 最後に出したプレイヤーが負ける
		}
		else{
			return 0;
		}
	}
	else {
		return 0;
	}
}


/* 配列の left から right の要素のマージソートを行う */
void m_sort(card_t *deck[], int left, int right)
{
	int mid, i;
	int left_end, right_head;
	int tmp_pos = 0;
	card_t *temp[MAX_CARD];
	
	if (left >= right)           /* 配列の要素がひとつなら */
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
	/* 片方の要素がなくなれば，残りは後ろにつける */
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
	
	/* temp から本来の格納場所へコピー */
	for (i = tmp_pos-1; 0<=i; i--){
		deck[right] = temp[i];
		right--;
	}
}


/* カードの順序を入れ替える */
void MergeSort(card_t *deck[], int size){
	m_sort(deck, 0, size-1);
}


/* 使用するカードに数字を割り当てる */
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

/* 各カードに乱数を振り，乱数に従ってソートする */
void shuffle(card_t *deck[]){
	int i;
	
	srand( (unsigned)time(NULL) );
	for(i=0; i < MAX_CARD; i++){
		deck[i]-> id = rand();
	}
	MergeSort(deck, MAX_CARD);
}

/* 各カードの構造体へのポインター配列を作成 */
void make_card_shadow(card_t deck[], card_t *shadow[], int size){
	int i;
	
	for( i=0; i < size; i++){
		shadow[i] = &deck[i];
	}
}

/* 初期化（カードの設定とシャッフル） */
void init_deck(card_t deck[]){
	make_card_shadow(deck, card_ptr, MAX_CARD);
	init_card(card_ptr);
	shuffle(card_ptr);
	if ( is_repeat_card(card_ptr) ){
		printf("カードが重複しています\n"); // for debug
	}
}

/* 場の状態の初期化 */
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


/* 各プレイヤーに初期手札を配布 */
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
	
	/* 各自の手札のソート */
	babble_sort(hands[PLAYER1], MAX_HAND);
	babble_sort(hands[PLAYER2], MAX_HAND);
}


/* 文字列 str を sep の文字を区切りに分割する */
/* 文字列の分割は最大 max 個まで                */
/* 分割された各文字列の先頭は ret へ格納      */
/* 返り値は分割数                             */
int split_str(char *str, char *ret[], char sep, int max){
	int col = 0; // 分割数
	
	ret[col++] = str;
	while (*str && col< max){
		if (*str == sep){
			ret[col++] = str + 1;
		}
		++str;
	}
	
	return col;
}


/* プレイヤーの入力を解釈する                */
/* 入力から選択されたカードと列を読み取る    */
/* 異常入力時はsele_cd, sele_ln に -1 が入る */
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


/* sele_cd と sele_ln に入っている値は有効なものか */
/*              有効：1    無効：0                 */
int is_valid_command(int sele_cd, int sele_ln){
	if (sele_cd < 0 || MAX_HAND <= sele_cd)
		return 0; // error
	if (sele_ln < 0 || MAX_LINE <= sele_ln)
		return 0; // error
	
	return 1;
}


/* lineのカード列をソート */
void sort_line(card_t *line_ptr[MAX_MEMBER], int size){
	if (size < 2)	return;
	babble_sort(line_ptr, size);
}


/* 選択されたカードと card が同じ構造体への参照をしているか */
/* つまり、同じカードを参照しているかの判定                 */
/*           同じ：1      異なる：0                         */
int is_the_card(int player, int sele_cd, card_t *card){
	if (hands[player][sele_cd] == card){
		return 1;
	}
	return 0;
}


/* 手札のカードを選択し列に出す */
/* 返り値：選択したカード番号   */
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
		/* 選択した列に空きがあれば挿入 */
		if ( (lines[player][selected_line][i]-> suit == 6) && (lines[player][selected_line][i]-> num == 10) ){
			lines[player][selected_line][i] = hands[player][selected_card];
			++lines_count[player][selected_line];
			sort_line(lines[player][selected_line], lines_count[player][selected_line]);
			return selected_card;
		}
	}
	/* 空白がなかった場合 */
	printf("ERROR:This line is full\n");
	return play_card(player);
}


/* 各ラインにおいて勝負が決まったかどうか */
void v_d_card_line(int player){
	int i;
	int line;
	int w_l; // 勝敗(1:勝ち 0:負け)
	
	for (i=0; i<MAX_LINE; i++){
		if ( lines_count[PLAYER1][i] == MAX_MEMBER && lines_count[PLAYER2][i] == MAX_MEMBER){
			if (lines_v_d[i] == 0){
				w_l = is_line_greater(lines[PLAYER1][i], lines[PLAYER2][i], player);
				if (w_l){ // PLAYER1が勝ったなら
					lines_v_d[i] = 1;
				}
				else {
					lines_v_d[i] = 2;
				}
			}
		}
	}
}


/* 連続した列をとり勝利したかを判定 */
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


/* ゲームの勝敗が決まったかどうかを判定 */
/* 各ラインで勝利規定数の勝負が行われたか */
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


/* 戦隊による勝敗とゲームの勝敗の処理 */
int victory_or_defeat(int player){
	int flag_finish = 0;
	
	v_d_card_line(player);    // 各ラインにおいて勝負が決まったかどうか
	flag_finish = win_lose(); // ゲームの勝敗が決まったか
	
	return flag_finish; // fix me
}


/* カードを1枚引く */
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


/* 返り値に次のプレイヤーの数字を返す */
int turn_chenge(int player){
	return 1-player;
}


/* メインとなるゲームの中身 */
void play_game(void){
	int selected_card = 0;
	int player = PLAYER1;
	int flag_finish = 0; // gemeの勝敗がついたなら 1 になる
	char player_name[PLAYER_NUM][10] = {"PLAYER1", "PLAYER2"};
	
	while(!flag_finish){
		printf("【%s's turn】\n", player_name[player]);
		print_field();
		selected_card = play_card(player);       // カードをプレイする
		flag_finish = victory_or_defeat(player); // 戦いで勝敗の確認と処理
		draw_a_card(player, selected_card);      // 手札の補充
		player = turn_chenge(player);            // 手番の交代
	}
	print_field();
}


/* クライアントとの接続                   */
/* 指定人数のプレイヤーが接続するまで待機 */
void begin_program(void){ // fix me
	
}


/* 接続しているクライアントに終了するメッセージと */
/* ソケットの終了を行う関数                       */
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
	//begin_program(); // 未作成
	init_deck(card);
	init_field();
	
	init_hands();
	play_game();
	//end_program(); // 未作成
	return 0;
}