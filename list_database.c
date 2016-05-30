#include <stdio.h>
#include <stdlib.h> /* malloc を使うため */
#include <string.h> /* 文字列に関係する関数 strcpy, strcmp などを使うため */

#define LINE_MAX 1000 /* 入力の一行の長さの最大値 */
#define TK_MAX 100000 /* token の数の最大値 */
#define TRUE  1
#define FALSE 0

enum type {ATOM, NIL, CONS};

struct token_stream {
	char **stream;
};

struct object {
	enum type t;
	char *name;
	struct object *car;
	struct object *cdr;
};

struct object nil = { NIL, NULL, NULL, NULL };

/**
 * オブジェクトのポインタを受けとってその表現を表示する関数
 */
void print_object(struct object *o){
  if(o == NULL){
		return;
  }
  if(o->t == NIL){
    printf("NIL");
    return;
  }
  if(o->t == ATOM){
		printf("\x1b[45m");
		printf("%s",o->name);
		printf("\x1b[49m"); 
		return;
  }
  if(o->t == CONS){
  	printf("\x1b[42m");
		printf("(");
		printf("\x1b[49m");
		print_object(o->car);
			while(o->t!=NIL){
  			o = o->cdr;
  			if(o->t != NIL){
  				printf(" ");
  			}
  			print_object(o->car);
			}
  	printf("\x1b[43m");
		printf(")");
  	printf("\x1b[49m");
		return ;
  }
}

/**
 * 2つのオブジェクトの正誤を判定する関数
 * あらかじめTRUEは1でFALSEは0だとマクロで定義した
 */
int equal_objects(struct object *o1, struct object *o2){
	if(o1->t == NIL && o2->t == NIL){
		return TRUE;
	}else if(	o1->t == ATOM
						&& o2->t == ATOM
						&& strcmp(o1->name, o2->name) == 0){
		return TRUE;
	}else if( o1->t == CONS
						&& o2->t == CONS
						&& equal_objects(o1->car, o2->car)
						&& equal_objects(o1->cdr, o2->cdr)){
		return TRUE;
	}else{
		return FALSE;
	}
}

/**
 * リストであるオブジェクトへのポインタ o と文字列 fn を受け取り
 * fn に対応するアトムがリストの何番目にあるか返す関数。
 * 無かった場合は -1 を返すことにする。
 */
int index_of_field(struct object *o, char *fn){
  int tmp = 1;
	int ans;
  while(1){
		if(strcmp(o->car->name,fn)==0){
			ans = tmp;
			break;
		}else{
			tmp++;
			o = o->cdr;
		}
		if(o->t == NIL){
			ans = -1;
			break;
		}
	}
	return ans;
}

/** 
 * リストのk番目のcar部を返す関数 
 */
struct object *kth_element(struct object* list_obj, int k){
	while(k != 1){
		list_obj = list_obj -> cdr;
		k--;
	}
	return list_obj -> car;
}

/**
 * fieldsとlist_objを受け取り成形されたデータを表示する関数　
 */
void get_information(struct object *fields, struct object *list_obj){
	int number_of_field = 1;
	while(list_obj->t != NIL){
		print_object(kth_element(fields, number_of_field));
		printf(": ");
		print_object(list_obj -> car);
		printf("\n");
		list_obj = list_obj -> cdr;				//リストを右に1つずつ進めている
		number_of_field++;
	}
	return;
}

/** 
 * fieldはNumberなどの属性 dbはデータベース qはクエリ、つまり探索したいデータ 
 */
void query(struct object *fields_p, struct object *db_p, struct object *q_p){
	int index_tmp = index_of_field(fields_p, q_p->car->name);
	int counter = 0;
	struct object *db_single_list;
	struct object *q_search = q_p -> cdr -> car;

	if(index_tmp == -1){
		printf("Field %s not Found\n\n", q_p->car->name);
	}else{
		while(db_p->t != NIL){
			db_single_list = db_p -> car;
			if(equal_objects(kth_element(db_single_list, index_tmp), q_search)){
				get_information(fields_p, db_single_list);
				printf("\n");
				db_p =  db_p -> cdr;
				counter++;
			}else{
				db_p = db_p -> cdr;
			}
			if(db_p->t == NIL && counter == 0){
				printf("Not Found %s on %s\n\n",
				 q_p->cdr->car->name,kth_element(fields_p,index_tmp)->name);
			}
		}
	}
	return;
}


