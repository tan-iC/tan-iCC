#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//トークンの種類
typedef enum{
    TK_RESERVED, //記号
    TK_NUM, //整数トークン
    TK_EOF, //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

//トークン型
struct Token{
    TokenKind kind; //トークンの型
    Token *next;    //次の入力トークン
    int val;        //kindがTK_NUMの場合、その数値
    char *str;      //トークン文字列 
};

//現在着目しているトークン
Token *token;

//入力プログラム
char *user_input;

//エラー箇所を報告する
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
    fprintf(stderr, "^ ");

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが期待している記号のとき、
//トークンを1つ読み進めて真を返す
//それ以外の場合、負を返す
bool consume(char op){
    if (token->kind != TK_RESERVED || token->str[0] !=op){
        return false;
    }

    token = token->next;
    return true;
}

//次のトークンが期待している記号のとき、
//トークンを1つ読み進める
//それ以外の場合にはエラーを報告する
void expect(char op){
    if (token->kind != TK_RESERVED || token->str[0] != op){
        error_at(token->str, "'%c'ではありません", op);
    }
    token = token->next;
}

//次のトークンが数値の場合、
//トークンを1つ読み進めてその数値を返す
//それ以外の場合にはエラーを報告する
int expect_number(){
    if (token->kind != TK_NUM){
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p){
        //空白文字をスキップ
        if (isspace(*p)){
            p++;
            continue;
        }

        // if (*p == '+' || *p == '-'){
        if (strchr("+-*/()", *p)){
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        if (isdigit(*p)){
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(token->str, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

//抽象構文木のノードの種類
typedef enum {
    ND_ADD, //+
    ND_SUB, //-
    ND_MUL, //*
    ND_DIV, ///
    ND_NUM, //整数
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node{
    NodeKind kind;  //ノードの型
    Node *lhs;      //左辺
    Node *rhs;      //右辺
    int val;        //kindがND_NUMの場合のみ使う
};

//新しいノードを作成する関数
Node *new_node(NodeKind kind){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

//ノードの分岐を作成
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//新しい数字ノードを作成する関数
Node *new_num(int val){
    // Node *node = calloc(1, sizeof(Node));
    Node *node = new_node(ND_NUM);
    // node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *mul();
Node *primary();
Node *unary();

//ノードを用いた式の作成
Node *expr(){
    Node *node = mul();

    for(;;){
        if (consume('+')){
            node = new_binary(ND_ADD, node, mul());

        }
        else if(consume('-')){
            node = new_binary(ND_SUB, node, mul());
        }
        else{
            return node;
        }
    }
}

//優先度1ノード
Node *mul(){
    Node *node = unary();

    for (;;){
        if (consume('*')){
            node = new_binary(ND_MUL, node, unary());
        }
        else if (consume('/')){
            node = new_binary(ND_DIV, node, unary());
        }
        else{
            return node;
        }
    }
}

//優先度2ノード
Node *primary(){
    //次のトークンが"("なら、"("expr")"
    if (consume('(')){
        Node *node = expr();
        expect(')');
        return node;
    }

    //そうでない場合数値
    return new_num(expect_number());
}

/*
expr    = mul ("+" mul | "-" mul)*
mul     = primary ("*" primary | "/" primary)*
primary = num | "(" expr ")"
*/

// 

/*
expr    = mul ("+" mul | "-" mul)*
mul     = unary ("*" unary | "/" unary)*
unary   = ("+" | "-")? primary
primary = num | "(" expr ")"
*/

Node *unary() {
  if (consume('+'))
    return primary();
  if (consume('-'))
    return new_binary(ND_SUB, new_num(0), primary());
  return primary();
}

//スタックマシン化
void gen(Node *node){
    if (node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind){
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv){
    if (argc != 2){
        fprintf(stderr, "引数の個数が正しくありません。\n");
        return 1;
    }

    //トークナイズする
    user_input = argv[1];
    token = tokenize();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    
    // printf("    mov rax, %d\n", expect_number());

    // while(!at_eof()){
    //     if (consume('+')){
    //         printf("    add rax, %d\n", expect_number());
    //         continue;
    //     }

    //     expect('-');
    //     printf("    sub rax, %d\n", expect_number());
    // }

    // printf("    ret\n");

    Node *node = expr();
    gen(node);
    printf("    pop rax\n");
    printf("    ret\n");
    
    return 0;
}
