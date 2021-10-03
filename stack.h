//
// Created by Legion on 01.10.2021.
//

#ifndef STACK_STACK_H
#define STACK_STACK_H

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define LIGHT_CHECK 1
#define FULL_CHECK 2
#define EXPENSIVE_CHECK 3

#define STACK_DEBUG EXPENSIVE_CHECK // change STACK_DEBUG here to change check level

#if (!defined(STACK_DEBUG))
    #define STACK_DEBUG LIGHT_CHECK
#endif

#define __LOCATION__  __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__
#define DEFAULT_LOCATION ((location) {NULL, NULL, NULL, NULL, -1})

#define StackConstructor(var) \
    Stack_t var;        \
    (var).name = #var;    \
    (var).creation = (location) {__LOCATION__ - 2}; \
    MakeStack(&(var));\

#define ASSERTION(var) \
    {                   \
        int code = Stack_tOk(var, (location) {__LOCATION__ - 1});\
        if (code) { \
            Stack_tDump(stack, code);    \
            assert("[!] VERIFY FAILED. VIEW THE DUMP FILE FOR A REPORT. [!]" && 0); \
        } \
    }

#define StackDestructor(var) \
    DeleteStack(var, (location) {__LOCATION__});

#define StackPush(stack, element) \
    Stack_tPush(stack, element)

typedef uint64_t stackElement;
typedef uint64_t trigger;
typedef uint64_t hash;
typedef uint64_t stackInfo;

typedef struct {
    const char* date;
    const char* time;
    const char* file;
    const char* function;
    int line;
} location;

typedef struct {
    #if (STACK_DEBUG >= FULL_CHECK)
        trigger initial_trigger;
    #endif

    location creation;
    const char* name;
    stackInfo max_size;
    stackInfo current_index;
    uint8_t* data;
    location error;
    location deletion;

    #if (STACK_DEBUG >= FULL_CHECK)
        trigger end_trigger;
    #endif
} Stack_t;


enum ERRORS {
    NO_ERROR,
    STACK_NULL_ERROR,
    DATA_NULL_ERROR,
    CURRENT_INDEX_ERROR,
    #if (STACK_DEBUG >= FULL_CHECK)
        TRIGGER_MODIFIED_ERROR,
    #endif
    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        HASH_MODIFIED_ERROR,
    #endif
    STACK_DELETION_ERROR,
    STACK_SIZE_ERROR,
    LOCATION_NULL_ERROR
};

#if (STACK_DEBUG == FULL_CHECK)
    stackInfo SECURITY_SIZE = 2 * sizeof (trigger);
    stackInfo ALTERATION = sizeof (trigger) + 2 * sizeof (hash);
#elif (STACK_DEBUG >= EXPENSIVE_CHECK)
    stackInfo SECURITY_SIZE = 2 * sizeof (trigger) + 2 * sizeof (hash);
    stackInfo ALTERATION = 2 * sizeof (trigger) + 2 * sizeof (hash);
#else
    stackInfo SECURITY_SIZE = 0;
    stackInfo ALTERATION = 0;
#endif

const stackInfo INITIAL_SIZE = 32;
const stackElement EMPTY_ELEMENT = 0xca11ab1e;
const stackElement POISON = 0xdeaddead;

#if (STACK_DEBUG >= FULL_CHECK)
    const stackElement INITIAL_TRIGGER = 0xDEADBEEF;
    const stackElement END_TRIGGER = 0xdeadfa11;
#endif

#if (STACK_DEBUG >= EXPENSIVE_CHECK)
    const hash HASH_RATE = 263;
#endif

const long double MEMORY_RATE = 1.61803398875; // 1 + (sqrt(5) - 1) / 2
const char* DUMP_FILE_PATH = "/mnt/c/Users/Legion/CLionProjects/Stack"; //change here to your dump file

char* Stack_tpError(int code);

void Stack_tDump(Stack_t* stack, int code);

void Stack_tPush(Stack_t* stack, stackElement value);

uint64_t Stack_tPop(Stack_t* stack);

void DeleteStack(Stack_t* stack, location LC);

void GetHash(Stack_t* stack, hash* current_hash, hash* current_data_hash);

void PutHash(Stack_t* stack);

void MakeStack(Stack_t* stack);

int Stack_tOk(Stack_t* stack, location LC);


#endif //STACK_STACK_H
