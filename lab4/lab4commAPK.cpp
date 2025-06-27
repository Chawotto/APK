/* Подключаем необходимые заголовочные файлы */
#include <dos.h>       // Для работы с портами ввода/вывода и прерываниями
#include <stdio.h>      // Стандартный ввод/вывод
#include <stdlib.h>     // Стандартные функции (system, exit)
#include <conio.h>      // Консольный ввод/вывод (getch)
#include <io.h>         // Ввод/вывод через порты (outp, inp)
#include <windows.h>    // Для работы с Windows API (не используется напрямую)

/* Определяем константы для работы с CMOS */
#define CMOS_ADDR  0x70  // Адресный порт CMOS
#define CMOS_DATA  0x71  // Порт данных CMOS
#define PIC_MASK   0xA1  // Порт маскирования прерываний
#define STATUS_REG_A 0x0A  // Регистр состояния A
#define STATUS_REG_B 0x0B  // Регистр состояния B
#define STATUS_REG_C 0x0C  // Регистр состояния C

/* Перечисление для компонентов даты/времени */
typedef enum {
    SECONDS,    // 0 - секунды
    MINUTES,    // 1 - минуты
    HOURS,      // 2 - часы
    DAY,        // 3 - день
    MONTH,      // 4 - месяц
    YEAR        // 5 - год
} TimeField;

/* Глобальные переменные */
unsigned int delayCounter = 0;  // Счетчик для задержки
unsigned int delayTarget = 0;   // Целевое значение задержки (мс)
unsigned int timeData[6];       // Массив для хранения времени и даты

/* Регистры CMOS для чтения времени */
const unsigned int timeRegisters[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 };
/* Регистры для установки времени */
const unsigned int settableRegisters[] = { 0x00, 0x02, 0x04 };
/* Регистры для установки будильника */
const unsigned int alarmRegisters[] = { 0x01, 0x03, 0x05 };

/* Указатели на старые обработчики прерываний */
void interrupt(*oldDelay)(...);
void interrupt(*oldAlarm)(...);

/* Прототипы функций */
unsigned int bcdToDec(unsigned int bcd);  // Преобразование BCD в десятичное
unsigned int decToBcd(unsigned int dec);  // Преобразование десятичного в BCD
void clearInputBuffer();                  // Очистка буфера ввода
void getDateTime();                       // Получение текущей даты и времени
void setDateTime();                       // Установка нового времени
void inputTime();                         // Ввод времени с клавиатуры
void setDelay(unsigned int ms);           // Установка задержки
void setAlarm();                          // Установка будильника
void setFrequency();                      // Установка частоты обновления

/* Обработчик прерывания для задержки */
void interrupt delayHandler(...) {
    delayCounter++;  // Увеличиваем счетчик

    // Считываем регистр C для очистки прерывания
    outp(CMOS_ADDR, STATUS_REG_C);
    inp(CMOS_DATA);

    // Посылаем сигнал конца прерывания контроллерам прерываний
    outp(0x20, 0x20);
    outp(0xA0, 0x20);

    // Проверяем, достигли ли целевого значения
    if(delayCounter >= delayTarget) {
        puts("Delay complete");
        disable();  // Запрещаем прерывания
        // Восстанавливаем старый обработчик
        setvect(0x70, oldDelay);
        enable();   // Разрешаем прерывания
        // Отключаем прерывание задержки
        outp(CMOS_ADDR, STATUS_REG_B);
        outp(CMOS_DATA, inp(CMOS_DATA) & ~0x40);
    }
}

/* Обработчик прерывания будильника */
void interrupt alarmHandler(...) {
    puts("\nAlarm triggered!");  // Выводим сообщение

    // Считываем регистр C для очистки прерывания
    outp(CMOS_ADDR, STATUS_REG_C);
    inp(CMOS_DATA);

    // Посылаем сигнал конца прерывания
    outp(0x20, 0x20);
    outp(0xA0, 0x20);

    disable();  // Запрещаем прерывания
    // Восстанавливаем старый обработчик
    setvect(0x70, oldAlarm);
    enable();   // Разрешаем прерывания
    // Отключаем прерывание будильника
    outp(CMOS_ADDR, STATUS_REG_B);
    outp(CMOS_DATA, inp(CMOS_DATA) & ~0x20);
}

