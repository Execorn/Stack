#include "stack.h"


const long double MEMORY_RATE = 1.61803398875; // 1 + (sqrt(5) - 1) / 2


int my_ceil(float num) {
    int int_num = (int) num;
    if (num == (float) int_num) {
        return int_num;
    }
    return int_num + 1;
}


void Stack_tPush(Stack_t* stack, stackElement value) {
    ASSERTION(stack);


    if (stack->max_size <= stack->current_index) {
        stack->data = IncreaseStackMemory(stack);
    }

    *(stackElement*) (stack->data + stack->current_index++ * sizeof (stackElement)) = value;

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
    PutHash(stack);
    #endif


    ASSERTION(stack);
}

uint8_t* IncreaseStackMemory(Stack_t* stack) {
    ASSERTION(stack);

    stackInfo data_size = stack->max_size * sizeof (stackElement);
    stackInfo new_data_size = (stackInfo) (data_size * my_ceil(MEMORY_RATE));
    stackInfo new_data_size_and_security = new_data_size + SECURITY_SIZE; //increase stack capacity but not security

    #if (STACK_DEBUG >= FULL_CHECK)
    trigger* right_data_trigger = (trigger*) (stack->data + data_size);
    trigger  actual_trigger     = *right_data_trigger;
    *right_data_trigger = 0;
    #endif

    stack->data = (uint8_t*) realloc(stack->data - ALTERATION, new_data_size_and_security);
    // [!!!] pointer CAN be changed
    assert("[!] Realloc failed [!]" && stack->data != NULL);
    stack->data += ALTERATION; //passing security coz it's already copied by realloc
    stack->max_size = stack->max_size * my_ceil(MEMORY_RATE);

    #if (STACK_DEBUG >= FULL_CHECK)
    data_size           = stack->max_size * sizeof (stackElement);
    right_data_trigger  =    (trigger*) (stack->data + data_size);
    *right_data_trigger =                          actual_trigger;
    #endif


    for (stackInfo cur_ind = stack->current_index + 1; cur_ind < stack->max_size; cur_ind++) {
        *(stackElement*) (stack->data + cur_ind * sizeof(stackElement)) = (stackElement) POISON;
    }

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
    PutHash(stack);
    #endif

    ASSERTION(stack);


    return stack->data;
}

uint8_t* DecreaseStackMemory(Stack_t* stack) {
    ASSERTION(stack);

    //stack->data -= ALTERATION; //shifting back to all bytes to realloc all (to prevent pointer place change)

    stackInfo data_size = stack->max_size * sizeof (stackElement);
    stackInfo new_data_size = (stackInfo) (data_size / my_ceil(MEMORY_RATE));
    stackInfo new_data_size_and_security = new_data_size + SECURITY_SIZE; //increase stack capacity but not security

    #if (STACK_DEBUG >= FULL_CHECK)
        trigger* right_data_trigger = (trigger*) (stack->data + data_size);
        trigger  actual_trigger     = *right_data_trigger;
        *right_data_trigger = 0;
    #endif

    stack->data = (uint8_t*) realloc(stack->data - ALTERATION, new_data_size_and_security);
    // [!!!] pointer CAN be changed
    assert("[!] Realloc failed [!]" && stack->data != NULL);
    stack->data += ALTERATION;
    stack->max_size /= my_ceil(MEMORY_RATE);

    #if (STACK_DEBUG >= FULL_CHECK)
        data_size = stack->max_size * sizeof (stackElement);
        right_data_trigger = (trigger*) (stack->data + data_size);
        *right_data_trigger = actual_trigger;
    #endif


    for (stackInfo cur_ind = stack->current_index + 1; cur_ind < stack->max_size; cur_ind++) {
        *(stackElement*) (stack->data + cur_ind * sizeof(stackElement)) = (stackElement) POISON;
    }

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
    PutHash(stack);
    #endif

    ASSERTION(stack);

    return stack->data;
}

stackElement Stack_tPop(Stack_t* stack) {
    ASSERTION(stack);


    if (stack->max_size >= my_ceil(MEMORY_RATE) * stack->current_index) {
        stack->data      = DecreaseStackMemory(stack);
    }


    stackElement value = *(stackElement*) (stack->data + (--stack->current_index) * sizeof (stackElement));
    *(stackElement*) (stack->data + stack->current_index * sizeof(stackElement)) = (stackElement) POISON;

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
    PutHash(stack);
    #endif


    ASSERTION(stack);
    return value;
}


stackInfo PushElementByIndex(Stack_t* stack, stackElement val, stackInfo wheretopush) {
    ASSERTION(stack);
    assert("[!] Stack overflow (DEV ONLY FUNCTION) [!]" && stack->max_size <= wheretopush);

    *(stackElement*) (stack->data + (wheretopush) * sizeof(stackElement)) = val; // pushing BY INDEX

    ASSERTION(stack);
    return 0;
}

