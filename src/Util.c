#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include "Util.h"
#include "Command.h"
#include "Table.h"
#include "SelectState.h"

///
/// Allocate State_t and initialize some attributes
/// Return: ptr of new State_t
///
State_t* new_State() {
    State_t *state = (State_t*)malloc(sizeof(State_t));
    state->saved_stdout = -1;
    return state;
}

///
/// Print shell prompt
///
void print_prompt(State_t *state) {
    if (state->saved_stdout == -1) {
        printf("db > ");
    }
}

///
/// Print the user in the specific format
///
int print_user(User_t *user, SelectArgs_t *sel_args,double avgid,double avgage,size_t sumid,size_t sumage,size_t count) {
    size_t idx;
    printf("(");
    for (idx = 0; idx < sel_args->fields_len; idx++) {
        if (!strncmp(sel_args->fields[idx], "*", 1)) {
            printf("%d, %s, %s, %d", user->id, user->name, user->email, user->age);
        } else {
            if (idx > 0) printf(", ");
            if (!strncmp(sel_args->fields[idx], "id", 2)) {
                printf("%d", user->id);
            } else if (!strncmp(sel_args->fields[idx], "name", 4)) {
                printf("%s", user->name);
            } else if (!strncmp(sel_args->fields[idx], "email", 5)) {
                printf("%s", user->email);
            } else if (!strncmp(sel_args->fields[idx], "age", 3)) {
                printf("%d", user->age);
            }else if (!strncmp(sel_args->fields[idx], "sum(age)", 8)) {
                printf("%ld)\n",sumage);
				return 1;
		    }else if (!strncmp(sel_args->fields[idx], "sum(id)", 8)) {
                printf("%ld)\n",sumid);
				return 1;
		    }else if (!strncmp(sel_args->fields[idx], "avg(id)", 7)) {
                printf("%.3f)\n",avgid);
				return 1;
	  	    } else if (!strncmp(sel_args->fields[idx], "avg(age)", 8)) {
                printf("%.3f)\n", avgage);				
				return 1;
	    	} else if (!strncmp(sel_args->fields[idx], "count", 5)) {
                printf("%ld)\n", count);				
				return 1;
	   		} 
        }
    }
    printf(")\n");
	return 0;
}


// size_t print_like(Likes_t *likes) {
//     size_t idx;
//    for (idx = 0; idx < likes->len; idx++) {
//           printf("(%ld, %d)",get_Like(likes, idx)->id1,get_Like(likes, idx)->id2);
//             }
// 	return 1;
// }
///
/// Print the users for given offset and limit restriction
///
void print_users(Table_t *table, Obey_t *obey, Command_t *cmd) {
    size_t idx,i;
	size_t sumid,sumage;
    double avgid,avgage;
	size_t count;
	int limit = cmd->cmd_args.sel_args.limit;
    int offset = cmd->cmd_args.sel_args.offset;
	sumid=0;
    sumage=0;
    avgid=0;
    avgage=0;
    count=0;
	int idxListLen=obey->len;
	// for (i = 0; i < idxListLen; i++) {
	// 		printf("idx%d:%d\n",i,obey->idxList[i]);
	// }
	for(i=0;i<idxListLen;i++){
		sumid=sumid+(table->user+obey->idxList[i])->id;
		sumage=sumage+(table->user+obey->idxList[i])->age;
		count++;
	}
		avgid=(float)sumid/count;
		avgage=(float)sumage/count;
    if (offset == -1) {
        offset = 0;
    }
    if (obey->idxList) {
        for (idx = offset; idx < idxListLen; idx++) {
            if (limit != -1 && (idx - offset) >= limit) {
                break;
            }
            if(print_user(get_User(table, obey->idxList[idx]), &(cmd->cmd_args.sel_args),avgid,avgage,sumid,sumage,count)){
				break;
			}
        }
    } else {
        for (idx = offset; idx < table->len; idx++) {
            if (limit != -1 && (idx - offset) >= limit) {
                break;
            }
            if(print_user(get_User(table, idx), &(cmd->cmd_args.sel_args),avgid,avgage,sumid,sumage,count)){
				break;
			}
        }
    }
}
///
/// This function received an output argument
/// Return: category of the command
///
int parse_input(char *input, Command_t *cmd) {
    char *token;
    int idx;
    token = strtok(input, " ,\n");
    for (idx = 0; strlen(cmd_list[idx].name) != 0; idx++) {
        if (!strncmp(token, cmd_list[idx].name, cmd_list[idx].len)) {
            cmd->type = cmd_list[idx].type;
        }
    }
    while (token != NULL) {
        add_Arg(cmd, token);
        token = strtok(NULL, " ,\n");
    }
    return cmd->type;
}

