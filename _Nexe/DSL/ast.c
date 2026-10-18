
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AST AST;

// AST for the BUILD.sbl model.
struct AST {
    enum {
        AST_BUILD,
        AST_TASK,
        AST_RUN
    } tag;

    union {
        struct {
            FILE *file;
        } build;

        struct {
            unsigned long current_hash;
            unsigned long previous_hash;
            int changed;
        } task;

        struct {
            unsigned long previous_hash;
            unsigned long new_hash;
            AST *right;
        } run;
    } data;
};

AST *ast_new(AST ast) {
    AST *ptr = (AST *)malloc(sizeof(AST));
    if (ptr != NULL) {
        *ptr = ast;
    }
    return ptr;
}

void ast_print(AST *ptr) {
    if (ptr == NULL) {
        return;
    }

    AST ast = *ptr;
    switch (ast.tag) {
        case AST_BUILD:
            printf("BUILD(file=%p)", (void *)ast.data.build.file);
            break;
        case AST_TASK:
            printf("TASK(current=%lu, previous=%lu, changed=%d)",
                   ast.data.task.current_hash,
                   ast.data.task.previous_hash,
                   ast.data.task.changed);
            break;
        case AST_RUN:
            printf("RUN(previous=%lu, new=%lu, right=%p)",
                   ast.data.run.previous_hash,
                   ast.data.run.new_hash,
                   (void *)ast.data.run.right);
            break;
        default:
            printf("UNKNOWN_AST");
            break;
    }
}

void ast_free(AST *ptr) {
    if (ptr == NULL) {
        return;
    }

    if (ptr->tag == AST_RUN && ptr->data.run.right != NULL) {
        ast_free(ptr->data.run.right);
    }

    free(ptr);
}

int main(void) {
    AST build = { .tag = AST_BUILD, .data.build.file = NULL };
    AST task = {
        .tag = AST_TASK,
        .data.task = {
            .current_hash = 1000,
            .previous_hash = 999,
            .changed = 1
        }
    };
    AST run = {
        .tag = AST_RUN,
        .data.run = {
            .previous_hash = 99,
            .new_hash = 101,
            .right = NULL
        }
    };

    AST *build_node = ast_new(build);
    AST *task_node = ast_new(task);
    AST *run_node = ast_new(run);

    if (build_node == NULL || task_node == NULL || run_node == NULL) {
        fprintf(stderr, "Failed to allocate AST nodes\n");
        ast_free(build_node);
        ast_free(task_node);
        ast_free(run_node);
        return 1;
    }

    ast_print(build_node);
    printf("\n");
    ast_print(task_node);
    printf("\n");
    ast_print(run_node);
    printf("\n");

    ast_free(build_node);
    ast_free(task_node);
    ast_free(run_node);
    return 0;
} 