stackElement PopElementByIndex(Stack_t* stack, stackInfo popidx) {
    ASSERTION(stack);

    stackElement value = *(stackElement*)(stack->data + popidx * sizeof(stackElement));
    *(stackElement*)(stack->data + popidx * sizeof(stackElement)) = (stackElement) POISON;

    ASSERTION(stack);
    return value;
}

void GetHash(Stack_t* stack, hash* current_hash, hash* current_data_hash) {
    assert("[!] In GetHash: Stack == NULL" && stack != NULL);

    uint8_t* current = (uint8_t*) stack + sizeof (trigger);
    hash current_stack_power = 1;
    for (size_t hashByte = 0; hashByte < sizeof(Stack_t) - 2 * sizeof (trigger); hashByte++) {
        *current_hash       += *(current + hashByte) * current_stack_power;
        current_stack_power *= HASH_RATE;
    }
    hash current_data_power = 1;
    for (size_t hashByte = 0; hashByte < stack->max_size * sizeof (stackElement); hashByte++) { //no extra trigger in the end
        *current_data_hash  += *(stack->data + hashByte) * current_data_power;
        current_data_power *= HASH_RATE;
    }
}

void PutHash(Stack_t* stack) {
#if (STACK_DEBUG < EXPENSIVE_CHECK)
    return;
#endif
    assert("[!] Stack pointer is NULL [!]" && stack != NULL);

    hash current_hash = 0, current_data_hash = 0;
    GetHash(stack, &current_hash, &current_data_hash);

    *(hash*) (stack->data - sizeof (hash) * 2)  = current_hash;
    *(hash*) (stack->data - sizeof (hash))      = current_data_hash;
}

stackErrno CheckStackPointer(Stack_t* stack) {
    if (stack == NULL) {
        return STACK_NULL_ERROR;
    }
    return NO_ERROR;
}

stackErrno CheckDataPointer(Stack_t* stack) {
    if (stack->data - ALTERATION == NULL) {
        return DATA_NULL_ERROR;
    }
    if (stack->data - ALTERATION == (uint8_t*) EMPTY_ELEMENT) {
        return STACK_DELETED_ERROR;
    }
    return NO_ERROR;
}

stackErrno CheckDataSize(Stack_t* stack) {
    if (stack->max_size < 0) {
        return STACK_SIZE_ERROR;
    }
    return NO_ERROR;
}

stackErrno CheckCurrentIndex(Stack_t* stack) {
    if (stack->current_index > stack->max_size) {
        return OVERFLOW;
    }
    if (stack->current_index < 0) {
        return UNDERFLOW;
    }
    return NO_ERROR;
}

stackErrno CheckTriggers(Stack_t* stack) {
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
    if (*(trigger*) (stack->data + data_size)      != END_TRIGGER) {
        return END_DATA_TRIGGER_MODIFIED;
    }
    return NO_ERROR;
}

stackErrno CheckHash(Stack_t* stack) {
    hash current_hash      = *(hash*) (stack->data - sizeof (hash) * 2); // so pointer to trigger passes
    hash current_data_hash = *(hash*) (stack->data - sizeof(hash));      // pointer to previous hash also passes

    hash check_hash = 0, check_data_hash = 0;
    GetHash(stack, &check_hash, &check_data_hash);

    return current_hash != check_hash || current_data_hash != check_data_hash;
}

stackErrno FinalCheck(Stack_t* stack) {
    stackErrno (*checks[])() = {CheckStackPointer, CheckDataPointer, CheckDataSize,
                                CheckCurrentIndex, CheckTriggers,    CheckHash};

    stackErrno checker = 0;
    stackInfo functions_completion_count = 3 + STACK_DEBUG; //so here is the number of functions completed (from 4 to 6)
    for (size_t func = 0; func < functions_completion_count; func++) {
        checker = checks[func](stack);
        if (checker) {
            return checker;
        }
    }
    return checker;
}


