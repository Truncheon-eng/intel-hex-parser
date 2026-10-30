# Синтаксический анализатор тестовых векторов

Программа на C++ для чтения файлов Intel HEX и извлечения входных и выходных тестовых векторов. Анализатор проверяет записи HEX, разбирает заголовок тестового вектора и выводит сведения о загруженных данных.

## Repository structure

<details>
<summary>Показать структуру репозитория</summary>

```text
syntax_analyzer_of_test_vectors/
|-- include/
|   |-- Definitions.hpp
|   |-- HexDumper.h
|   |-- InOutData8.hpp
|   |-- InOutData112.hpp
|   |-- InOutData128.hpp
|   |-- InOutData320.hpp
|   |-- InOutFlagStruct.hpp
|   |-- InOutFramerTXDataStrucHeader.hpp
|   |-- InOutFramerTXDataStruct.hpp
|   |-- TestVectorLoader.hpp
|   |-- VectorTransformer.hpp
|   |-- hex_loader.h
|   |-- hex_parser_logger.h
|   `-- intelhexclass.h
|-- src/
|   |-- HexDumper.cpp
|   |-- InOutData8.cpp
|   |-- InOutData112.cpp
|   |-- InOutData128.cpp
|   |-- InOutData320.cpp
|   |-- TestVectorLoader.cpp
|   |-- hex_loader.cpp
|   |-- hex_parser_logger.cpp
|   |-- intelhexclass.cpp
|   `-- main.cpp
|-- tools/
|   |-- acceptance_real_vector.sh
|   |-- format.sh
|   `-- gen_test_vector.py
|-- test_vectors/
|   |-- test_canonical_4vec.hex
|   |-- test_canonical_1000vec.hex
|   |-- test_patterns.hex
|   |-- test_gaps.hex
|   |-- test_extended.hex
|   `-- ...
|-- reference_vec/
|   `-- FRAMER_PCS_TX.hex           
|-- build/ 
|-- .clang-format
|-- .gitignore
|-- automatic_test_runner.sh
|-- Makefile
|-- README.md
```

В `include/` находятся интерфейсы и структуры данных, в `src/` их реализации. Каталог `tools/` содержит генератор векторов, тест и скрипт-форматирования. В `test_vectors/` приведены тестовые HEX-файлы; в дереве показана только часть набора.

</details>

## Setup

Команды выполняются из корня репозитория. Перед запуском тестовых скриптов используйте:

```shell
make
```

Исполняемый файл будет создан в `build/hex_test`.

```shell
# Набор тестовых векторов (негативные + канонические позитивные)
./automatic_test_runner.sh

# Генерация тестовых векторов (формат эталонного файла)
python3 tools/gen_test_vector.py --count 4 --seed 42 --out test_vectors/my_vec.hex

