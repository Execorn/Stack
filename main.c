#include <math.h>
#include "stack.h"


int main() {
    StackConstructor(myStack);
    StackPush(&myStack, 10);StackPush(&myStack, 10001);
    stackElement a = Stack_tPop(&myStack);
    printf("%lu \n", a);
    StackDestructor(&myStack);
    StackPush(&myStack, 100);
}

hash Power(int64_t base, uint64_t power) {
    hash current = 1;
    for (size_t current_power = 0; current_power < power; current_power++) {
        current *= base;
    }
    return current;
}

/*
 *  NO_ERROR,
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
 *
 */

char* Stack_tpError(int code) {
    switch (code) {
        case NO_ERROR:
            return "All is OK.";
        case STACK_NULL_ERROR:
            return "[!] Stack is NULL [!]";
        case DATA_NULL_ERROR:
            return "[!] Data is NULL [!]";
        case STACK_SIZE_ERROR:
            return "[!] Stack size is negative [!]";
        case STACK_DELETED_ERROR:
            return "[!] Stack is already deleted [!]";
        case OVERFLOW:
            return "[!] Stack overflow [!]";
        case UNDERFLOW:
            return "[!] Stack underflow [!]";
        case INITIAL_TRIGGER_MODIFIED:
            return "[!] Left canary is modified [!]";
        case END_TRIGGER_MODIFIED:
            return "[!] Right canary is modified [!]";
        case INITIAL_DATA_TRIGGER_MODIFIED:
            return "[!] Left data canary is modified [!]";
        case END_DATA_TRIGGER_MODIFIED:
            return "[!] Right data canary is modified [!]";
        default:
            return "[!] Something's gone wrong. Can't get the error [!]";
    }
}


void Stack_tDump(Stack_t* stack, int code) {
    FILE* dump_output = fopen(DUMP_FILE_PATH, "a");
    assert("[!] Dump file can't be opened [!]" && dump_output != NULL);

    fprintf(dump_output, "DUMP from %s, %s \n", __DATE__, __TIME__);
    fprintf(dump_output, "ERROR in file '%s' (%d) in stack called %s. Info below: \n",
           stack->creation.file, stack->error.line, stack->name);
    fprintf(dump_output, "Error reason: %s. \nError location:\n"
                         "Date: %s\n"
                         "Time: %s\n"
                         "File: %s\n"
                         "Function: %s\n"
                         "Line: %d\n",
                         Stack_tpError(code), stack->error.date, stack->error.time,
                         stack->error.file, stack->error.function, stack->error.line);

    #if (STACK_DEBUG >= FULL_CHECK)
        fprintf(dump_output, "Left Stack trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
               stack->initial_trigger, stack->initial_trigger == INITIAL_TRIGGER ?
               "YES" : "NO", INITIAL_TRIGGER);
        fprintf(dump_output, "Right Stack trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
               stack->end_trigger, stack->end_trigger == END_TRIGGER ?
               "YES" : "NO", END_TRIGGER);
        if (stack->data != NULL) {
            fprintf(dump_output, "Left Data trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
                    *(trigger*) (stack->data - ALTERATION),
                    *(trigger*) (stack->data - ALTERATION) == INITIAL_TRIGGER ?
                    "YES" : "NO", INITIAL_TRIGGER);
            fprintf(dump_output, "Right Stack trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
                    *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)),
                    *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) == INITIAL_TRIGGER ?
                    "YES" : "NO", END_TRIGGER);
        }
    #endif

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        if (stack->data != NULL) {
            hash current_hash = 0, current_data_hash = 0;
            GetHash(stack, &current_hash, &current_data_hash);
            fprintf(dump_output, "Stack hash: %lu. Valid ? %s. Valid hash: %lu\n",
                    *(trigger*) (stack->data - ALTERATION + sizeof (trigger)),
                    *(trigger*) (stack->data - ALTERATION + sizeof (trigger)) == current_hash ?
                    "YES" : "NO", current_hash);
            fprintf(dump_output, "Stack hash: %lu. Valid ? %s. Valid hash: %lu\n",
                    *(trigger*) (stack->data - ALTERATION + sizeof (trigger) + sizeof (hash)),
                    *(trigger*) (stack->data - ALTERATION + sizeof (trigger) + (sizeof (hash))) == current_data_hash ?
                    "YES" : "NO", current_data_hash);
        }
    #endif

    if (stack->data != NULL) {
        for (size_t current_byte = 0;
        current_byte < stack->max_size * sizeof (stackElement);
        current_byte += sizeof (stackElement)) {
            stackElement current_value = *(stackElement*) (stack->data + current_byte);
            fprintf(dump_output, "Value: %lu. Valid ? %s. Address: %p. Index: %lu\n",
                    current_value,
                    current_value != POISON ? "YES" : "POISON", &current_value,
                    current_byte / sizeof (stackElement));
        }
    }

    fprintf(dump_output, "\n \n \n \n \n \n");
    fclose(dump_output);
}

