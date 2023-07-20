
#include <epicsStdlib.h>
#include <postfix.h>
#include <string.h>
#include <stdint.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    char* input = (char*)malloc(size+1);
    memcpy(input, data, size);
    input[size] = 0;

    char* out = (char*)alloca(INFIX_TO_POSTFIX_SIZE(strlen(input))+32);

    short error;
    postfix(input, out, &error);

    free(input);
    return 0;
}