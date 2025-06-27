#include <dos.h>
#include <conio.h>
#include <iostream.h>
#include <math.h>

void printBinary(unsigned char byte) {
    for (int i = 7; i >= 0; --i) {
        cout << ((byte >> i) & 1);
    }
}

unsigned char readStatusWord(int channel) {
    unsigned char command = 0xE0 | (channel << 6);
    outportb(0x43, command);
    return inportb(0x40 + channel);
}

unsigned int readCounter(int channel) {
    outportb(0x43, 0x00 | (channel << 6));
    unsigned char low = inportb(0x40 + channel);
    unsigned char high = inportb(0x40 + channel);
    return (high << 8) | low;
}

void setTimerFrequency(int frequency) {
    const double TIMER_BASE_FREQ = 1193180.0;
    double exact_divisor = TIMER_BASE_FREQ / frequency;
    int divisor = (int)(exact_divisor + 0.5);
    if (divisor < 1) divisor = 1;
    if (divisor > 65535) divisor = 65535;
    outportb(0x43, 0xB6);
    outportb(0x42, divisor & 0xFF);
    outportb(0x42, divisor >> 8);
}

void speakerOn() {
    unsigned char temp = inportb(0x61);
    temp |= 0x03;
    outportb(0x61, temp);
}

void speakerOff() {
    unsigned char temp = inportb(0x61);
    temp &= 0xFC;
    outportb(0x61, temp);
}

void playNote(int frequency, int duration) {
    setTimerFrequency(frequency);
    speakerOn();
    delay(duration);
    speakerOff();
}

void playHappyBirthday() {
    int notes[] = {
        262, 262, 294, 262, 349, 330,
        262, 262, 294, 262, 392, 349,
        262, 262, 523, 440, 349, 330, 294,
        466, 466, 440, 349, 392, 349
    };
    int durations[] = {
        300, 300, 600, 600, 600, 1200,
        300, 300, 600, 600, 600, 1200,
        300, 300, 600, 600, 600, 600, 1200,
        300, 300, 600, 600, 600, 1200
    };
    int numNotes = sizeof(notes) / sizeof(notes[0]);

    for (int i = 0; i < numNotes; ++i) {
        playNote(notes[i], durations[i]);
        delay(90);
    }
}

int main() {
    cout << "Playing Happy Birthday melody..." << endl;
    playHappyBirthday();
    cout << "Melody finished." << endl;

    cout << "\nTimer Channels Information:" << endl;
    for (int channel = 0; channel < 3; ++channel) {
        cout << "Channel " << channel << ":" << endl;
        unsigned char status = readStatusWord(channel);
        cout << "  Status Word (binary): ";
        printBinary(status);
        cout << endl;
        unsigned int counter = readCounter(channel);
        cout << "  Counter CE (hex): " << hex << counter << endl;
    }

    return 0;
}