/** 
 * 文字列を受けとって対応するアトムを生成し、そのポインタを返す関数 
 */
struct object *new_atom(char *n){
  struct object *a = malloc(sizeof(struct object));
  a->t = ATOM;
  a->name = n;
  return a;
}

/** 
 * 2つのオブジェクトを受けとって対応するコンスセルを生成し、そのポインタを返す関数 
 */
struct object *new_list(struct object *car, struct object *cdr){
  struct object *a = malloc(sizeof(struct object));
  a->t = CONS;
  a->car = car;
  a->cdr = cdr;
  return a;
}

/**
 * リストをパースする関数 
 */
struct object *parse_list(struct token_stream *ts){
  if(ts->stream == NULL) return NULL;
  if(*(ts->stream) == NULL) return NULL;
  else if(strcmp(*(ts->stream),"(") == 0){
    struct object *car, *cdr;
    ts->stream++;
    if(*(ts->stream) == NULL){
      return NULL;
    }
    if(strcmp(*(ts->stream),")") == 0){
      ts->stream++;
      car = &nil;
      cdr = parse_list(ts);
      return new_list(car, cdr);
    }
    car = parse_list(ts);
    cdr = parse_list(ts);
    return new_list(car, cdr);
  }
  else if(strcmp(*(ts->stream),")") == 0){
    ts->stream++;
    return &nil;
  }
  else {
    struct object *car, *cdr;
    car = new_atom(*(ts->stream));
    ts->stream++;
    cdr = parse_list(ts);
    return new_list(car, cdr);
  }
}

/**
 * token の配列をパースしてオブジェクトのポインタを返す関数
 */
struct object *parse(struct token_stream *ts){
  if(ts->stream == NULL) return NULL;
  if(*(ts->stream) == NULL) return NULL;
  if(strcmp(*(ts->stream),"(") == 0){
    ts->stream++;
    if(*(ts->stream) == NULL){
      return NULL;
    }
    if(strcmp(*(ts->stream),")") == 0){
      ts->stream++;
      return &nil;
    }

    return parse_list(ts);
  }
  else {
    return new_atom(*(ts->stream++));
  }
}

/** 
 * 字句解析をする関数
 */
char **lex(FILE *in){
  char str[LINE_MAX];
  char **s = malloc(TK_MAX*sizeof(char *));
  char **t = s;

  while(fgets(str, LINE_MAX, in) != NULL && t-s < TK_MAX){
    int i, j;
    i = 0;
    while(str[i] != '\0' && t-s <TK_MAX){
      switch(str[i]){
        case '\n':
        case ' ':
          i++;
          break;
        case '(':
          *t = "(";
          t++;
          i++;
          break;
        case ')':
          *t = ")";
          t++;
          i++;
          break;
        default :
          j=i+1;
          while(str[j] != ')'
					&& str[j] != '('
					&& str[j] != ' '
					&& str[j] != '\n'
					&& str[j] != '\0')
					j++;
          *t = malloc((j-i+1)*sizeof(char));
          strncpy(*t, str+i, j-i+1);
          (*t)[j-i] = '\0';
          t++;
          i=j;
      }
    }
  }
  *t = NULL;
  return s;
}

/**
 * 字句解析結果を表示する関数(使わなくてよい)
 */
void test_print(char **s){
  int i = 0;
  while(*s != NULL){
    printf("%d: \"%s\"\n", i++, *s);
    s++;
  }
  return;
}


int main(){
	struct object *fields, *db, *q;
  struct token_stream TS;
  TS.stream = lex(stdin);
  fields = parse(&TS);
  db = parse(&TS);
  q = parse(&TS);
  while(q != NULL){
		query(fields, db, q);
    q = parse(&TS);
	}
 	printf("\x1b[0m");  
	return 0;
}
