.intel_syntax noprefix
.global plus, main

plus:
        add rsi, rdi/*rsi = rsi + rdi*/
        mov rax, rsi/*rax = rsi*/
        ret/*スタックからアドレスをポップしジャンプ*/

main:
        mov rdi, 3
        mov rsi, 4
        call plus
        ret
