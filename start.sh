#!/bin/bash

# Генерация файлов сборки
cmake .

# Выполнение сборки
if make; then
    # Если make завершилось успешно, запускаем исполняемый файл
    ./main text.txt
else
    # Если make завершилось с ошибкой, выводим сообщение и не запускаем ./main
    echo "Build failed, not running ./main"
    exit 1
fi