///
/// Handle built-in commands
/// Return: command type
///
void handle_builtin_cmd(Table_t *table, Command_t *cmd, State_t *state) {
    if (!strncmp(cmd->args[0], ".exit", 5)) {
        archive_table(table);
        exit(0);
    } else if (!strncmp(cmd->args[0], ".output", 7)) {
        if (cmd->args_len == 2) {
            if (!strncmp(cmd->args[1], "stdout", 6)) {
                close(1);
                dup2(state->saved_stdout, 1);
                state->saved_stdout = -1;
            } else if (state->saved_stdout == -1) {
                int fd = creat(cmd->args[1], 0644);
                state->saved_stdout = dup(1);
                if (dup2(fd, 1) == -1) {
                    state->saved_stdout = -1;
                }
                __fpurge(stdout); //This is used to clear the stdout buffer
            }
        }
    } else if (!strncmp(cmd->args[0], ".load", 5)) {
        if (cmd->args_len == 2) {
            load_table(table, cmd->args[1]);
        }
    } else if (!strncmp(cmd->args[0], ".help", 5)) {
        print_help_msg();
    }
}

///
/// Handle query type commands
/// Return: command type
///
int handle_query_cmd(Table_t *table,Likes_t *likes, Command_t *cmd) {
    if (!strncmp(cmd->args[0], "insert", 6)&&!strncmp(cmd->args[2], "user", 4)) {
        handle_insert_user_cmd(table, cmd);
        return INSERT_CMD;
    }else if (!strncmp(cmd->args[0], "insert", 6)&&!strncmp(cmd->args[2], "like", 4)) {
        handle_insert_like_cmd(likes, cmd);
        return INSERT_CMD;
    }else if (!strncmp(cmd->args[0], "select", 6)&&!strncmp(cmd->args[3], "like", 4)) {
        handle_select_like_cmd(likes, cmd);
        return SELECT_CMD;
    }else if (!strncmp(cmd->args[0], "select", 6)) {
        handle_select_cmd(table, cmd, likes);
        return SELECT_CMD;
    }else if (!strncmp(cmd->args[0], "update", 6)) {
        handle_update_cmd(table, cmd);
        return UPDATE_CMD;
    }else if (!strncmp(cmd->args[0], "delete", 6)) {
        handle_delete_cmd(table, cmd);
    }else {
        return DELETE_CMD;
        return UNRECOG_CMD;
    }
}