void Stack_tDump(Stack_t* stack, int code) {
    FILE* dump_output = fopen("dump.txt", "a"); // or DUMP_FILE_PATH
    assert("[!] Dump file can't be opened [!]" && dump_output != NULL);
    setvbuf(dump_output, NULL, _IONBF, 0);

    fprintf(dump_output, "DUMP from %s, %s \n", __DATE__, __TIME__);
    fprintf(dump_output, "ERROR in file '%s' (%d) in stack called '%s'. Info below: \n",
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
    fprintf(dump_output, "Left Stack trigger: %lu. Valid ? %s. Valid trigger: %u \n",
            stack->initial_trigger, stack->initial_trigger == INITIAL_TRIGGER ?
                                    "YES" : "NO", INITIAL_TRIGGER);
    fprintf(dump_output, "Right Stack trigger: %lu. Valid ? %s. Valid trigger: %u \n",
            stack->end_trigger, stack->end_trigger == END_TRIGGER ?
                                "YES" : "NO", END_TRIGGER);
    if (stack->data != NULL && stack->data != (uint8_t*) EMPTY_ELEMENT) {
        fprintf(dump_output, "Left Data trigger: %lu. Valid ? %s. Valid trigger: %u \n",
                *(trigger*) (stack->data - ALTERATION),
                *(trigger*) (stack->data - ALTERATION) == INITIAL_TRIGGER ?
                "YES" : "NO", INITIAL_TRIGGER);
        fprintf(dump_output, "Right Stack trigger: %lu. Valid ? %s. Valid trigger: %u \n",
                *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)),
                *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) == INITIAL_TRIGGER ?
                "YES" : "NO", END_TRIGGER);
    }
#endif

#if (STACK_DEBUG >= EXPENSIVE_CHECK)
    if (stack->data != NULL && stack->data != (uint8_t*) EMPTY_ELEMENT) {
        hash current_hash = 0, current_data_hash = 0;
        GetHash(stack, &current_hash, &current_data_hash);
        fprintf(dump_output, "Stack hash: %lu. Valid ? %s. Valid hash: %lu\n",
                *(trigger*) (stack->data - ALTERATION + sizeof (trigger)),
                *(trigger*) (stack->data - ALTERATION + sizeof (trigger)) == current_hash ?
                "YES" : "NO", current_hash);
        fprintf(dump_output, "Data hash: %lu. Valid ? %s. Valid hash: %lu\n",
                *(trigger*) (stack->data - ALTERATION + sizeof (trigger) + sizeof (hash)),
                *(trigger*) (stack->data - ALTERATION + sizeof (trigger) + (sizeof (hash))) == current_data_hash ?
                "YES" : "NO", current_data_hash);
    }
#endif

    if (stack->data != NULL && stack->data != (uint8_t*) EMPTY_ELEMENT) {
        for (size_t current_byte = 0;
             current_byte < stack->max_size * sizeof (stackElement);
             current_byte += sizeof (stackElement)) {
            stackElement current_value = *(stackElement*) (stack->data + current_byte);
            fprintf(dump_output, "Value: %s. Valid ? %s. Address: %p. Index: %lu\n",
                    current_value,
                    current_value != (stackElement) POISON ? "YES" : "POISON", &current_value,
                    current_byte / sizeof (stackElement));
        }
    }

    fprintf(dump_output,
            "\n\n         --------------------------------------         \n\n"); //dump separator
    fclose(dump_output);
}

char* Stack_tpError(stackErrno code) {
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
            return "[!] Something's gone wrong. Can't get the error [!]"; // NULL here or error code in string
    }
}



void MakeStack(Stack_t* stack) {
    assert("[!] Stack pointer is NULL [!]" && stack != NULL);

    stack->max_size      = INITIAL_SIZE;
    stack->current_index = 0;
    stack->deletion      = DEFAULT_LOCATION;
    stack->error         = DEFAULT_LOCATION;

    stackInfo data_size = stack->max_size * sizeof (stackElement);
    stackInfo data_and_security_size = data_size + SECURITY_SIZE;

    stack->data = (uint8_t*) calloc(data_and_security_size, sizeof(stack->data[0]));
    assert("[!] Couldn't allocate memory [!]" && stack->data != NULL);

    stack->data += ALTERATION; // [!] ATTENTION [!] shifting pointer into data start
#if (STACK_DEBUG >= FULL_CHECK)
    stack->initial_trigger = INITIAL_TRIGGER;

    *(trigger*) (stack->data - ALTERATION) = INITIAL_TRIGGER;
    *(trigger*) (stack->data + data_size)  =     END_TRIGGER;

    stack->end_trigger = END_TRIGGER;
#endif

    assert("[!] Data pointer is NULL [!]" && stack->data != NULL);

    for (size_t current_byte = 0; current_byte < data_size; current_byte += sizeof (stackElement)) {
        *(stackElement*) (stack->data + current_byte) = (stackElement) POISON; // poisoning all elements at default
    }

#if (STACK_DEBUG >= EXPENSIVE_CHECK)
    PutHash(stack);
#endif
}

void DeleteStack(Stack_t* stack, location LOCATION) {
    ASSERTION(stack);


    free(stack->data -= ALTERATION);
    stack->data = (uint8_t*) EMPTY_ELEMENT;

    stack->max_size = -1; //size is -1 for security
    stack->deletion = LOCATION;
    stack->data  += ALTERATION; //shifting pointer back for ASSERTION to work correctly
}