#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define COLORS 1

struct VIDEO
{
    unsigned char symbol;
    unsigned char attribute;
};

unsigned char colors[COLORS] = { 0x71 };
char color = 0x89;

void changeColor()
{
    color = colors[rand() % COLORS];
    return;
}

void print()
{
    char temp;
    int i, val;
    VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);

    val = inp(0x21);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
    screen++;

    val = inp(0xA1);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
    screen += 63;

    outp(0x20, 0x0A);
    val = inp(0x20);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
    screen++;

    outp(0xA0, 0x0A);
    val = inp(0xA0);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
    screen += 63;

    outp(0x20, 0x0B);
    val = inp(0x20);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
    screen++;

    outp(0xA0, 0x0B);
    val = inp(0xA0);
    for (i = 0; i < 8; i++)
    {
        temp = val % 2;
        val = val >> 1;
        screen->symbol = temp + '0';
        screen->attribute = color;
        screen++;
    }
}

void interrupt(*oldVector8)(...);
void interrupt(*oldVector9)(...);
void interrupt(*oldVector10)(...);
void interrupt(*oldVector11)(...);
void interrupt(*oldVector12)(...);
void interrupt(*oldVector13)(...);
void interrupt(*oldVector14)(...);
void interrupt(*oldVector15)(...);
void interrupt(*oldVector70)(...);
void interrupt(*oldVector71)(...);
void interrupt(*oldVector72)(...);
void interrupt(*oldVector73)(...);
void interrupt(*oldVector74)(...);
void interrupt(*oldVector75)(...);
void interrupt(*oldVector76)(...);
void interrupt(*oldVector77)(...);

void interrupt newVector8(...) { print(); oldVector8(); }
void interrupt newVector9(...) { changeColor(); print(); oldVector9(); }
void interrupt newVector10(...) { changeColor(); print(); oldVector10(); }
void interrupt newVector11(...) { changeColor(); print(); oldVector11(); }
void interrupt newVector12(...) { changeColor(); print(); oldVector12(); }
void interrupt newVector13(...) { changeColor(); print(); oldVector13(); }
void interrupt newVector14(...) { changeColor(); print(); oldVector14(); }
void interrupt newVector15(...) { changeColor(); print(); oldVector15(); }
void interrupt newVector70(...) { changeColor(); print(); oldVector70(); }
void interrupt newVector71(...) { changeColor(); print(); oldVector71(); }
void interrupt newVector72(...) { changeColor(); print(); oldVector72(); }
void interrupt newVector73(...) { changeColor(); print(); oldVector73(); }
void interrupt newVector74(...) { changeColor(); print(); oldVector74(); }
void interrupt newVector75(...) { changeColor(); print(); oldVector75(); }
void interrupt newVector76(...) { changeColor(); print(); oldVector76(); }
void interrupt newVector77(...) { changeColor(); print(); oldVector77(); }

void intialize() {
    oldVector8 = getvect(0x08);
    oldVector9 = getvect(0x09);
    oldVector10 = getvect(0x0A);
    oldVector11 = getvect(0x0B);
    oldVector12 = getvect(0x0C);
    oldVector13 = getvect(0x0D);
    oldVector14 = getvect(0x0E);
    oldVector15 = getvect(0x0F);

    setvect(0xA0, newVector9);
    setvect(0xA1, newVector10);
    setvect(0xA2, newVector11);
    setvect(0xA3, newVector12);
    setvect(0xA4, newVector13);
    setvect(0xA5, newVector14);
    setvect(0xA6, newVector15);

    _disable();

    outp(0x20, 0x11);
    outp(0x21, 0xA0);
    outp(0x21, 0x04);
    outp(0x21, 0x01);

    outp(0xA0, 0x11);
    outp(0xA1, 0x08);
    outp(0xA1, 0x04);
    outp(0xA1, 0x01);

    _enable();
}

int main() {
    unsigned far *fp;
    intialize();
    system("cls");

    printf("                   -  MASK\n");
    printf("                   -  REQUEST\n");
    printf("                   -  SERVICE\n");
    printf(" MASTER   SLAVE\n");

    FP_SEG(fp) = _psp;
    FP_OFF(fp) = 0x2c;
    _dos_freemem(*fp);
    _dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);
    return 0;
}