void Stack_tPush(Stack_t* stack, uint64_t value) {
    ASSERTION(stack);


    if (stack->max_size <= stack->current_index) {
        stack->data -= ALTERATION; //shifting back to all bytes to realloc all (to prevent pointer place change)

        uint64_t data_size = stack->max_size * sizeof (stackElement);
        uint64_t new_data_size = (uint64_t) (data_size * MEMORY_RATE);
        uint64_t new_data_size_and_security = new_data_size + SECURITY_SIZE; //increase stack capacity but not security

        stack->data = (uint8_t*) realloc(stack->data, new_data_size_and_security); //(!!) pointer can be changed
        assert("[!] Realloc failed [!]" && stack->data != NULL);

        stack->data += ALTERATION; //passing security coz it's already copied by realloc
        for (size_t current_byte = data_size; current_byte < new_data_size; current_byte += sizeof (stackElement)) {
            *(stackElement*) (stack->data + current_byte) = POISON;
        }

        stack->max_size = new_data_size / sizeof (stackElement);
        *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) = END_TRIGGER;
    }

    *(stackElement*) (stack->data + stack->current_index++ * sizeof (stackElement)) = value;

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        PutHash(stack);
    #endif


    ASSERTION(stack);
}

stackElement Stack_tPop(Stack_t* stack) {
    ASSERTION(stack);


    if (stack->max_size >= (int64_t) (MEMORY_RATE * stack->current_index)) {
        stack->data -= ALTERATION; //shifting back to all bytes to realloc all (to prevent pointer place change)

        uint64_t data_size = stack->max_size * sizeof (stackElement);
        uint64_t new_data_size = (uint64_t) (data_size / MEMORY_RATE);
        uint64_t new_data_size_and_security = new_data_size + SECURITY_SIZE; //increase stack capacity but not security

        stack->data = (uint8_t*) realloc(stack->data, new_data_size_and_security); //(!!) pointer can be changed
        assert("[!] Realloc failed [!]" && stack->data != NULL);

        stack->data += ALTERATION; //passing security coz it's already copied by realloc

        stack->max_size = new_data_size / sizeof (stackElement);
        *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) = END_TRIGGER;
    }

    assert("[!] Popped element doesn't exist [!]" && stack->current_index > 0);

    stackElement value = *(stackElement*) (stack->data + (--stack->current_index) * sizeof (stackElement));
    *(stackElement*) (stack->data + stack->current_index * sizeof(stackElement)) = POISON;

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        PutHash(stack);
    #endif


    ASSERTION(stack);
    return value;
}

void DeleteStack(Stack_t* stack, location LC) {
    ASSERTION(stack);


    stack->data -= ALTERATION; //shifting back to first byte
    free(stack->data);
    stack->data = NULL;

    stack->max_size = -1; //size is -1 for security
    stack->deletion = LC;
}

void GetHash(Stack_t* stack, hash* current_hash, hash* current_data_hash) {
    assert("[!] In GetHash: Stack == NULL" && stack != NULL);

    uint8_t* current = (uint8_t*) stack + sizeof (trigger);
    for (size_t hashByte = 0; hashByte < sizeof(Stack_t) - 2 * sizeof (trigger); hashByte++) {
        *current_hash += *(current + hashByte) * Power(HASH_RATE, hashByte);
    }

    for (size_t hashByte = 0; hashByte < stack->max_size * sizeof (stackElement); hashByte++) { //no extra trigger in the end
        if (*(stack->data + hashByte) != POISON) {
            *current_data_hash += *(stack->data + hashByte) * Power(HASH_RATE, hashByte);
        }
    }
}