void handle_update_cmd(Table_t *table, Command_t *cmd) {
	size_t i,ii;
	size_t id1;  //id1存储第一个比较等号左边，id2存储第一个比较等号种类，id3存储第二个比较左边，id4存储第二个比较等号种类
	size_t num;
	size_t idx;
   	char a1[20];//记录第一个比较里等号左边
	char a2[20];
    char a3[20];//记录第一个比较里等号右边
	Obey_t *obey=handle_where(cmd,table);
	for(i=0;i<cmd->args_len;i++){
		if(!strcmp(cmd->args[i],"set")){
			num=i;
			break;
		}
	}
	//开始将第一个比较的参数分解
	ii=0;
	i=0;
	while(cmd->args[num+1][i]>=97&&cmd->args[num+1][i]<=122){
		a1[ii]=cmd->args[num+1][i];
		ii++;
		i++;
	}  //第一阶段，得到第一个比较的等号左边
	a1[ii]='\0';
	ii=0;
	if(i==strlen(cmd->args[num+1]))
	{
	num++;
	i=0;
	}
	while(cmd->args[num+1][i]=='<'||cmd->args[num+1][i]=='>'||cmd->args[num+1][i]=='!'||cmd->args[num+1][i]=='='){
	a2[ii]=cmd->args[num+1][i];
	ii++;
	i++;
	}
	a2[ii]='\0';//第二阶段，得到第一个比较里的等号种类
	ii=0;
	if(i==strlen(cmd->args[num+1]))
	{
	num++;
	i=0;
	}
	while(i<strlen(cmd->args[num+1])){

			a3[ii]=cmd->args[num+1][i];
			ii++;

	i++;
	}
	a3[ii]='\0';//第三阶段，得到第一个比较右边的值（不包含双引号）
	//分解结束

	if(strcmp(a1,"id")==0){
		id1=1;
	}else if(strcmp(a1,"name")==0){
		id1=2;
	}else if(strcmp(a1,"email")==0){
		id1=3;
	}else if(strcmp(a1,"age")==0){
		id1=4;
	}//判断完id1
	if(id1==1&&obey->len>=2){
		return;
	}
	for (i = 0; i < obey->len; i++) {
			if(id1==1){
				(table->user+obey->idxList[i])->id=atoi(a3);
			}else if(id1==2){
				strcpy((table->user+obey->idxList[i])->name,a3);
			}else if(id1==3){
				strcpy((table->user+obey->idxList[i])->email,a3);
			}else{
				(table->user+obey->idxList[i])->age=atoi(a3);
			}
		}
	
}

void handle_delete_cmd(Table_t *table, Command_t *cmd) {
	size_t idx;
	size_t i,ii;
	Obey_t *obey=handle_where(cmd,table);
	for (ii = 0; ii < obey->len; ii++) {
			for(i=obey->idxList[ii];i<table->len;i++){
				(table->user+i)->id=(table->user+i+1)->id;
				(table->user+i)->age=(table->user+i+1)->age;
				strcpy((table->user+i)->email,(table->user+i+1)->email);
				strcpy((table->user+i)->name,(table->user+i+1)->name);
				}
			table->len--;
		}
	}
///
/// The return value is the number of rows insert into table
/// If the insert operation success, then change the input arg
/// `cmd->type` to INSERT_CMD
///
int handle_insert_user_cmd(Table_t *table, Command_t *cmd) {
    int ret = 0;
    User_t *user = command_to_User(cmd);
    if (user) {
        ret = add_User(table, user);
        if (ret > 0) {
            cmd->type = INSERT_CMD;
        }
    }
	
    return ret;
}

///
/// The return value is the number of rows insert into table
/// If the insert operation success, then change the input arg
/// `cmd->type` to INSERT_CMD
///
int handle_insert_like_cmd(Likes_t *likes, Command_t *cmd) {
    int ret = 0;
    Like_t *like = command_to_Like(cmd);
    if (like) {
        ret = add_Like(likes, like);		
        if (ret > 0) {
            cmd->type = INSERT_CMD;
        }
    }
    return ret;
}
///
/// The return value is the number of rows select from table
/// If the select operation success, then change the input arg
/// `cmd->type` to SELECT_CMD
///
int handle_select_cmd(Table_t *table, Command_t *cmd, Likes_t *likes) {
	size_t i;
	int a=0;
    cmd->type = SELECT_CMD;
	Obey_t *obey=handle_where(cmd,table);
	for(i=0;i<cmd->args_len;i++){
		if(!strncmp(cmd->args[i],"join",4)){
			a=1;
			break;
		}
	}
	if(a==1){
		handle_join_cmd(table,likes,cmd,obey);
	} else {
    field_state_handler(cmd, 1);
    print_users(table,obey, cmd);}
    return 1;
}

