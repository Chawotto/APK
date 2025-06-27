#include <dos.h>
#include <conio.h>
#include <stdio.h>

#define COM1 0x3F8

void init_serial(unsigned short port) {
    outp(port + 1, 0x00);
    outp(port + 3, 0x80);
    outp(port + 0, 0x0C);
    outp(port + 1, 0x00);
    outp(port + 3, 0x03);
    outp(port + 2, 0xC7);
    outp(port + 4, 0x0B);
}

void direct_port_demo(unsigned short port) {
    char ch_send = 'A';
    char ch_recv;
    clrscr();
    printf("Demonstration of direct port access\n");
    printf("Sending character '%c' via COM1\n", ch_send);

    while (!(inp(port + 5) & 0x20));
    outp(port, ch_send);

    printf("Waiting for incoming data (10 sec)...\n");
    delay(10000);
    if (inp(port + 5) & 0x01) {
        ch_recv = inp(port);
        printf("Received character: '%c'\n", ch_recv);
    } else {
        printf("No data received.\n");
    }

    printf("Press any key...\n");
    getch();
}

void bios_int14_demo(void) {
    union REGS regs;
    char ch_send = 'B';
    char ch_recv;

    clrscr();
    printf("Demonstration of BIOS int 14h operation\n");
    printf("Sending character '%c' via COM1\n", ch_send);

    regs.h.ah = 0x01;
    regs.h.al = ch_send;
    regs.x.dx = 0;
    int86(0x14, &regs, &regs);

    printf("Waiting for incoming data (10 sec)...\n");
    delay(10000);
    regs.h.ah = 0x02;
    regs.x.dx = 0;
    int86(0x14, &regs, &regs);
    if (!(regs.h.ah & 0x80)) {
        ch_recv = regs.h.al;
        printf("Received character: '%c'\n", ch_recv);
    } else {
        printf("No data received.\n");
    }

    printf("Press any key...\n");
    getch();
}

void registers_demo(unsigned short port) {
    char ch_send = 'C';
    char ch_recv;
    clrscr();
    printf("Demonstration of register-based operation\n");
    printf("Sending character '%c' via COM1\n", ch_send);

    while (!(inp(port + 5) & 0x20));
    outp(port + 0, ch_send);

    printf("Waiting for incoming data (10 sec)...\n");
    delay(10000);
    if (inp(port + 5) & 0x01) {
        ch_recv = inp(port + 0);
        printf("Received character: '%c'\n", ch_recv);
    } else {
        printf("No data received.\n");
    }

    printf("Press any key...\n");
    getch();
}

int main(void) {
    int choice;
    unsigned short port = COM1;

    do {
        clrscr();
        printf("COM port operation demonstration\n");
        printf("1. Direct port interaction\n");
        printf("2. Using BIOS interrupt 14h\n");
        printf("3. Working with registers\n");
        printf("4. Exit\n");
        printf("Select an option (1-4): ");

        scanf("%d", &choice);

        switch(choice) {
            case 1:
                direct_port_demo(port);
                break;
            case 2:
                bios_int14_demo();
                break;
            case 3:
                init_serial(port);
                registers_demo(port);
                break;
            case 4:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice! Press any key...\n");
                getch();
        }
    } while (choice != 4);

    return 0;
}