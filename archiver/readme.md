# Архиватор

В данном задании вам предстоит реализовать архиватор на основе [алгоритма Хаффмана](https://en.wikipedia.org/wiki/Huffman_coding).

Программа-архиватор должна иметь следующий интерфейс командной строки:
* `archiver -c archive_name file1 [file2 ...]` - заархивировать файлы `file1, file2, ...` и сохранить результат в файл `archive_name`.
* `archiver -d archive_name` - разархивировать файлы из архива `archive_name` и положить в текущую директорию.
* `archiver -h` - вывести справку по использованию программы.

Имена файлов (только имена файлов с расширениями, без дополнительного пути) должны сохраняться при архивации и разархивации.

## Алгоритм

Алгоритм сжатия  устроен следующим образом:
1. Подсчитывается частотность 8-битных символов в файле. Кроме содержимого файла надо учесть частоты символов в имени файла, а также добавить три служебных символа `FILENAME_END=256`, `ONE_MORE_FILE=257`, `ARCHIVE_END=258` с частотами 1. Назначение этих символов будет описано позже. Таким образом, для кодирования расширенного алфавита необходимо 9 бит.
1. Строится бинарный [бор](https://en.wikipedia.org/wiki/Trie) кодирования следующей процедурой:
    1. Для каждого символа алфавита добавляется соответствующая вершина в очередь с приоритетом. Упорядочение вершин в очереди осуществляется по неубыванию частот символов в файле (в "начале" очереди всегда вершина с символом с наименьшей встречаемостью в файле), а при равенстве частот - по возрастанию символов (для вершин, не являющихся листьями, в качестве сивола для сравнения используется наименьший из символов их потомков).
    1. Пока в очереди больше одного элемента, из нее последовательно извлекаются две вершины A и B с минимальными приоритетами. Создается новая вершина С, детьми которой являются вершины A и B.
       Вершина C помещается в очередь с приоритетом, равным сумме приоритетов вершин A и B.
    1. По окончанию процедуры в очереди остается ровно одна вершина, которая является корнем построенного бора. Листовые вершины являются терминальными.
       В каждой терминальной вершине записан символ из исходного файла.
       Каждая нетерминальная вершина дерева содержит два ребра: левое и правое, которые помечаются битами 0 и 1 соответственно.
       Каждой терминальной вершине соответствует битовая последовательность, получающаяся спуском из корня бора к терминальной вершине и выписыванием битов всех пройденных ребер.
       Для наглядности можно воспользоваться следующими иллюстрациями:
    * [gif demo](https://commons.wikimedia.org/wiki/File:Huffmantree_ru_animated.gif?uselang=ru)
    * [gif demo](https://commons.wikimedia.org/wiki/File:Huffman_huff_demo.gif)
    * [graphic demo](https://people.ok.ubc.ca/ylucet/DS/Huffman.html).
1. Всем символам ставится в соответствие бинарная кодовая последовательность посредством построенного бора.
1. Код приводится к [канонической форме](https://en.wikipedia.org/wiki/Canonical_Huffman_code). Каноническая форма кода отличается тем, что позволяет однозначно восстановить коды по списку символов и длинам кодов для них. Алгоритм восстановления канонического кода есть [в википедии](https://en.wikipedia.org/wiki/Canonical_Huffman_code).
1. Все символы файла заменяются на соответствующие кодовые бинарные последовательности, и результ записывается вместе со вспомогательной информацией в файл. Формат файла архива описан ниже.

Алгоритм декодирования в целом обратен алгоритму кодирования и устроен следующим образом:
1. Из файла восстанавливается таблица кодирования (соответствие между сиволами и их кодами).
1. По таблице кодирования строится бинарный бор.
1. По бинарным последовательностям, прочитанным из входного файла, производится трассировка по бору от корня к листовым вершинам. При достижении очередной листовой вершины бора определяется соответсвующий ей символ, который записывается в выходной файл.

## Формат файла
Все значения записываются в формате "от младшего к старшему" (аналог little-endian для битов). Т.е. сначала идет бит, соответствующий `2^0`, затем - `2^1` и т.д. до бита, соответствующего `2^8`.
Внутри `unsigned char` биты считаем от младшего к старшему.

Пример: дана последовательность `unsigned char` `{1, 3, 7, }`. Требуется прочитать из неё два 9-битных значения `a` и `b`.
Представление на битовом уровне:
```
10000000 11000000 11100000
aaaaaaaa abbbbbbb bb
```
Следовательно, `a = 257`, `b = 385`.


Файл с архивом должен иметь следующий формат:
1. 9 бит - количество символов в алфавите `SYMBOLS_COUNT`
1. Блок данных для восстановления канонического кода
   1. `SYMBOLS_COUNT` значений по 9 бит - символы алфавита в порядке следования канонических кодов
   1. Список из `MAX_SYMBOL_CODE_SIZE` значений по 9 бит, i-й (при нумерации с 0) элемент которого - это количество символов с длиной кода `i+1`. `MAX_SYMBOL_CODE_SIZE` - максимальная длина кода в текущем кодировании. Канонические коды соответствуют по порядку символам из предыдущего пункта. `MAX_SYMBOL_CODE_SIZE` не записывается явным образом в файл, т.к. его можно восстановить по имеющимся данным.
1. Закодированное имя файла
1. Закодированный служебный символ `FILENAME_END`
1. Закодированное содержимое файла
1. Если в архиве есть ещё фалы, то закодированный служебный символ `ONE_MORE_FILE` и кодировка продолжается с п.1.
1. Закодированный служебный символ `ARCHIVE_END`.

## Реализация
Старайтесь делать все компоненты программы по возможности более универсальными и не привязанными к специфике конкретной задачи.
Например, алгоритмы кодирования и декодирования должны работать с потоками ввода-вывода, а не файлами.

Программа должна корректно обрабатывать очень большие (во много раз превосходящие по объему оперативную память) файлы. Это означает, что при работе программы данные не должны накапливаться в памяти.

Файлы с форматом, не соответствующим спецификации, не должны приводить к "падению" программы. Все исключительные ситуации должны корректно обрабатываться с выводом понятного пользователю сообщения об ошибке.
При любых ошибках программа должна возвращать код `111`, при успешном завершении - код `0`. 

Скорее всего, вам понадобятся следующие компоненты:
- Обертки над потоками ввода-вывода, позволяющие читать и писать значения по 1 и 9 бит.
- Бор с интерфейсом, позволяющим удобно его обходить.
- Очередь с приоритетами.

Подробный дизайн программы рекомендуется обсудить с преподавателем на семинарах.

## Сдача

Проект проверяется преподавателями. Все сдачи будут проходить код ревью. Автомерж роботом для проекта не применяется. 

Код проекта должен лежать в папке [src](src). Для сдачи проекта используйте ветку `submits/archiver`

## Оценивание

Минимальная реализация проекта должна быть полностью функциональной (т.е. корректно архивировать и разархивировать файлы) - **она оценивается в 6 баллов**.

Решения, не удовлетворяющие требованиям к минимальной реализации, оцениваются в 0 баллов.

Дополнительные баллы (максимум **1 балл за каждый пункт**) можно получить:
- Обобщенную (не завязанную на специфику конкретной задачи) реализацию очереди с приоритетами без использования стандартных очередей, алгоритмов и коллекций, обеспечивающих сортированность. Реализация должна быть алгоритмически эффективной, т.е. иметь сложность вставки и извлечения не более O(logN), где N - число элементов в очереди.
- Обобщенную реализацию класса для работы с аргументами командной строки (без использования готовых аналогов) 
- Обобщенную реализацию потоков побитового ввода и вывода
- Покрытие юнит тестами всех критичных компонентов. Ваш семинарист поможет с определением достаточности покрытия.

Баллы могут быть сняты за серьезные недочеты, например:
- Отсутствие или неверную декомпозицию на файлы, классы, методы и функции
- Утечки памяти
- Неконтролируемое падение программы
- Нечитаемый код
- Нарушение стиля кодирования
- UB на каком-либо входе или наборе аргументов функции
- Выбор неэффективных структур данных или их неэффективное использование
- Неоправданные копирования объектов
- Неверные сигнатуры функций и методов

## Дедлайн

Для проекта установлен **жесткий** дедлайн `23:59 23.10.2022` - это означает, что решения,
отправленные после этого времени, не будут оцениваться.

Дополнительных штрафов за время сдачи нет.