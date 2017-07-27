#include "bf.h"
#include <jit/jit-dump.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>

void myputchar(char c) {
    putchar(c);
}

char mygetchar() {
    return getchar();
}

void add_putchar_insn(jit_function_t function, jit_value_t c) {
    jit_type_t params[] = {jit_type_int};
    jit_type_t putchar_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 1, 0);
    jit_insn_call_native(function, "putchar", myputchar, putchar_signature, &c, 1, 0);
    jit_type_free(putchar_signature);
}

#define TAPE_SIZE 30000
#define STACK_SIZE 2560

Function get_compiled(jit_context_t context, char* program) {
    jit_context_build_start(context);
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, 0, 0, 0);
    jit_function_t function = jit_function_create(context, signature);
    jit_type_free(signature);

    jit_value_t temp1, temp2;
    jit_value_t const0 = jit_value_create_nint_constant(function, jit_type_ubyte, 0);
    jit_value_t const1 = jit_value_create_nint_constant(function, jit_type_ubyte, 1);
    jit_value_t temp_byte = jit_value_create(function, jit_type_ubyte);

    jit_value_t tape_size = jit_value_create_nint_constant(function, jit_type_int, TAPE_SIZE);
    jit_value_t memory = jit_insn_alloca(function, tape_size);
    jit_type_t getchar_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_ubyte, 0, 0, 1);

    jit_label_t *stack[STACK_SIZE] = {0};
    int stack_pos = 0;


    for (char *it = program; *it; it++) {
        switch (*it) {
        case '+': {
            jit_value_t temp = jit_insn_load_relative(function, memory, 0, jit_type_ubyte);
            temp = jit_insn_add(function, temp, const1);
            jit_insn_store(function, temp_byte, temp);
            jit_insn_store_relative(function, memory, 0, temp_byte);
            }
            break;
        case '-': {
            jit_value_t temp = jit_insn_load_relative(function, memory, 0, jit_type_ubyte);
            temp = jit_insn_sub(function, temp, const1);
            jit_insn_store(function, temp_byte, temp);
            jit_insn_store_relative(function, memory, 0, temp_byte);
            }
            break;
        case '>':
            temp1 = jit_insn_load(function, memory);
            temp1 = jit_insn_add(function, temp1, const1);
            jit_insn_store(function, memory, temp1);
            break;
        case '<':
            temp1 = jit_insn_load(function, memory);
            temp1 = jit_insn_sub(function, temp1, const1);
            jit_insn_store(function, memory, temp1);
            break;
        case '.':
            add_putchar_insn(function, jit_insn_load_relative(function, memory, 0, jit_type_ubyte));
            break;
        case ',':
            jit_insn_store_relative(
                function,
                memory,
                0,
                jit_insn_call_native(function, "mygetchar", mygetchar, getchar_signature, 0, 0, 0)
            );
            break;
        case '[':
            if (stack_pos + 2 >= STACK_SIZE) {
                printf("Too many nested loops!\n");
                exit(1);
            }
            stack[stack_pos] = malloc(sizeof(jit_label_t));
            *stack[stack_pos] = jit_label_undefined;
            jit_insn_label(function, stack[stack_pos]);
            stack_pos += 1;
            temp1 = jit_insn_load_relative(function, memory, 0, jit_type_ubyte);
            temp2 = jit_insn_eq(function, temp1, const0);
            stack[stack_pos] = malloc(sizeof(jit_label_t));
            *stack[stack_pos] = jit_label_undefined;
            jit_insn_branch_if(function, temp2, stack[stack_pos]);
            stack_pos += 1;
            break;
        case ']':
            if (stack_pos < 2) {
                printf("Unmatched closing bracket\n");
                exit(1);
            }
            stack_pos -= 2;
            temp1 = jit_insn_load_relative(function, memory, 0, jit_type_ubyte);
            temp2 = jit_insn_eq(function, temp1, const0);
            jit_insn_branch_if_not(function, temp2, stack[stack_pos]);
            jit_insn_label(function, stack[stack_pos+1]);
            break;
        }
    }

    jit_type_free(getchar_signature);

    jit_insn_default_return(function);
    jit_context_build_end(context);

    //jit_dump_function(stdout, function, "run");
    jit_function_compile(function);
    //jit_dump_function(stdout, function, "run");
    Function result = (Function)jit_function_to_closure(function);

   return result;
}