int handle_select_like_cmd(Likes_t *likes, Command_t *cmd) {
    cmd->type = SELECT_CMD;
	int i,idx;
	int offset=0;
	int limit=-1;
	size_t idxList[10000];
	int se[7]={0};
	int n=1;
	int state=0;
	int sumid1=0;
	int sumid2=0;
	double avgid1,avgid2;
	for(i=0;i<likes->len;i++){
		idxList[i]=i;
		sumid1=sumid1+get_Like(likes,i)->id1;
		sumid2=sumid2+get_Like(likes,i)->id2;
	}
	avgid1=(double)sumid1/likes->len;
	avgid2=(double)sumid2/likes->len;


	for(i=0;i<cmd->args_len-1;i++){
		if(strncmp(cmd->args[i],"count",5)==0){
			se[2]=1;
		}
		if(strncmp(cmd->args[i],"sum(id1)",8)==0){
			se[3]=1;
		}
		if(strncmp(cmd->args[i],"sum(id2)",8)==0){
			se[4]=1;
		}
		if(strncmp(cmd->args[i],"avg(id1)",8)==0){
			se[5]=1;
		}
		if(strncmp(cmd->args[i],"avg(id2)",8)==0){
			se[6]=1;
		}
		if(strcmp(cmd->args[i],"*")==0){
			se[0]=1;
			se[1]=2;
			state=1;
		}
		if(strcmp(cmd->args[i],"id1")==0&&state!=1){
			se[0]=n;
			n++;
		}	
		if(strcmp(cmd->args[i],"id2")==0&&state!=1){
			se[1]=n;
			n++;
		}
		if(strcmp(cmd->args[i],"offset")==0){
			offset=atoi(cmd->args[i+1]);
		}
		if(strcmp(cmd->args[i],"limit")==0){
			limit=atoi(cmd->args[i+1]);
		}
	}
		for (idx = offset; idx < likes->len; idx++) {
			if (limit != -1 && (idx - offset) >= limit) {
				break;
			}
			if(se[2]){
				if(offset<=0&&(limit>=1||limit==-1)){
					printf("(%d)\n",likes->len);
					return 1;
				}else{
					return 1;
				}
			}
			if(se[3]){
				if(offset<=0&&(limit>=1||limit==-1)){
					printf("(%d)\n",sumid1);
					return 1;
				}else{
					return 1;
				}
			}
			if(se[4]){
				if(offset<=0&&(limit>=1||limit==-1)){
				printf("(%d)\n",sumid1);
					return 1;
				}else{
					return 1;
				}
			}
		    if(se[5]){
				if(offset<=0&&(limit>=1||limit==-1)){
					printf("(%.3f)\n",avgid1);
					return 1;
				}else{
					return 1;
				}
			}
		    if(se[6]){
				if(offset<=0&&(limit>=1||limit==-1)){
					printf("(%.3f)\n",avgid2);
					return 1;
				}else{
					return 1;
				}
			}
			printf("(");

			if(se[0]==0&&se[1]==0){
			}
			if(se[0]==1&&se[1]==0){
				printf("%d",get_Like(likes,idxList[idx])->id1);
			}			
			if(se[0]==0&&se[1]==1){
				printf("%d",get_Like(likes,idxList[idx])->id2);
			}
			if(se[0]==1&&se[1]==2){
				printf("%d,%d",get_Like(likes,idxList[idx])->id1,get_Like(likes,idxList[idx])->id2);
			}
			if(se[0]==2&&se[1]==1){
				printf("%d,%d",get_Like(likes,idxList[idx])->id2,get_Like(likes,idxList[idx])->id1);
			}
			printf(")\n");
		}
    return 1;
}

