#include "stack.h"


int main() {
    printf("started \n");
    printf("started StackConstructor\n");
    StackConstructor(myStack);
    printf("started StackPush\n");
    StackPush(&myStack, 10);
    printf("finished StackPush\n");
    printf("started StackPush\n");
    StackPush(&myStack, 10001);
    printf("started StackPop\n");
    uint64_t a = Stack_tPop(&myStack);
    printf("finished StackPop\n");
    printf("%lu \n", a);
    printf("started StackDestructor\n");
    StackDestructor(&myStack);
    printf("finished all");
}

hash Power(int64_t base, uint64_t power) {
    hash current = 0;
    for (size_t current_power = 0; current_power < power; current_power++) {
        current *= base;
    }
    return current;
}

char* Stack_tpError(int code) {
    switch (code) {
        case STACK_NULL_ERROR:
            return "[!] Stack is NULL [!]";
        case STACK_SIZE_ERROR:
            return "[!] Stack size is negative [!]";
        case NO_ERROR:
            return "All is OK.";
        case DATA_NULL_ERROR:
            return "[!] Data is NULL [!]";
        case CURRENT_INDEX_ERROR:
            return "[!] Current index got invalid value [!]";
        case TRIGGER_MODIFIED_ERROR:
            return "[!] Canary is modified [!]";
        case HASH_MODIFIED_ERROR:
            return "[!] Hash is wrong. Something has changed without control [!]";
        case STACK_DELETION_ERROR:
            return "[!] Stack is called after deletion [!]";
        default:
            return "[!] Something's gone wrong. Can't get the error [!]";
    }
}


void Stack_tDump(Stack_t* stack, int code) {
    FILE* dump_output = fopen(DUMP_FILE_PATH, "a");
    fprintf(dump_output, "DUMP from %s, %s", __DATE__, __TIME__);
    fprintf(dump_output, "ERROR in file '%s' (%d) in stack called %s. Info below:",
           stack->creation.file, stack->error.line, stack->name);
    fprintf(dump_output, "Error reason : %s. \n Error location : \n "
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
        fprintf(dump_output, "Left Data trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
                *(trigger*) (stack->data - ALTERATION),
                *(trigger*) (stack->data - ALTERATION) == INITIAL_TRIGGER ?
                "YES" : "NO", INITIAL_TRIGGER);
        fprintf(dump_output, "Right Stack trigger: %lu. Valid ? %s. Valid trigger: %lu \n",
                *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)),
                *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) == INITIAL_TRIGGER ?
                "YES" : "NO", END_TRIGGER);
    #endif

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
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
    #endif

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

void Stack_tPush(Stack_t* stack, uint64_t value) {
    ASSERTION(stack);

    printf("all good 111\n");

    if (stack->max_size <= stack->current_index) {
        stack->data -= ALTERATION; //shifting back to all bytes to realloc all (to prevent pointer place change)

        uint64_t data_size = stack->max_size * sizeof (stackElement);
        uint64_t new_data_size = (uint64_t) (data_size * MEMORY_RATE);
        uint64_t new_data_size_and_security = new_data_size + SECURITY_SIZE; //increase stack capacity but not security

        stack->data = (uint8_t*) realloc(stack->data, new_data_size_and_security); //(!!) pointer can be changed
        assert("[!] Realloc failed [!]" && stack->data != NULL);
        printf("all good 122\n");

        stack->data += ALTERATION; //passing security coz it's already copied by realloc
        for (size_t current_byte = data_size; current_byte < new_data_size; current_byte += sizeof (stackElement)) {
            *(stackElement*) (stack->data + current_byte) = POISON;
        }

        stack->max_size = new_data_size / sizeof (stackElement);
        *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) = END_TRIGGER;
    }
    printf("all good 132\n");

    *(stackElement*) (stack->data + stack->current_index++ * sizeof (stackElement)) = value;

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        PutHash(stack);
    #endif

    printf("all good 140\n");

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

    stack->max_size = -1; //size is -1 for security
    stack->deletion = LC;
}

void GetHash(Stack_t* stack, hash* current_hash, hash* current_data_hash) {
    uint8_t* current = (uint8_t*) stack;
    for (size_t hashByte = 0; hashByte < sizeof(Stack_t); hashByte++) {
        *current_hash += *(current + hashByte) * Power(HASH_RATE, hashByte);
    }

    uint8_t* current_data = stack->data - ALTERATION;
    for (size_t hashByte = 0; hashByte < stack->max_size * sizeof (stackElement) + SECURITY_SIZE; hashByte++) { //1 extra trigger in the end
        if (*(current_data + hashByte) != POISON) {
            *current_data_hash += *(current_data + hashByte) * Power(HASH_RATE, hashByte);
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
    *(stack->data - sizeof (hash)) = current_data_hash;
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
        *(trigger*) (stack->data + data_and_security_size) = END_TRIGGER;

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

int Stack_tOk(Stack_t* stack, location LC) {
    return NO_ERROR;
    /*
    if (stack == NULL) {
        return STACK_NULL_ERROR;
    }

    if (stack->deletion == DEFAULT_LOCATION && stack->error != NULL) {
        stack->error = LC;
        return STACK_DELETION_ERROR;
    }

    if (stack->creation == DEFAULT_LOCATION) {
        stack->error = LC;
        return LOCATION_NULL_ERROR;
    }

    if (stack->data - ALTERATION == NULL && (stack->max_size > 0 || stack->current_index > 0)) {
        stack->error = LC;
        return DATA_NULL_ERROR;
    }

    if (stack->current_index > stack->max_size || stack->current_index < 0) {
        stack->error = LC;
        return CURRENT_INDEX_ERROR;
    }

    #if (STACK_DEBUG >= FULL_CHECK)
    if (*(trigger*) (stack->data - ALTERATION) != INITIAL_TRIGGER ||
        *(trigger*) (stack->data + stack->max_size * sizeof (stackElement)) != END_TRIGGER) {
            stack->error = LC;
            return TRIGGER_MODIFIED_ERROR;
        }
    #endif

    #if (STACK_DEBUG >= EXPENSIVE_CHECK)
        uint64_t current_hash = 0;
        uint64_t current_data_hash = 0;
        GetHash(stack, &current_hash, &current_data_hash);
        if (current_hash != *(trigger*) (stack->data - ALTERATION + sizeof (trigger))
            || current_data_hash != *(trigger*) (stack->data - ALTERATION + sizeof (trigger) + sizeof (hash)) ) {
            return HASH_MODIFIED_ERROR;
        }
    #endif
    return NO_ERROR;
    */
}
