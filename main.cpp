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
// TODO: Add Ctrl+Backspace handler
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

// Класс для измерения времени
class Timer {
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    bool is_running = false;
public:
    void start() {
        start_time = std::chrono::steady_clock::now();
        is_running = true;
    }

    void stop() {
        end_time = std::chrono::steady_clock::now();
        is_running = false;
    }

    bool working() {
        return is_running;
    }

    double get_elapsed_time() const {
        std::chrono::duration<double> elapsed = end_time - start_time;
        return elapsed.count();
    }
};

void runTypingTest(const std::string& target_text) {
    enum symbolType {
        COMMON = 1, CORRECT, INCORRECT 
    };   
    // Очищаем экран перед началом теста
    clear();

    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax); // Получаем размеры экрана
    int height, width = xMax * 0.8;
    std::clog << "width = " << width << '\n';
    std::vector <std::string> text = separate(target_text, width);
    if (text.empty()) {
        return;
    }
    height = text.size();
    int number_symbols = 0;    
    for (int i = 0; i < height; ++i) {
        number_symbols += text[i].size();
    }

    int startY = (yMax - height) / 2;
    int startX = (xMax - width) / 2;
    std::clog << "startY = " << startY << ", startX = " << startX << '\n';

    // Задаем цветовые пары
    init_pair(COMMON, COLOR_WHITE, COLOR_BLACK);       // Обычный белый текст
    init_pair(CORRECT, COLOR_GREEN, COLOR_BLACK);       // Правильный зелёный текст
    init_pair(INCORRECT, COLOR_RED, COLOR_BLACK);         // Неправильный красный текст

    int current_col = startX;
    int current_row = startY;

    // Выводим текст для набора
    attron(COLOR_PAIR(COMMON));
    for (int i = 0; i < height; ++i) {
        mvprintw(current_row + i, startX, text[i].c_str());
    }
    attroff(COLOR_PAIR(COMMON));
    refresh();

    int row = 0;
    int col = 0; // Индекс текущего символа ввода
    int errors = 0; // Счетчик ошибок
    assert(text.size() > 0);
    int row_wrong_symbol = text.size();
    assert(text[0].size() > 0);
    int col_wrong_symbol = text.back().size();


    // Основной цикл ввода
    Timer timer;
    while (row < height) {
        move(startY + row, startX);
        col = 0;
        int line_size = text[row].size();
        std::clog << "row = " << row << ", text[row] = \"" << text[row] << "\"\n";
        while (col < line_size) {
            // Подчёркивание символа, который нужно ввести
            attron(A_UNDERLINE);
            addch(text[row][col]);
            move(startY + row, startX + col);
            attroff(A_UNDERLINE);
            int ch = getch(); // Получаем нажатие клавиши
            if (!timer.working()) {
                timer.start();
            }

            std::clog << "Read symbol (int)\'" << ch << "\', (char)\'" << (char)ch << '\'' << '\n';  

            if (!check(ch)) {
                continue;
            }
            std::clog << "Need symbol (int)\'" << static_cast<int>(text[row][col]) << "\', (char)\'" << static_cast<char>(text[row][col]) << '\'' << '\n';
            std::clog << "col = " << col << ", text[" << row << "][" << col << "] = \'" << text[row][col] << '\'' << '\n';

            if (ch == KEY_BACKSPACE) {
                std::clog << "Backspace handling\n";
                if (col == 0 && row == 0) continue;
                // Обработка клавиши Backspace для удаления последнего символа
                attron(COLOR_PAIR(COMMON));
                addch(text[row][col]);
                attroff(COLOR_PAIR(COMMON));
                if (col == 0) {
                    --row;
                    line_size = text[row].size();
                    col = text[row].size() - 1;
                } else {
                    --col;
                }
                attron(COLOR_PAIR(COMMON));
                mvaddch(startY + row, startX + col, text[row][col]); // Очищаем символ на экране
                attroff(COLOR_PAIR(COMMON));
                move(startY + row, startX + col); // Возвращаемся на предыдущую позицию
            } else if (row_wrong_symbol <= row && col_wrong_symbol < col) {
                std::clog << "There is an uncorrected error\n";
                attron(COLOR_PAIR(INCORRECT));
                addch(text[row][col]);
                attroff(COLOR_PAIR(INCORRECT));
                ++col;
            } else if (ch == text[row][col]) {
                std::clog << "Correct symbol handling\n";
                // Выводим правильно набранный символ зелёным цветом
                row_wrong_symbol = height;
                col_wrong_symbol = text.back().size();
                attron(COLOR_PAIR(CORRECT));
                addch(ch);
                attroff(COLOR_PAIR(CORRECT));
                ++col;
            } else if (ch != ERR) {
                std::clog << "Wrong symbol handling\n";
                // Выводим неправильно набранный символ красным текстом
                if (row_wrong_symbol == height && col_wrong_symbol == text.back().size()) {
                    row_wrong_symbol = row;
                    col_wrong_symbol = col;
                    ++errors; // Увеличиваем счетчик ошибок
                }
                attron(COLOR_PAIR(INCORRECT));
                addch(text[row][col]);
                attroff(COLOR_PAIR(INCORRECT));
                ++col;
            } else {
                assert(true);
            }
            
            std::clog << std::endl;
            refresh();
        }
        ++row;
    }
    timer.stop();

    double typingSpeed = (static_cast<double>(number_symbols) / timer.get_elapsed_time()) * 60;
    double percentageOfErrors = (static_cast<double>(errors) / number_symbols) * 100;
    // TODO: Change printing results
    // Выводим результаты
    mvprintw(startY + height + 2, 0, "Time taken: %.2f seconds", timer.get_elapsed_time());
    mvprintw(startY + height + 3, 0, "Typing speed: %.2f characters per minute", typingSpeed);
    mvprintw(startY + height + 4, 0, "Errors: %d, %.2f%%", errors, percentageOfErrors);
    mvprintw(startY + height + 5, 0, "Press ENTER to try again, or any other key to exit...");
    refresh();

    // Ждем нажатия клавиши
    int key = getch();
    if (key == '\n') {
        // Пользователь решает повторить тест
        runTypingTest(target_text);
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
    // std::string target_text = "The quick brown fox jumps over the lazy dog near the riverbank while the sun sets behind the hills, casting a golden glow across the serene landscape; meanwhile, in the bustling city, cars honk and people rush through the crowded streets, oblivious to the tranquil scene unfolding just a few miles away, where birds chirp melodiously and a gentle breeze rustles the leaves of ancient trees, creating a harmonious symphony that contrasts sharply with the cacophony of urban life, reminding us that peace and chaos coexist in the world, each with its unique beauty and rhythm, as night falls and stars begin to twinkle in the vast sky above.";
    std::string target_text = "The quick brown fox jumps over the lazy dog near the riverbank while the sun sets behind the hills, casting";
    std::clog << "runTypingTest\n";
    // Запускаем тест на набор текста
    runTypingTest(target_text);

    // Завершаем ncurses
    endwin();
    std::clog.rdbuf(oldClogBuffer);
    return 0;
}
