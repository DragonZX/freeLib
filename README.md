# freeLib
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![build on Ubuntu 20.04](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2020.04.yml/badge.svg?branch=master)](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2020.04.yml)
[![build on Ubuntu 22.04](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2022.04.yml/badge.svg?branch=master)](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2022.04.yml)
[![build on Ubuntu 24.04](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2024.04.yml/badge.svg?branch=master)](https://github.com/petrovvlad/freeLib/actions/workflows/build%20on%20Ubuntu%2024.04.yml)

freeLib - каталогизатор для библиотек LibRusEc и Flibusta

Это форк общедоступного freeLib 5.0 , разработка которого прекращена. 
![screenshot](./doc/screenshot.png#gh-light-mode-only)
![screenshot](./doc/screenshot-dark.png#gh-dark-mode-only)
* Создание собственных библиотек на основе файлов FB2(.ZIP), EPUB, FBD.
* Конвертация в форматы AZW3 (KF8), MOBI, MOBI7 (KF7), EPUB.
* Работа с несколькими библиотеками.
* Импорт библиотек из inpx-файлов.
* Поиск и фильтрация книг.
* Серверы OPDS и HTTP.
* Сохранение книг в выбранную папку.
* Различные настройки экспорта для нескольких устройств.
* Отправка выбранных файлов книг на email (Send to Kindle).
* Установка тегов для книги, автора, серии и фильтрация по тегам.
* Настройка форматирования книг (шрифты, буквица, заголовки, переносы, сноски)
* Чтение книг с помощью внешних приложений. Можно назначить отдельную программу для каждого формата.

#### Сборка и установка из исходников в Ubuntu

Установить необходимые компоненты: 
в Ubuntu ≥ 23.04
```
sudo apt update && sudo apt install git cmake build-essential qt6-base-dev libqt6core5compat6-dev qt6-httpserver-dev libqt6websockets6-dev libqt6svg6 libquazip1-qt6-dev
```
в Ubuntu < 23.04
```
sudo apt update && sudo apt install git cmake build-essential qtbase5-dev libqt5sql5-sqlite libquazip5-dev
```
Для отображения обложек djvu файлов установить библиотеку [DjVuLibre](https://djvu.sourceforge.net/)
```
sudo apt install libdjvulibre-dev
```
Скачать исходники программы:
```
git clone --recurse-submodules https://github.com/petrovvlad/freeLib.git
```
Собрать и установить:
```
mkdir freeLib/build && cd freeLib/build && \
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr .. && cmake --build . -j2 && \
sudo make install
```

### Установка в Arch Linux
```
yay -S freelib
```

Для конвертации книг в AZW3, MOBI необходимо установить **kindlegen**.

#### Обсуждение
канал Matrix:  [#freeLib:matrix.p-vlad.ru](https://matrix.to/#/#freeLib:matrix.p-vlad.ru)