void PutHash(Stack_t* stack) {
    #if (STACK_DEBUG < EXPENSIVE_CHECK)
        return;
    #endif
    assert("[!] Stack pointer is NULL [!]" && stack != NULL);

    uint64_t current_hash = 0;
    uint64_t current_data_hash = 0;
    GetHash(stack, &current_hash, &current_data_hash);

    *(hash*) (stack->data - 2 * sizeof (hash)) = current_hash;
    *(hash*) (stack->data - sizeof(hash)) = current_data_hash;
}

void MakeStack(Stack_t* stack) {
    assert("[!] Stack pointer is NULL [!]" && stack != NULL);

    stack->max_size = INITIAL_SIZE;
    stack->current_index = 0;
    stack->deletion = DEFAULT_LOCATION;
    stack->error = DEFAULT_LOCATION;

    uint64_t data_size = stack->max_size * sizeof (stackElement);
    uint64_t data_and_security_size = data_size + SECURITY_SIZE;

    stack->data = (uint8_t*) calloc(data_and_security_size, sizeof(stack->data[0]));
    assert("[!] Couldn't allocate memory [!]" && stack->data != NULL);

    stack->data += ALTERATION; // shifting pointer into data start
    #if (STACK_DEBUG >= FULL_CHECK)
        stack->initial_trigger = INITIAL_TRIGGER;

        *(trigger*) (stack->data - ALTERATION) = INITIAL_TRIGGER;
        *(trigger*) (stack->data + data_size) = END_TRIGGER;

        stack->end_trigger = END_TRIGGER;
    #endif

    assert("[!] Data pointer is NULL [!]" && stack->data != NULL);

    for (size_t current_byte = 0; current_byte < data_size; current_byte += sizeof (stackElement)) {
        *(stackElement*) (stack->data + current_byte) = POISON;
    }

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        PutHash(stack);
    #endif
}


errno CheckStackPointer(Stack_t* stack) {
    if (stack == NULL) {
        return STACK_NULL_ERROR;
    }
    return NO_ERROR;
}

errno CheckDataPointer(Stack_t* stack) {
    if (stack->data == NULL) {
        return DATA_NULL_ERROR;
    }
    if (stack->data - ALTERATION == (uint8_t*) EMPTY_ELEMENT) {
        return STACK_DELETED_ERROR;
    }
    return NO_ERROR;
}

errno CheckDataSize(Stack_t* stack) {
    if (stack->max_size < 0) {
        return STACK_SIZE_ERROR;
    }
    return NO_ERROR;
}

errno CheckCurrentIndex(Stack_t* stack) {
    if (stack->current_index > stack->max_size) {
        return OVERFLOW;
    }
    if (stack->current_index < 0) {
        return UNDERFLOW;
    }
    return NO_ERROR;
}

errno CheckTriggers(Stack_t* stack) {
    if (stack->initial_trigger  != INITIAL_TRIGGER) {
        return INITIAL_TRIGGER_MODIFIED;
    }
    if (stack->end_trigger != END_TRIGGER) {
        return END_TRIGGER_MODIFIED;
    }
    stackInfo data_size = stack->max_size * sizeof(stackElement);
    if (*(trigger*) (stack->data - ALTERATION) != INITIAL_TRIGGER) {
        return INITIAL_DATA_TRIGGER_MODIFIED;
    }
    if (*(trigger*) (stack->data + data_size) != END_TRIGGER) {
        return END_DATA_TRIGGER_MODIFIED;
    }
    return NO_ERROR;
}

errno CheckHash(Stack_t* stack) {
    hash current_hash = *(hash*) (stack->data - sizeof (hash) * 2); // so pointer to trigger passes
    hash current_data_hash = *(hash*) (stack->data - sizeof(hash)); // pointer to previous hash also passes

    hash check_hash = 0, check_data_hash = 0;
    GetHash(stack, &check_hash, &check_data_hash);

    return current_hash != check_hash || current_data_hash != check_data_hash;
}

errno FinalCheck(Stack_t* stack) {
    assert("[!] Stack pointer is NULL [!]" && stack);

    errno (*checks[])() = {CheckStackPointer, CheckDataPointer, CheckDataSize, CheckCurrentIndex, CheckTriggers,
                           CheckHash};

    errno checker = 0;
    for (size_t func = 0; func < 3 + STACK_DEBUG; func++) {
        checker = checks[func](stack);
        if (checker) {
            return checker;
        }
    }
    return checker;
}