/* Функция получения текущей даты и времени */
void getDateTime() {
    unsigned char status;
    printf("\nCurrent date and time:\n");

// Читаем все компоненты времени
    for(int i = 0; i < 6; i++) {
        // Ждем, пока не обновится регистр
        do {
            outp(CMOS_ADDR, STATUS_REG_A);
            status = inp(CMOS_DATA);
        } while(status & 0x80);  // Проверяем бит обновления

        // Читаем значение из соответствующего регистра
        outp(CMOS_ADDR, timeRegisters[i]);
        timeData[i] = bcdToDec(inp(CMOS_DATA));
    }

    // Выводим дату и время в формате ДД.ММ.ГГГГ ЧЧ:ММ:СС
    printf("Date: %02u.%02u.20%02u\n"
           "Time: %02u:%02u:%02u\n\n",
           timeData[DAY], timeData[MONTH], timeData[YEAR],
           timeData[HOURS], timeData[MINUTES], timeData[SECONDS]);
}

/* Функция установки нового времени */
void setDateTime() {
    inputTime();  // Получаем новое время от пользователя

    disable();  // Запрещаем прерывания

    unsigned char status;
    // Ждем, пока можно будет обновлять время
    do {
        outp(CMOS_ADDR, STATUS_REG_A);
        status = inp(CMOS_DATA);
    } while(status & 0x80);

    // Устанавливаем бит SET для запрета автоматического обновления
    outp(CMOS_ADDR, STATUS_REG_B);
    outp(CMOS_DATA, inp(CMOS_DATA) | 0x80);

    // Записываем новые значения времени
    for(int i = 0; i < 3; i++) {
        outp(CMOS_ADDR, settableRegisters[i]);
        outp(CMOS_DATA, timeData[i]);
    }

    // Сбрасываем бит SET, разрешая обновление
    outp(CMOS_ADDR, STATUS_REG_B);
    outp(CMOS_DATA, inp(CMOS_DATA) & ~0x80);

    enable();  // Разрешаем прерывания
    system("cls");  // Очищаем экран
    printf("Time has been updated.\n");
}

/* Функция ввода времени с клавиатуры */
void inputTime() {
    int value;

    printf("\nEnter new time:\n");

    // Ввод часов с проверкой
    do {
        clearInputBuffer();
        printf("Hours (0-23): ");
    } while(scanf("%d", &value) != 1  value < 0  value > 23);
    timeData[HOURS] = decToBcd(value);

    // Ввод минут с проверкой
    do {
        clearInputBuffer();
        printf("Minutes (0-59): ");
    } while(scanf("%d", &value) != 1  value < 0  value > 59);
    timeData[MINUTES] = decToBcd(value);

    // Ввод секунд с проверкой
    do {
        clearInputBuffer();
        printf("Seconds (0-59): ");
    } while(scanf("%d", &value) != 1  value < 0  value > 59);
    timeData[SECONDS] = decToBcd(value);
}

/* Функция установки будильника */
void setAlarm() {
    inputTime();  // Получаем время будильника

    disable();  // Запрещаем прерывания

    // Сохраняем старый обработчик и устанавливаем новый
    oldAlarm = getvect(0x70);
    setvect(0x70, alarmHandler);
    // Разрешаем прерывания от RTC
    outp(PIC_MASK, inp(PIC_MASK) & 0xFE);

    // Ждем, пока можно будет обновлять регистры
    unsigned char status;
    do {
        outp(CMOS_ADDR, STATUS_REG_A);
        status = inp(CMOS_DATA);
    } while(status & 0x80);

    // Устанавливаем время будильника
    for(int i = 0; i < 3; i++) {
        outp(CMOS_ADDR, alarmRegisters[i]);
        outp(CMOS_DATA, timeData[i]);
    }

    enable();  // Разрешаем прерывания

    // Включаем прерывание будильника
    outp(CMOS_ADDR, STATUS_REG_B);
    outp(CMOS_DATA, inp(CMOS_DATA) | 0x20);

    printf("Alarm has been set.\n");
}