int handle_join_cmd(Table_t *table,Likes_t *likes,Command_t *cmd,Obey_t *Obey){
    int a;
	size_t i,j;
	int sum = 0;
	User_t *user;
	Like_t* like;
	if(!strncmp(cmd->args[9],"id1",3)){
		a=1;
	} else {
		a=2;
	}
	node_t *node = NULL;
	if (a==1) {
		for (i=0;i<likes->len;i++){
			Like_t *like = get_Like(likes,i);
			node = insert_node(node,like->id1);
			}
		for (j=0;j<Obey->len;j++){
			user = get_User(table,Obey->idxList[j]);
			node_t *find = search_node(node,user->id);
			if(find){
				sum = sum + 1;
			}
		}
	}
	else {
		for (i=0;i<likes->len;i++){
			Like_t *like = get_Like(likes,i);
			node = insert_node(node,like->id2);
			}
		for (j=0;j<Obey->len;j++){
			user = get_User(table,Obey->idxList[j]);
			node_t *find = search_node(node,user->id);
			if(find){
				sum = sum + 1;
			}
		}
	}
	/*if(a==1){
	    for(i=0;i<Obey->len;i++){
		    user = get_User(table,Obey->idxList[i]);
		    for(j=0;j<likes->len;j++){
			    like = get_Like(likes,j);
			    if(user->id==like->id1){
				    sum = sum+1;
				    break;
			    }
			}
		}
	}
	else{
		for(i=0;i<Obey->len;i++){
			user = get_User(table,Obey->idxList[i]);
			for(j=0;j<likes->len;j++){
				like = get_Like(likes,j);
				if(user->id==like->id2){
					sum = sum+1;
					break;
				}
			}
		}
	}*/
	printf("(%d)",sum);
	return 0;
}

node_t *create_tree(int k){
	node_t *node = (node_t*)malloc(sizeof(node_t));
	if(!node){
		return NULL;
	}
	node->left = NULL;
	node->right = NULL;
	node->key = k;
	return node;
}

node_t *insert_node(node_t *node,int k){
	if(!node){
		node = create_tree(k);
		node->left = NULL;
		node->right = NULL;
	} else {
		if (k > node->key) {
			node->right = insert_node(node->right,k);
		} else if (k < node->key) {
			node->left = insert_node(node->left,k);
		}
	}
    return node;
}

node_t *search_node(node_t *node,int k){
	while(node){
		if(node->key < k){
			node = node->right;
		} else if (node->key > k){
			node = node->left;
		} else {
			return node;
		}
	}
	return NULL;
}