# Тест на реальном эталонном векторе (нужен reference_vec/FRAMER_PCS_TX.hex,
# в git не входит; при отсутствии файла тест пропускается)
./tools/acceptance_real_vector.sh 2000
```

Эталонный файл `reference_vec/FRAMER_PCS_TX.hex` добавляется отдельно. Приёмочный скрипт использует первые 2000 строк файла при указанном выше аргументе.

## Architecture

Обработка выполняется последовательно: файл Intel HEX, заголовок теста, пары входных и выходных векторов.

1. **Точка входа.** [src/main.cpp](src/main.cpp) обрабатывает аргументы командной строки, начинает загрузку HEX-файла, проверку заголовка и чтение векторов. Затем выводит результаты и статистику.
2. **Разбор Intel HEX.** Класс `intelhex` в [include/intelhexclass.h](include/intelhexclass.h) и [src/intelhexclass.cpp](src/intelhexclass.cpp) декодирует текстовые записи, проверяет их формат и контрольную сумму. Обрабатывает типы записей `00`-`05`: данные, конец файла, расширенные сегментные и линейные адреса, стартовые адреса. Байты данных хранятся в `std::map<unsigned long, unsigned char>`, где ключом служит адрес.
3. **Доступ к данным.** `HexLoader` наследует `intelhex` и предоставляет загрузку файла, чтение байтов, блоков и чисел с порядком байтов little-endian или big-endian. Интерфейс находится в [include/hex_loader.h](include/hex_loader.h), реализация в [src/hex_loader.cpp](src/hex_loader.cpp). Отсутствующие байты при чтении по умолчанию заменяются на `0xFF`.
4. **Разбор тестовых векторов.** `TestVectorLoader` читает заголовок с адреса `0x0C000000`, проверяет сигнатуру, идентификатор модуля, размеры и описание сигналов. Поля заголовка читаются в big-endian. По размерам данных вычисляется число векторов, затем из входной и выходной областей читаются пары блоков в `TestVector`. См. [include/TestVectorLoader.hpp](include/TestVectorLoader.hpp), [src/TestVectorLoader.cpp](src/TestVectorLoader.cpp) и [include/Definitions.hpp](include/Definitions.hpp).
5. **Диагностика.** [HexParserLogger](src/hex_parser_logger.cpp) собирает ошибки и предупреждения низкоуровневого парсера. [HexDumper](src/HexDumper.cpp) выводит статистику и помогает анализировать адреса, разрывы и содержимое данных.

## Requirements

| Компонент | Назначение |
| --- | --- |
| `g++` с поддержкой C++17 и соответствующая `libstdc++` | Компиляция C++-исходников; в Makefile задан `-std=c++17`, в коде используется `std::transform_reduce` |
| GNU Make | Сборка через [Makefile](Makefile) |
| Bash | Запуск тестовых и служебных скриптов |
| Python 3 | Генерация тестовых векторов; используются только модули стандартной библиотеки |

Для установки основных зависимостей на примере Debian:

```shell
sudo apt update
sudo apt install build-essential python3
```

Версия `g++` из `build-essential` зависит от выпуска Debian. Проверить установленные инструменты можно командами:

```shell
g++ --version
make --version
python3 --version
```

Дополнительно: `gdb` необходим для `make debug`; `clang-format`, Perl и утилиты `find`, `sort`, `wc` нужны для `make format`. Для обычной сборки эти инструменты не требуются.

## Examples of running

Запуск `automatic_test_runner.sh` (пример вывода на основе теста `test_canonical_4vec.hex`):
```shell
Testing: test_canonical_4vec.hex
----------------------------------------
=== HEX FILE ANALYZER ===
File: test_vectors/test_canonical_4vec.hex
========================================
Start Load file test_vectors/test_canonical_4vec.hex

=== Test Vector Analysis ===
[INFO] === START PARSING HEADER ===
Signature at 0xc000000: 0x54455354
Module ID at 0xc000004: 0x46525458
Total data length at 0xc000008: 0xe8
Input base address at 0xc00000c: 0x1000000
Input vector size at 0xc000010: 0x2a
Output base address at 0xc000014: 0x0
Output vector size at 0xc000018: 0x10
Reserved at 0xc00001c: 0xc0fef00d
[INFO] === VALIDATING BASIC HEADER ===
[+] Subsystem signature recognized: TEST
[+] Module ID recognized: FRTX
[i] Test configuration: TEST -> FRTX
[+] Input vector size: 42 bytes
[+] Output vector size: 16 bytes
[+] Estimated vectors: 4 (232 bytes total)
[!] Zero output base address
[+] === BASIC HEADER VALIDATION PASSED ===
[INFO] === TEST CONFIGURATION SUMMARY ===
[INFO] Subsystem:      TEST
[INFO] Module:         FRTX
[INFO] Input size:     42 bytes per vector
[INFO] Output size:    16 bytes per vector
[INFO] Total data:     232 bytes
[INFO] Estimated:      4 test vectors
[INFO] ==================================
=== VALIDATING BASIC SIGNAL PART ===
[i] Input signals: 3
[i] Output signals: 3
DEBUG: Input width[0] = 320
DEBUG: Input width[1] = 8
DEBUG: Input width[2] = 4
DEBUG: Output width[0] = 111
DEBUG: Output width[1] = 8
DEBUG: Output width[2] = 5
[+] Signal data validation passed

