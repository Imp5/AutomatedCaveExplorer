#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <inttypes.h>


struct ZxScreen
{
    uint8_t page[3][256 * 8];
    uint8_t attr[32 * 24];
} scr;

int attr_addr(int x, int y)
{
    return y * 32 + x;
}

int bits_addr(int x, int y)
{
    unsigned int p = y * 32 + x;
    return (p & 0xff) | ((p & 0xff00) << 3);
}

// x = 0..15
// y = 0..3
// dxdy = 0..3

uint16_t get_sprite_code_offset(unsigned int x, unsigned int y, unsigned int dxdy)
{
    return y * 32 + (x & 1) * 128 + dxdy * 256 + (x / 2) * 256 * 4;
}


int main()
{
    const char* inputFile = "..\\..\\Graphics\\sprites.scr";
    const char* outputFile = "..\\..\\Graphics\\sprites.bin";
    const char* fontFile = "..\\..\\Graphics\\font.bin";

    FILE* in = fopen(inputFile, "rb");
    if (!in)
    {
        printf("file not found\n");
        return 1;
    }

    FILE* out = fopen(outputFile, "wb");
    if (!out)
    {
        fclose(in);
        printf("cannot open for write (sprites)\n");
        return 1;
    }

    FILE* font = fopen(fontFile, "wb");
    if (!out)
    {
        fclose(in);
        fclose(out);
        printf("cannot open for write (font)\n");
        return 1;
    }

    size_t res = fread((void*)&scr, sizeof(scr), 1, in);
    fclose(in);
    if (res != 1)
    {
        printf("error during read\n");
        return 1;
    }


    uint8_t outBuf[1024 * 8] = { 0 };

    memset(outBuf, 0xc7, sizeof(outBuf));

    uint8_t spriteCode[32] = {
      0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0, 0x24, 0x36, 0,
      0x62, 0x36, 0, 0x63, 0x2c, 0xc9, 0, 0, 0
    };


    for (int x = 0; x < 32; x += 2)
    {
        for (int frame = 0; frame < 8; frame += 2)
        {
            for (int dy = 0; dy < 2; dy++)
                for (int dx = 0; dx < 2; dx++)
                {
                    for (int y = 0; y < 8; y++)
                    {
                        int bits = bits_addr(x + dx, frame + dy) + 256 * y;
                        spriteCode[3 * y + 1] = scr.page[0][bits];
                    }
                    spriteCode[25] = scr.attr[attr_addr(x + dx, frame + dy)];

                    int to = get_sprite_code_offset(x / 2, frame / 2, dx + dy * 2);
                    memcpy((char*)outBuf + to, spriteCode, 32);
                }
        }
    }

    fwrite(&outBuf, 1024 * 8, 1, out);


    for (int f = 0; f < 64; f++)
    {
        int frame = 1 + f / 32;
        int x = f % 32;
        for (int y = 0; y < 8; y++)
        {
            int bits = bits_addr(x, frame) + 256 * y;
            fwrite(&(scr.page[1][bits]), 1, 1, font);
        }
    }

    fclose(out);
    fclose(font);
    return 0;
}