Obey_t* handle_where(Command_t *cmd,Table_t *table){			//sign指示是从user中做筛选还是从likes中筛选
    size_t i,ii;
	size_t id1,id2,id3,id4;  //id1存储第一个比较等号左边，id2存储第一个比较等号种类，id3存储第二个比较左边，id4存储第二个比较等号种类 
	size_t num;
	size_t a=0;//判断是否存在where 
	size_t s1,s2; //记录judge的返回值 
	size_t state;//记录是or还是and还是仅有一个参数 1：仅有一个参数，2：or，3：and 
   	char a1[20];//记录第一个比较里等号左边
    char a2[20];//记录第一个比较里的等号种类 
   	char a3[20];//记录第一个比较里等号右边 
    char a4[20];//记录第二个比较里等号左边
	char a5[20];//记录第二个比较里的等号种类 
	char a6[20];//记录第二个比较里等号右边
	Obey_t *obey  = (State_t*)malloc(sizeof(Obey_t));
	obey->len=0;

	for(i=0;i<cmd->args_len;i++){
		if(!strcmp(cmd->args[i],"where")){
			num=i;
			a=1;
			break;
		}
	}
	if(a==0)
	{
		for(i=0;i<table->len;i++){
			obey->idxList[i]=i;
			obey->len++;
			}
		return obey;
	}
	//开始将第一个比较的参数分解 
	ii=0;
	i=0;
	while(cmd->args[num+1][i]>=97&&cmd->args[num+1][i]<=122){
		a1[ii]=cmd->args[num+1][i];
		ii++;
		i++;
	}  //第一阶段，得到第一个比较的等号左边 
	a1[ii]='\0';
	ii=0;
	if(i==strlen(cmd->args[num+1]))
	{
		num++;
		i=0;	
	}
	while(cmd->args[num+1][i]=='<'||cmd->args[num+1][i]=='>'||cmd->args[num+1][i]=='!'||cmd->args[num+1][i]=='='){
		a2[ii]=cmd->args[num+1][i];
		ii++;
		i++;
	}
	a2[ii]='\0';//第二阶段，得到第一个比较里的等号种类 
	ii=0;
	if(i==strlen(cmd->args[num+1]))
	{
		num++;
		i=0;	
	}
	while(i<strlen(cmd->args[num+1])){
		
			a3[ii]=cmd->args[num+1][i];
			ii++;
		
		i++;
	}
	a3[ii]='\0';//第三阶段，得到第一个比较右边的值（不包含双引号） 
	//分解结束
	if(num+2<cmd->args_len)
	{
		state=1;
		if(strcmp(cmd->args[num+2],"and")==0||strcmp(cmd->args[num+2],"or")==0){
			if(strcmp(cmd->args[num+2],"and")==0){
				state=3;
			}else{
				state=2;
			}
			
		ii=0;
		i=0;
		while(cmd->args[num+3][i]>=97&&cmd->args[num+3][i]<=122){
			a4[ii]=cmd->args[num+3][i];
			ii++;
			i++;
		}  //第一阶段，得到第二个比较的等号左边 
		a4[ii]='\0';
		ii=0;
		if(i==strlen(cmd->args[num+3]))
		{
			num++;
			i=0;	
		}
		while(cmd->args[num+3][i]=='<'||cmd->args[num+3][i]=='>'||cmd->args[num+3][i]=='!'||cmd->args[num+3][i]=='='){
			a5[ii]=cmd->args[num+3][i];
			ii++;
			i++;
		}
		a5[ii]='\0';//第二阶段，得到第二个比较里的等号种类 
		ii=0;
		if(i==strlen(cmd->args[num+3]))
		{
			num++;
			i=0;	
		}
		while(i<strlen(cmd->args[num+3])){
			
			a6[ii]=cmd->args[num+3][i];
			ii++;
			
		i++;
		}
		a6[ii]='\0';//第三阶段，得到第二个比较右边的值（不包含双引号） 

	}	
	}else{
		state=1;
	}
	if(strcmp(a1,"id")==0){
		id1=1;
	}else if(strcmp(a1,"name")==0){
		id1=2;
	}else if(strcmp(a1,"email")==0){
		id1=3;
	}else if(strcmp(a1,"age")==0){
		id1=4;
	}//判断完id1 
	if(strcmp(a2,"=")==0){
		id2=1;
	}else if(strcmp(a2,"!=")==0){
		id2=2;
	}else if(strcmp(a2,">")==0){
		id2=3;
	}else if(strcmp(a2,"<")==0){
		id2=4;
	}else if(strcmp(a2,">=")==0){
		id2=5;
	}else if(strcmp(a2,"<=")==0){
		id2=6;
	}//判断完id2 

	if(state!=1){
		if(strcmp(a4,"id")==0){
			id3=1;
		}else if(strcmp(a4,"name")==0){
			id3=2;
		}else if(strcmp(a4,"email")==0){
			id3=3;
		}else if(strcmp(a4,"age")==0){
			id3=4;
		}//判断完id3 
		if(strcmp(a5,"=")==0){
			id4=1;
		}else if(strcmp(a5,"!=")==0){
			id4=2;
		}else if(strcmp(a5,">")==0){
			id4=3;
		}else if(strcmp(a5,"<")==0){
			id4=4;
		}else if(strcmp(a5,">=")==0){
			id4=5;
		}else if(strcmp(a5,"<=")==0){
			id4=6;
		}//判断完id4 	
	} 
	for(i=0;i<table->len;i++){
		if(state==1){
		if(judge(get_User(table,i),id1,id2,a3)){
			obey->idxList[obey->len]=i;
			obey->len++;
		}else{										//不符合条件
		}    //state=1的情况 
		}
		if(state!=1){
			s1= judge(get_User(table,i),id1,id2,a3);
			s2= judge(get_User(table,i),id3,id4,a6);
		if(state==2){
			if(s1||s2){
			obey->idxList[obey->len]=i;
			obey->len++;
					}else{}//不符合条件		
		}
		if(state==3){
			if(s1&&s2){
				obey->idxList[obey->len]=i;
				obey->len++;
			}else{
		}	//符合条件
		}
		}  //state不等于1的情况 	
		}
	// 	getchar();
	// 	printf("len=%d\n",obey->len);

	// for(i=0;i<obey->len;i++){
	// 	printf("第%d次",i);
	// 	printf("%d\n",obey->idxList[i]);
	// }

	return obey;
}



