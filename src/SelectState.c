#include <string.h>
#include <stdlib.h>
#include "Command.h"
#include "SelectState.h"

void field_state_handler(Command_t *cmd, size_t arg_idx) {
    cmd->cmd_args.sel_args.fields = NULL;
    cmd->cmd_args.sel_args.fields_len = 0;
    cmd->cmd_args.sel_args.limit = -1;
    cmd->cmd_args.sel_args.offset = -1;

    while(arg_idx < cmd->args_len) {
        if (!strncmp(cmd->args[arg_idx], "*", 1)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "id", 2)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "name", 4)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "email", 5)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "age", 3)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        } else if (!strncmp(cmd->args[arg_idx], "avg(id)", 7)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "avg(age)", 8)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "sum(id)", 7)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "sum(age)", 8)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "count(*)", 8)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "count(id)", 9)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "count(name)",11)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "count(email)", 12)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "count(age)", 10)) {
            add_select_field(cmd, cmd->args[arg_idx]);
        }else if (!strncmp(cmd->args[arg_idx], "from", 4)) {
            table_state_handler(cmd, arg_idx+1);
            // printf("detect from offset=%d",cmd->cmd_args.sel_args.offset);
            // getchar();
            return;
        } else {
            cmd->type = UNRECOG_CMD;
            return;
        }
        arg_idx += 1;
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void table_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len&& (!strncmp(cmd->args[arg_idx], "user", 4)||!strncmp(cmd->args[arg_idx], "like",4))) {

        arg_idx++;
        if (arg_idx == cmd->args_len) {
            return;
        } else {
        for(int i=arg_idx;i<cmd->args_len-1;i++){
            if (!strncmp(cmd->args[i], "offset", 6)) {
            offset_state_handler(cmd, i+1);
            // printf("jiancedaooffset");
            // getchar();
            return;
        }          
            if (!strncmp(cmd->args[i], "limit", 5)) {
            limit_state_handler(cmd, i+1);
            // printf("jiancedaolimit");
            // getchar();
            return;
        }
    }
        }
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void offset_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len) {
        cmd->cmd_args.sel_args.offset = atoi(cmd->args[arg_idx]);

        arg_idx++;

        if (arg_idx == cmd->args_len) {
            return;
        } else if (arg_idx < cmd->args_len
                && !strncmp(cmd->args[arg_idx], "limit", 5)) {

            limit_state_handler(cmd, arg_idx+1);
            return;
        }
    }
    cmd->type = UNRECOG_CMD;
    return;
}

void limit_state_handler(Command_t *cmd, size_t arg_idx) {
    if (arg_idx < cmd->args_len) {
        cmd->cmd_args.sel_args.limit = atoi(cmd->args[arg_idx]);

        arg_idx++;

        if (arg_idx == cmd->args_len) {
            return;
        }
    }
    cmd->type = UNRECOG_CMD;
    return;
}