/* Функция установки задержки */
void setDelay(unsigned int ms) {
    delayCounter = 0;  // Сбрасываем счетчик
    delayTarget = ms;  // Устанавливаем целевое значение

    disable();  // Запрещаем прерывания

    // Сохраняем старый обработчик и устанавливаем новый
    oldDelay = getvect(0x70);
    setvect(0x70, delayHandler);
    // Разрешаем прерывания от RTC
    outp(PIC_MASK, inp(PIC_MASK) & 0xFE);
    enable();  // Разрешаем прерывания

    // Включаем прерывание периодического сигнала
    outp(CMOS_ADDR, STATUS_REG_B);
    outp(CMOS_DATA, inp(CMOS_DATA) | 0x40);
}

Илюха Бугор, [14.04.2025 12:57]
/* Функция установки частоты обновления */
void setFrequency() {
    int freq;
    printf("\nSelect frequency:\n");
    printf("1. 2 Hz\n2. 4 Hz\n3. 8 Hz\n4. 16 Hz\n5. 32 Hz\n");
    printf("6. 64 Hz\n7. 128 Hz\n8. 256 Hz\n9. 512 Hz\n");
    printf("10. 1024 Hz\n11. 2048 Hz\n12. 4096 Hz\n13. 8192 Hz\n");
    printf("Enter choice (1-13): ");

    int choice;
    scanf("%d", &choice);

    // Проверка корректности ввода
    if(choice < 1 || choice > 13) {
        printf("Invalid choice!\n");
        return;
    }

    // Вычисляем значение для регистра
    freq = 0x0F - choice + 1;

    disable();  // Запрещаем прерывания
    outp(CMOS_ADDR, STATUS_REG_A);
    // Устанавливаем новую частоту (биты 0-3)
    outp(CMOS_DATA, (inp(CMOS_DATA) & 0xF0) | freq);
    enable();  // Разрешаем прерывания

    printf("Frequency set to %d Hz\n", 1 << choice);
}

/* Функция очистки буфера ввода */
void clearInputBuffer() {
    while(getchar() != '\n');  // Читаем символы до конца строки
}

/* Преобразование из BCD в десятичное */
unsigned int bcdToDec(unsigned int bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

/* Преобразование из десятичного в BCD */
unsigned int decToBcd(unsigned int dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

/* Главная функция */
int main() {
    while(1) {
        // Выводим меню
        printf("\nRTC Controller Menu:\n");
        printf("1. Show current date and time\n");
        printf("2. Set time\n");
        printf("3. Set alarm\n");
        printf("4. Set delay\n");
        printf("5. Set frequency\n");
        printf("0. Exit\n\n");
        printf("Enter your choice: ");

        int choice = getch();  // Читаем выбор без ожидания Enter
        printf("%c\n", choice);  // Эхо выбранного пункта

        // Обрабатываем выбор пользователя
        switch(choice) {
            case '1':  // Показать время
                system("cls");
                getDateTime();
                break;

            case '2':  // Установить время
                system("cls");
                setDateTime();
                break;

            case '3':  // Установить будильник
                setAlarm();
                break;

            case '4':  // Установить задержку
                system("cls");
                printf("Enter delay in milliseconds: ");
                scanf("%u", &delayTarget);
                setDelay(delayTarget);
                break;

            case '5':  // Установить частоту
                system("cls");
                setFrequency();
                break;

            case '0':  // Выход
                printf("\nExiting program.\n");
                return 0;

            default:  // Неверный ввод
                printf("\nInvalid choice. Try again.\n");
                break;
        }
    }
}
