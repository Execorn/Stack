//
// Created by Legion on 01.10.2021.
//

#ifndef STACK_STACK_H
#define STACK_STACK_H

#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#define     LIGHT_CHECK 1
#define      FULL_CHECK 2
#define EXPENSIVE_CHECK 3

#define STACK_DEBUG EXPENSIVE_CHECK // change STACK_DEBUG here to change check level

#if (!defined(STACK_DEBUG))
    #define STACK_DEBUG LIGHT_CHECK
#endif

#define __LOCATION__  __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__
#define DEFAULT_LOCATION ((location) {NULL, NULL, NULL, NULL, -1})

#define StackConstructor(var) \
    Stack_t       var;        \
    (var).name = #var;    \
    (var).creation = (location) {__LOCATION__ - 2}; \
    MakeStack(&(var));\

#define ASSERTION(var) \
    {                   \
        int code = FinalCheck(var); \
        if (code) {    \
            (var)->error = (location) {__LOCATION__};             \
            Stack_tDump(var, code); \
            assert("[!] VERIFY FAILED. VIEW THE DUMP FILE FOR A REPORT. [!]" && 0); \
        } \
    }

#define StackDestructor(var) \
    DeleteStack(var, (location) {__LOCATION__});

#define StackPush(stack, element) \
    Stack_tPush(stack, element)

#define StackPop(stack) Stack_tPop(stack)

typedef int64_t stackElement;
typedef uint64_t trigger;
typedef uint64_t hash;
typedef int64_t stackInfo;
typedef int stackErrno;

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
    STACK_SIZE_ERROR,
    STACK_DELETED_ERROR,
    OVERFLOW,
    UNDERFLOW,
    INITIAL_TRIGGER_MODIFIED,
    END_TRIGGER_MODIFIED,
    INITIAL_DATA_TRIGGER_MODIFIED,
    END_DATA_TRIGGER_MODIFIED
};

#if (STACK_DEBUG == FULL_CHECK)
    stackInfo SECURITY_SIZE = 2 * sizeof (trigger);
    stackInfo ALTERATION    =     sizeof (trigger);
#elif (STACK_DEBUG >= EXPENSIVE_CHECK)
    stackInfo SECURITY_SIZE = 2 * sizeof (trigger) + 2 * sizeof (hash);
    stackInfo ALTERATION    =     sizeof (trigger) + 2 * sizeof (hash);
#else
    stackInfo SECURITY_SIZE = 0;
    stackInfo ALTERATION    = 0;
#endif

const stackInfo    INITIAL_SIZE  = 32;
const stackElement EMPTY_ELEMENT = 0xca11;
const stackElement POISON        = 0xdeaddead;

#if (STACK_DEBUG >= FULL_CHECK)
    const stackElement INITIAL_TRIGGER = 0xDEADBEEF;
    const stackElement     END_TRIGGER = 0xdeadfa11;
#endif

const hash HASH_RATE = 263;

const long double MEMORY_RATE = 1.61803398875; // 1 + (sqrt(5) - 1) / 2
// const char* DUMP_FILE_PATH = "/mnt/c/Users/Legion/CLionProjects/Stack/Dump.txt"; //change here to your dump file



char* Stack_tpError(stackErrno code);

void Stack_tDump(Stack_t* stack, stackErrno code);

void Stack_tPush(Stack_t* stack, stackElement value);

stackElement Stack_tPop(Stack_t* stack);

void DeleteStack(Stack_t* stack, location LOCATION);

void GetHash(Stack_t* stack, hash* current_hash, hash* current_data_hash);

void PutHash(Stack_t* stack);

void MakeStack(Stack_t* stack);

stackErrno CheckStackPointer(Stack_t* stack);

stackErrno CheckDataPointer(Stack_t* stack);

stackErrno CheckDataSize(Stack_t* stack);

stackErrno CheckCurrentIndex(Stack_t* stack);

stackErrno CheckTriggers(Stack_t* stack);

stackErrno CheckHash(Stack_t* stack);

stackErrno FinalCheck(Stack_t* stack);

hash Power(int64_t base, uint64_t power);


#endif //STACK_STACK_H