size_t judge(User_t *user,int id1,int id2,char* comps){
	size_t left;
	size_t right;
	char s1[50];
	if(id1==1||id1==4){
	if(id1==1){
		left=user->id;
	}else{
		left=user->age;
	}
	right=atoi(comps);
    // printf("left=%d,right=%d",left,right);
	if(id2==1){ //判断是否=
		if(left==right)
		return 1;
		else
		return 0;
	}

	if(id2==2){//判断是否!=
		if(left!=right)
			return 1;
		else
			return 0;

	}

	if(id2==3){//判断是否>
	if(left>right)
		return 1;
	else
		return 0;

	}

	if(id2==4){//判断是否<
	if(left<right)
		return 1;
	else
		return 0;
	}

	if(id2==5){//判断是否>=
	if(left>=right)
		return 1;
	else
		return 0;
	}

	if(id2==6){//判断是否<=
	if(left<=right)
		return 1;
	else
		return 0;
	}
	}
	else{
		if(id1==2){
			strcpy(s1,user->name);
		}else{
			strcpy(s1,user->email);
		}
		if (id2==1){
			if(!strcmp(s1,comps))
				return 1;
			else
				return 0;
		}
		if(id2==2){
			if(strcmp(s1,comps))
				return 1;
			else
				return 0;
	}
	}
}


/// Show the help messages
///
void print_help_msg() {
    const char msg[] = "# Supported Commands\n"
    "\n"
    "## Built-in Commands\n"
    "\n"
    "  * .exit\n"
    "\tThis cmd archives the table, if the db file is specified, then exit.\n"
    "\n"
    "  * .output\n"
    "\tThis cmd change the output strategy, default is stdout.\n"
    "\n"
    "\tUsage:\n"
    "\t    .output (<file>|stdout)\n\n"
    "\tThe results will be redirected to <file> if specified, otherwise they will display to stdout.\n"
    "\n"
    "  * .load\n"
    "\tThis command loads records stored in <DB file>.\n"
    "\n"
    "\t*** Warning: This command will overwrite the records already stored in current table. ***\n"
    "\n"
    "\tUsage:\n"
    "\t    .load <DB file>\n\n"
    "\n"
    "  * .help\n"
    "\tThis cmd displays the help messages.\n"
    "\n"
    "## Query Commands\n"
    "\n"
    "  * insert\n"
    "\tThis cmd inserts one user record into table.\n"
    "\n"
    "\tUsage:\n"
    "\t    insert <id> <name> <email> <age>\n"
    "\n"
    "\t** Notice: The <name> & <email> are string without any whitespace character, and maximum length of them is 255. **\n"
    "\n"
    "  * select\n"
    "\tThis cmd will display all user records in the table.\n"
    "\n";
    printf("%s", msg);
}

