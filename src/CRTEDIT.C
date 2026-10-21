/* Explicit reconstruction exception: redirect the original startup call.
 * Unmodified LINK output is retained as CIVRAW.EXE. Only four destination
 * bytes change; runtime libraries and all linked objects remain untouched.
 * Historical source/object/post-link mechanism is still unknown.
 * Host acceptance additionally pins both complete files with SHA-256.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char block[512];
static unsigned char before[5] = { 0x9a, 0x48, 0x04, 0xc3, 0x1f };
static unsigned char after[4] = { 0xc1, 0x01, 0xa8, 0x01 };

static unsigned long crc_byte(unsigned long crc, unsigned char value)
{
    int bit;
    crc ^= value;
    for (bit = 0; bit < 8; ++bit)
        crc = (crc & 1) ? (crc >> 1) ^ 0xedb88320UL : crc >> 1;
    return crc;
}

int main(void)
{
    FILE *input, *output;
    unsigned long crc, output_crc;
    long length, position;
    unsigned int count, i;
    input = fopen("CIVRAW.EXE", "rb");
    if (!input)
        return 1;
    crc = 0xffffffffUL;
    length = 0;
    while ((count = fread(block, 1, sizeof(block), input)) != 0) {
        for (i = 0; i < count; ++i)
            crc = crc_byte(crc, block[i]);
        length += count;
    }
    if (ferror(input) || length != 302982L || (crc ^ 0xffffffffUL) != 0x78898c9fUL)
        goto bad_input;
    if (fseek(input, 0x1fcafL, SEEK_SET) || fread(block, 1, 5, input) != 5 || memcmp(block, before, 5) || fseek(input, 0L, SEEK_SET))
        goto bad_input;
    output = fopen("CIV.EXE", "wb");
    if (!output)
        goto bad_input;
    output_crc = 0xffffffffUL;
    position = 0;
    while ((count = fread(block, 1, sizeof(block), input)) != 0) {
        for (i = 0; i < count; ++i) {
            if (position + i >= 0x1fcb0L && position + i < 0x1fcb4L)
                block[i] = after[(int) (position + i - 0x1fcb0L)];
            output_crc = crc_byte(output_crc, block[i]);
        }
        if (fwrite(block, 1, count, output) != count)
            goto bad_output;
        position += count;
    }
    if (ferror(input) || position != length || (output_crc ^ 0xffffffffUL) != 0xfaad94aaUL)
        goto bad_output;
    if (fclose(output)) {
        fclose(input);
        remove("CIV.EXE");
        return 1;
    }
    fclose(input);
    puts("Startup exception: four call-target bytes changed; CIVRAW.EXE retained.");
    return 0;
bad_output:
    fclose(output);
    remove("CIV.EXE");
bad_input:
    fclose(input);
    puts("Startup edit rejected: input/output did not satisfy the pinned checks.");
    return 1;
}
