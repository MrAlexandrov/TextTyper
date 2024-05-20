#include <iostream>
#include <curses.h>
#include <string>
#include <chrono>
#include <vector>
#include <fstream> // Для перенаправления вывода std::clog
#include <cassert>
#include <sstream>
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

std::vector <std::string> separate(const std::string Text, const int& width) {
    std::vector <std::string> words;
    std::stringstream line(Text);
    std::string word;
    while (line >> word) {
        words.push_back(word);
    }
    #ifdef DEBUG
    std::clog << "words.size() = " << words.size() << '\n';
    #endif
    for (int i = 0; i < words.size(); ++i) {
        if (width < words[i].size()) {
            std::clog << "Can't separate the text\n";
            return {};
        }
    }
    std::clog << "Text can be separated\n";
    for (std::string i : words) {
        std::clog << i << '\n';
    }
    std::vector <std::string> lines;
    for (auto&& current_word : words) {
        if (lines.empty() || lines.back().size() + 1 + current_word.size() > width) {
            lines.push_back(current_word);
        } else {
            lines.back() += ' ';
            lines.back() += current_word;
        }
    }
    for (int i = 0; i < lines.size() - 1; ++i) {
        lines[i] += ' ';
    }
    return lines;
}

void runTypingTest(const std::string& targetText) {
    int textSize = targetText.size();
    enum symbolType {
        COMMON = 1, CORRECT, INCORRECT 
    };   
    // Очищаем экран перед началом теста
    clear();

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax); // Получаем размеры экрана
    int height, width = xMax * 0.8;
    std::clog << "width = " << width << '\n';
    std::vector <std::string> Text = separate(targetText, width);
    if (Text.empty()) {
        return;
    }
    height = Text.size();

    int startY = (yMax - height) / 2;
    int startX = (xMax - width) / 2;
    std::clog << "startY = " << startY << ", startX = " << startX << std::endl;

    // Задаем цветовые пары
    init_pair(COMMON, COLOR_WHITE, COLOR_BLACK);       // Обычный белый текст
    init_pair(CORRECT, COLOR_GREEN, COLOR_BLACK);       // Правильный зелёный текст
    init_pair(INCORRECT, COLOR_RED, COLOR_BLACK);         // Неправильный красный текст


    int current_line = startY;
    // Выводим текст для набора
    attron(COLOR_PAIR(COMMON));
    for (int i = 0; i < Text.size(); ++i) {
        mvprintw(current_line + i, startX, Text[i].c_str());
    }
    attroff(COLOR_PAIR(COMMON));
    refresh();

    // Переменные для отслеживания времени и правильности ввода
    auto startTime = std::chrono::steady_clock::now();
    int index = 0; // Индекс текущего символа ввода
    int errors = 0; // Счетчик ошибок
    int indexWrongSymbol = textSize;

    // Основной цикл ввода
    for (int i = 0; i < height; ++i) {
        index = 0;
        textSize = Text[i].size();
        while (index < textSize) {
            int ch = getch(); // Получаем нажатие клавиши

            std::clog << "Read symbol (int)\'" << ch << "\', (char)\'" << (char)ch << '\'' << std::endl;  

            if (!check(ch)) {
                continue;
            }

            std::clog << "index = " << index << ", targetText[index] = \'" << targetText[index] << '\'' << '\n';

            if (ch == KEY_BACKSPACE) {
                std::clog << "Backspace handling\n";
                if (index == 0) continue;
                // Обработка клавиши Backspace для удаления последнего символа
                --index; // Уменьшаем индекс на 1
                attron(COLOR_PAIR(COMMON));
                mvaddch(startY + i, startX + index, Text[i][index]); // Очищаем символ на экране
                attroff(COLOR_PAIR(COMMON));
                move(startY + i, startX + index); // Возвращаемся на предыдущую позицию
            } else if (indexWrongSymbol < index) {
                std::clog << "There is an uncorrected error\b";
                attron(COLOR_PAIR(INCORRECT));
                mvaddch(startY + i, startX + index, Text[i][index]);
                attroff(COLOR_PAIR(INCORRECT));
                ++index;
            } else if (ch == Text[i][index]) {
                std::clog << "Correct symbol handling\n";
                // Выводим правильно набранный символ зелёным цветом
                indexWrongSymbol = textSize;
                attron(COLOR_PAIR(CORRECT));
                mvaddch(startY + i, startX + index, ch);
                attroff(COLOR_PAIR(CORRECT));
                ++index;
            } else if (ch != ERR) {
                std::clog << "Wrong symbol handling\n";
                // Выводим неправильно набранный символ красным текстом
                if (indexWrongSymbol == textSize) {
                    indexWrongSymbol = index;
                    ++errors; // Увеличиваем счетчик ошибок
                }
                attron(COLOR_PAIR(INCORRECT));
                mvaddch(startY + i, startX + index, Text[i][index]);
                attroff(COLOR_PAIR(INCORRECT));
                ++index;
            } else {
                assert(true);
            }
            
            std::clog << '\n';
            refresh();
        }
    }

    // Вычисляем время и скорость набора
    auto endTime = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    double typingSpeed = (static_cast<double>(targetText.size()) / elapsedSeconds.count()) * 60;
    double percentageOfErrors = (static_cast<double>(errors) / targetText.size()) * 100;
    // TODO: Change printing results
    // Выводим результаты
    mvprintw(startY + height + 2, 0, "Time taken: %.2f seconds", elapsedSeconds.count());
    mvprintw(startY + height + 3, 0, "Typing speed: %.2f characters per minute", typingSpeed);
    mvprintw(startY + height + 4, 0, "Errors: %d, %.2f%%", errors, percentageOfErrors);
    mvprintw(startY + height + 5, 0, "Press ENTER to try again, or any other key to exit...");
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
    // std::string targetText = "The quick brown fox jumps over the lazy dog near the riverbank while the sun sets behind the hills, casting a golden glow across the serene landscape; meanwhile, in the bustling city, cars honk and people rush through the crowded streets, oblivious to the tranquil scene unfolding just a few miles away, where birds chirp melodiously and a gentle breeze rustles the leaves of ancient trees, creating a harmonious symphony that contrasts sharply with the cacophony of urban life, reminding us that peace and chaos coexist in the world, each with its unique beauty and rhythm, as night falls and stars begin to twinkle in the vast sky above.";
    std::string targetText = "The quick brown fox jumps over the lazy dog near the riverbank while the sun sets behind the hills, casting";
    std::clog << "runTypingTest\n";
    // Запускаем тест на набор текста
    runTypingTest(targetText);

    // Завершаем ncurses
    endwin();
    std::clog.rdbuf(oldClogBuffer);
    return 0;
}
