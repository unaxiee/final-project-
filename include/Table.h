#ifndef TABLE_H
#define TABLE_H
#include <stdlib.h>
#include <stdio.h>
#include "User.h"

#define INIT_TABLE_SIZE 10000
#define EXT_LEN 500

typedef struct Table {
    size_t capacity;
    size_t len;
    User_t *user;
    unsigned char *cache_map;
    FILE *fp;
    char *file_name;
} Table_t;   //store the table from user

typedef struct Likes {
    size_t capacity;
    size_t len;
    Like_t *like;
    unsigned char *cache_map;
} Likes_t;   //store the table from like

Table_t *new_Table(char *file_name);
Likes_t *new_Likes();
int add_User(Table_t *table, User_t *user);
int add_Like(Likes_t *likes, Like_t *like);
int archive_table(Table_t *table);
int load_table(Table_t *table, char *file_name);
User_t* get_User(Table_t *table, size_t idx);
Like_t* get_Like(Likes_t *likes, size_t idx);

#endif
