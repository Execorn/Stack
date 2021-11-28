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

#define PRINT_DEBUG 1

#if (!defined(STACK_DEBUG))
    #define STACK_DEBUG LIGHT_CHECK
#endif

#if (!defined(PRINT_DEBUG))
    #define PRINT_DEBUG 0
#endif

#define __LOCATION__  __DATE__, __TIME__, __FILE__, __FUNCTION__, __LINE__
#define DEFAULT_LOCATION ((location) {NULL, NULL, NULL, NULL, -1})

#define StackConstructor(var) \
    Stack_t       var;        \
    (var).name = #var;    \
    (var).creation = (location) {__LOCATION__}; \
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

typedef uint8_t* stackElement;
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
    NO_ERROR = 0,
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

enum Security{
#if (STACK_DEBUG == FULL_CHECK)
    SECURITY_SIZE = 2 * sizeof (trigger),
    ALTERATION    =     sizeof (trigger),
#elif (STACK_DEBUG >= EXPENSIVE_CHECK)
    SECURITY_SIZE = 2 * sizeof (trigger) + 2 * sizeof (hash),
    ALTERATION    =     sizeof (trigger) + 2 * sizeof (hash),
#else
    SECURITY_SIZE = 0,
    ALTERATION    = 0,
#endif
    INITIAL_SIZE     =         32,
    EMPTY_ELEMENT    =     0xca11,
    POISON           = 0xdeaddead,

#if (STACK_DEBUG >= FULL_CHECK)
    INITIAL_TRIGGER  = 0xDEADBEEF,
    END_TRIGGER      = 0xdeadfa11,
#endif
    HASH_RATE        =        263,
};
// const char* DUMP_FILE_PATH = "/mnt/c/Users/Legion/CLionProjects/Stack/Dump.txt"; //change here to your dump file

extern const long double MEMORY_RATE; // 1 + (sqrt(5) - 1) / 2

char* Stack_tpError(stackErrno code);

void Stack_tDump(Stack_t* stack, stackErrno code);

void Stack_tPush(Stack_t* stack, stackElement value);

stackInfo PushElementByIndex(Stack_t* stack, stackElement val, stackInfo wheretopush);

stackElement PopElementByIndex(Stack_t* stack, stackInfo popidx);

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

uint8_t* IncreaseStackMemory(Stack_t* stack);

uint8_t* DecreaseStackMemory(Stack_t* stack);

int my_ceil(float num);

#endif //STACK_STACK_H