--- FINAL VALIDATION ---

=== INPUT SIGNALS ===
Index | Bits | Bytes | Offset (hex) | Offset (dec) | Range
------|------|-------|--------------|--------------|----------------
    0 |  320 |    40 | 0x0000000000 |            0 | [0:39]
    1 |    8 |     1 | 0x0000000028 |           40 | [40:40]
    2 |    4 |     1 | 0x0000000029 |           41 | [41:41] (padded)

=== OUTPUT SIGNALS ===
Index | Bits | Bytes | Offset (hex) | Offset (dec) | Range
------|------|-------|--------------|--------------|----------------
    0 |  111 |    14 | 0x0000000000 |            0 | [0:13]
    1 |    8 |     1 | 0x000000000e |           14 | [14:14]
    2 |    5 |     1 | 0x000000000f |           15 | [15:15] (padded)

=== SUMMARY ===
Total input size:  42 bytes (0x2a)
Total output size: 16 bytes (0x10)

=== ALIGNED ADDRESSES (for 32-bit access) ===
Input region:
  Signal[2]: offset 0x29 -> aligned 0x2c (+3 bytes padding)

=== VALIDATION ===
Expected input size:  42 bytes
Calculated input size: 42 bytes
Expected output size: 16 bytes
Calculated output size: 16 bytes
[+]  Final validation passed
DEBUG: Vector calculation:
  Total data: 232 bytes
  Input vector: 42 bytes
  Output vector: 16 bytes
  Vector pair: 58 bytes
  Total vectors: 4
[INFO] Calculated total vectors: 4
[INFO] === HEADER PARSING COMPLETED SUCCESSFULLY ===
Total test vectors: 4
[i] Start read ALL vectors!
[INFO] Reading all 4 vectors
[INFO] Reading vectors 0 to 3 (4 vectors)
[INFO] Read 4 vectors
[+] All Vector SUCCESSFUL read!

=== LOADING STATISTICS ===
Total vectors loaded: 4
Input vector size:  42 bytes
Output vector size: 16 bytes
Total input data:   168 bytes
Total output data:  64 bytes
Vectors with input: 4
Vectors with output:4

=== FIRST 5 INPUT VECTORS ===

Vector 0 (input, 42 bytes):
  0000: 57 7d 53 ec c2 8a 70 a6 1c 75 10 a1 cd 89 21 6c
  0010: a1 6c ff ca ea 49 87 47 7e 86 db cc b9 70 46 fc
  0020: 2e 18 38 4e 51 d8 20 c5 c3 00
  Last 2 bytes: 0xc300

Vector 1 (input, 42 bytes):
  0000: ef 80 05 3a 88 ae 39 96 de 50 e8 01 86 5b 36 98
  0010: 65 4e bf 52 00 a5 fa 09 39 b9 9d 7a 1d 7b 28 2b
  0020: f8 23 40 41 f3 54 87 d8 6c c0
  Last 2 bytes: 0x6cc0

Vector 2 (input, 42 bytes):
  0000: 66 9f cc bf e0 e7 3d 7e 73 20 ad 0a 75 70 03 24
  0010: 1e 75 22 10 a9 24 79 8e f8 6d 43 f2 7c f2 d0 61
  0020: 30 31 dc b5 d8 d2 ef 1b 32 c0
  Last 2 bytes: 0x32c0

Vector 3 (input, 42 bytes):
  0000: 1f ce ad 37 7f 62 61 e5 47 d8 5d 8e ec 7f 26 e2
  0010: 32 19 07 2f 79 55 d0 f8 f6 6d cd 1e 54 c2 01 c7
  0020: 87 e8 92 d8 f9 4f 61 97 6f c0
  Last 2 bytes: 0x6fc0

=== FIRST 5 OUTPUT VECTORS ===

