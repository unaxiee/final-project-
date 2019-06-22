#ifndef DB_UTIL_H
#define DB_UTIL_H
#include "Command.h"
#include "Table.h"

typedef struct State {
    int saved_stdout;
} State_t;

typedef struct Obey {
    int len;
    size_t idxList[100000];
}Obey_t;

typedef struct node {
    int key;
    int num;
    struct node_t *left;
    struct node_t *right;
} node_t;

State_t* new_State();
void print_prompt(State_t *state);
int print_user(User_t *user, SelectArgs_t *sel_args,double avgid,double avgage,size_t sumid,size_t sumage,size_t count);
void print_users(Table_t *table, Obey_t *obey, Command_t *cmd);
int parse_input(char *input, Command_t *cmd);
void handle_builtin_cmd(Table_t *table, Command_t *cmd, State_t *state);
int handle_query_cmd(Table_t *table, Likes_t *likes,Command_t *cmd);
int handle_insert_cmd(Table_t *table, Command_t *cmd);
int handle_select_cmd(Table_t *table, Command_t *cmd,Likes_t *Likes);
int handle_select_like_cmd(Likes_t *likes, Command_t *cmd);
void print_help_msg();
Obey_t* handle_where(Command_t *cmd,Table_t *table);
size_t judge(User_t *users,int id1,int id2,char* comps);
void handle_delete_cmd(Table_t *table, Command_t *cmd);
void handle_update_cmd(Table_t *table, Command_t *cmd);
size_t print_like(Likes_t *likes);
int add_Like(Likes_t *likes, Like_t *like);
int handle_join_cmd(Table_t *table,Likes_t *likes,Command_t *cmd,Obey_t *Obey);
node_t* insert_node(node_t *node,int k);
int search_node(node_t* node,int k);
node_t* create_tree(int k);
#endif
