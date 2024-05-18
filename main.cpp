#include <iostream>
#include <curses.h>
#include <string>
#include <chrono>
#include <vector>
#include <fstream> // Для перенаправления вывода std::clog
#include <cassert>
// TODO: Maybe add some more symbols
std::string symbols = "!@#$%^&*()_+ '\",.:;";

std::vector <int> keySymbols = {KEY_BACKSPACE};

bool check(int ch) {
    if ('a' <= ch && ch <= 'z') return true;
    if ('A' <= ch && ch <= 'Z') return true;
    if ('0' <= ch && ch <= '9') return true;
    for (char i : symbols) {
        if (i == ch) {
            return true;
        }
    }
    for (int i : keySymbols) {
        if (i == ch) {
            return true;
        }
    }
    return false;
}   

void runTypingTest(const std::string& targetText) {
    int textSize = targetText.size();
    enum symbolType {
        COMMON = 1, CORRECT, INCORRECT 
    };   
    // Очищаем экран перед началом теста
    clear();

    // Выводим текст для набора в центре экрана
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax); // Получаем размеры экрана
    int startY = yMax / 2;
    int startX = (xMax - targetText.length()) / 2;
    std::clog << "startY = " << startY << ", startX = " << startX << std::endl;

    // Задаем цветовые пары
    init_pair(COMMON, COLOR_WHITE, COLOR_BLACK);       // Обычный белый текст
    init_pair(CORRECT, COLOR_GREEN, COLOR_BLACK);       // Правильный зелёный текст
    init_pair(INCORRECT, COLOR_RED, COLOR_BLACK);         // Неправильный красный текст

    // Выводим текст для набора с начальным цветом бледно-белого текста
    attron(COLOR_PAIR(COMMON));
    mvprintw(startY, startX, targetText.c_str());
    attroff(COLOR_PAIR(COMMON));
    refresh();

    // Переменные для отслеживания времени и правильности ввода
    auto startTime = std::chrono::steady_clock::now();
    int index = 0; // Индекс текущего символа ввода
    int errors = 0; // Счетчик ошибок
    int indexWrongSymbol = textSize;

    // Основной цикл ввода
    while (index < textSize) {
        int ch = getch(); // Получаем нажатие клавиши

        std::clog << "Read symbol (int)\'" << ch << "\', (char)\'" << (char)ch << '\'' << std::endl;  

        if (!check(ch)) {
            continue;
        }

        std::clog << "index = " << index << ", targetText[index] = \'" << targetText[index] << '\'' << std::endl;


        if (ch == KEY_BACKSPACE) {
            std::clog << "Backspace handling" << std::endl;
            if (index == 0) continue;
            // Обработка клавиши Backspace для удаления последнего символа
            --index; // Уменьшаем индекс на 1
            attron(COLOR_PAIR(COMMON));
            mvaddch(startY, startX + index, targetText[index]); // Очищаем символ на экране
            attroff(COLOR_PAIR(COMMON));
            move(startY, startX + index); // Возвращаемся на предыдущую позицию
        } else if (indexWrongSymbol < index) {
            std::clog << "There is an uncorrected error" << std::endl;
            attron(COLOR_PAIR(INCORRECT));
            mvaddch(startY, startX + index, targetText[index]);
            attroff(COLOR_PAIR(INCORRECT));
            ++index;
        } else if (ch == targetText[index]) {
            std::clog << "Correct symbol handling" << std::endl;
            // Выводим правильно набранный символ зелёным цветом
            indexWrongSymbol = textSize;
            attron(COLOR_PAIR(CORRECT));
            mvaddch(startY, startX + index, ch);
            attroff(COLOR_PAIR(CORRECT));
            ++index;
        } else if (ch != ERR) {
            std::clog << "Wrong symbol handling" << std::endl;
            // Выводим неправильно набранный символ красным текстом
            if (indexWrongSymbol == textSize) {
                indexWrongSymbol = index;
                ++errors; // Увеличиваем счетчик ошибок
            }
            attron(COLOR_PAIR(INCORRECT));
            mvaddch(startY, startX + index, targetText[index]);
            attroff(COLOR_PAIR(INCORRECT));
            ++index;
        } else {
            assert(true);
        }
        
        std::clog << std::endl;
        refresh();
    }

    // Вычисляем время и скорость набора
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    double typingSpeed = (static_cast<double>(textSize) / elapsedSeconds.count()) * 60;
    double percentageOfErrors = (static_cast<double>(errors) / textSize) * 100;
    // TODO: Change printing results
    // Выводим результаты
    mvprintw(startY + 2, 0, "Time taken: %.2f seconds", elapsedSeconds.count());
    mvprintw(startY + 3, 0, "Typing speed: %.2f characters per minute", typingSpeed);
    mvprintw(startY + 4, 0, "Errors: %d, %.2f%%", errors, percentageOfErrors);
    mvprintw(startY + 5, 0, "Press ENTER to try again, or any other key to exit...");
    refresh();

    // Ждем нажатия клавиши
    int key = getch();
    if (key == '\n') {
        // Пользователь решает повторить тест
        runTypingTest(targetText);
    }
}

int main() {
    std::ofstream logFile("program.log");
    std::streambuf* oldClogBuffer = std::clog.rdbuf(logFile.rdbuf());

    std::clog << "Initialization start\n";
    // Инициализация ncurses
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Скрываем курсор
    std::clog << "Initialization end\n"; 

    // TODO: Add the ability to type text from a file
    // TODO: Add the ability to customize the window for text and text under the window
    // Определяем текст для набора
    std::string targetText = "Hello, world!";
    std::clog << "runTypingTest\n";
    // Запускаем тест на набор текста
    runTypingTest(targetText);

    // Завершаем ncurses
    endwin();
    std::clog.rdbuf(oldClogBuffer);
    return 0;
}