Vector 0 (output, 16 bytes):
  0000: 39 0c 8c 7d 72 47 34 2c d8 10 0f 2f 6f 77 0d 18

Vector 1 (output, 16 bytes):
  0000: 65 d6 70 e5 8e 03 51 d8 ae 8e 4f 6e ac 34 2f 40

Vector 2 (output, 16 bytes):
  0000: c2 31 b7 b0 87 16 eb 3f c1 28 96 b9 62 23 17 68

Vector 3 (output, 16 bytes):
  0000: 74 94 28 77 33 c2 8e e8 ba 53 bd b5 6b 88 24 90

[+] Vector loading completed successfully!

=== Loaded Data Information ===
=== HEX Loader Information ===
Total data bytes: 278
Default endianness: little-endian
Checksum verification: enabled
Verbose mode: disabled
Address range: 0x0 - 0xc00002d
=============================

=== Data Analysis ===
=== HEX File Statistics ===
=== HEX Loader Information ===
Total data bytes: 278
Default endianness: little-endian
Checksum verification: enabled
Verbose mode: disabled
Address range: 0x0 - 0xc00002d
=============================
Successfully retrieved 278 addresses from HexLoader.
Min address: 0x0
Max address: 0xc00002d
Total data bytes: 278
Address range: 0x0 - 0xc00002d (201326638 bytes)
Data density: 0.0%

=== DATA GAPS ANALYSIS ===
Total addresses: 278
Address range: 0x0 - 0xc00002d (201326638 bytes)

Found 2 gaps:
Total gap bytes: 201326360 (100.0% of address range)

Top 2 gaps (largest first):
  Gap #1: 0x10000a8 - 0xbffffff (184549208 bytes)
  Gap #2: 0x40 - 0xffffff (16777152 bytes)

Memory visualization:
Memory map visualization (0x0 - 0xc00002d):
█░░░██░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░█

Legend:
█ - Data present   ░ - Gap (no data)

Zoom on largest gap (0x10000a8 - 0xbffffff):
Memory map visualization (0xffffe0 - 0xc0000c7):
█░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░░░░░░ ░░░█

Legend:
█ - Data present   ░ - Gap (no data)

=== Memory Regions Analysis ===
Found 3 memory regions:
  Region 0: 0x0 - 0x3f (64 bytes)
  Region 1: 0x1000000 - 0x10000a7 (168 bytes)
  Region 2: 0xc000000 - 0xc00002d (46 bytes)

========================================
ANALYSIS COMPLETE

✓ No parser errors or warnings
========================================
```

Генерация тестовых векторов с помощью `gen_test_vector.py`:
```shell
:020000040C00EE
:200000005445535446525458000000E8010000000000002A0000000000000010C0FEF00D7E
:0E0020000303014000080004006F0008000503
:020000040000FA
:20000000390C8C7D7247342CD8100F2F6F770D1865D670E58E0351D8AE8E4F6EAC342F40B6
:20002000C231B7B08716EB3FC12896B9622317687494287733C28EE8BA53BDB56B8824902B
:020000040100F9
:20000000577D53ECC28A70A61C7510A1CD89216CA16CFFCAEA4987477E86DBCCB97046FC59
:200020002E18384E51D820C5C300EF80053A88AE3996DE50E801865B3698654EBF5200A541
:20004000FA0939B99D7A1D7B282BF8234041F35487D86CC0669FCCBFE0E73D7E7320AD0ADF
:20006000757003241E752210A924798EF86D43F27CF2D0613031DCB5D8D2EF1B32C01FCE1D
:20008000AD377F6261E547D85D8EEC7F26E23219072F7955D0F8F66DCD1E54C201C787E826
:0800A00092D8F94F61976FC07F
:00000001FF
```

Тестирование `intel-hex` парсера на эталанном векторе:
```shell
> ./tools/acceptance_real_vector.sh 2000
ACCEPTANCE PASSED (first 34 lines of the real FRAMER_PCS_TX.hex)
```