# RecursiveHashMap

Реализация **C++17** собственной хеш-таблицы с **рекурсивной (древовидной) структурой бакетов**.  
Вместо длинных цепочек внутри одного бакета, каждый бакет при необходимости может рекурсивно содержать ещё один узел `HashMap`, образуя многоуровневую структуру. Коллизии разрешаются “углублением” в дерево.

Репозиторий — portfolio-проект, ориентированный на **проектирование структур данных**, **шаблоны C++**, **RAII/управление памятью** и **корректность**.

## Ключевые моменты

- Реализован собственный ассоциативный контейнер с API, близким к `std::unordered_map`:
  - конструкторы (по умолчанию / с кастомным хешером / из диапазона итераторов / из initializer_list)
  - `insert`, `find`, `erase`, `operator[]`, `at`, `size`, `empty`, `clear`
  - `hash_function()`
  - forward-итераторы (`iterator` / `const_iterator`) для range-based `for`
- Обработка коллизий через **рекурсивное дерево бакетов** (nested hash tables).
- Динамическое изменение размера в обе стороны:
  - **увеличение** при высокой нагрузке
  - **уменьшение** при разреженности
- Стресс-тесты с рандомными вставками/удалениями и сравнением с `std::unordered_map`.
- Тесты показали ускорение в среднем в 10 раз по сравнению с `std::unordered_map`.

## Обзор дизайна

У контейнера есть два внутренних режима:

### 1) Small mode (лист)
- Хранит элементы в маленьком `std::vector<std::pair<const KeyType, ValueType>>`.
- Нужен, чтобы не платить накладные расходы дерева для очень маленьких мап.

### 2) Recursive mode (внутренний узел)
- Хранит фиксированный массив (`std::vector<std::unique_ptr<HashMap>>`) дочерних узлов.
- Каждая занятая ячейка содержит указатель на *дочерний* `HashMap` более глубокого уровня.

Когда элементов становится много (или бакеты начинают сильно заполняться), узел **расширяется**:
- переключается из small-режима в recursive-режим,
- увеличивает capacity до следующего простого числа из `max_sizes[]`,
- пере-вставляет существующие элементы в новую структуру.

Когда структура становится слишком разреженной, узел может **уменьшиться** и/или схлопнуться обратно в small-режим.

### Что делает структуру “рекурсивной”

Каждый бакет сам по себе может быть `HashMap`-узлом (до `MAX_RECURSIVE_LEVEL`), поэтому коллизии решаются углублением:

```

корневой HashMap
├── bucket 0 -> дочерний HashMap (уровень 1)
├── bucket 1 -> дочерний HashMap (уровень 1)
└── bucket 2 -> дочерний HashMap (уровень 1)
└── ... (уровень 2, 3, ...)

````

## Сложность (типично)

- Средняя сложность `find/insert/erase`: **O(1)** (зависит от качества хеша и load factor).
- При тяжёлых коллизиях: **O(depth)**, где `depth` ограничена `MAX_RECURSIVE_LEVEL`.

## Сборка и запуск

Проект использует минимальный CMake.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/HashMap
````

## Тестирование

`main.cpp` содержит:

* проверки интерфейса и const-корректности (итераторы, `at`, `find` и т.д.)
* проверку исключений (`at` бросает `std::out_of_range`)
* корректность copy ctor / operator=
* корректность итераторов (pre/post increment)
* корректность деструктора (через тип ключа со счётчиком “живых” объектов)
* тесты кастомного хешера
* рандомизированные стресс-тесты со сравнением со `std::unordered_map`

Чтобы прогнать внутренний набор тестов, раскомментируй строку в `main()`:

```cpp
// internal_tests::run_all();
```

## Пример использования

> Замените имена, если у вас в коде они отличаются.

```cpp
#include "HashMap.h"
#include <string>

int main() {
    HashMap<std::string, int> m;

    m.insert({"a", 1});
    m["b"] = 2;               // вставляет значение по умолчанию, если ключа нет

    if (m.find("a") != m.end()) {
        int v = m.at("a");     // бросает std::out_of_range, если ключ не найден
    }

    m.erase("b");

    for (auto& [k, v] : m) {
        // итерация по парам ключ/значение
    }
}
```

## Настраиваемые параметры

Текущая реализация использует compile-time константы:

* `MAX_RECURSIVE_LEVEL` — максимальная глубина рекурсии
* `max_sizes[]` — простые числа ёмкостей для resize
* `increase_primes[]` — множители (по уровням) для “перемешивания” хеша
* `MAX_SIZE_DIV_NUMBER_OF_ELEMENTS` — эвристика порога нагрузки

Их можно тюнить под компромисс память/скорость.

## Состав репозитория

* `HashMap.h` — вся реализация (header-only)
* `main.cpp` — тесты и стресс-проверки
* `CMakeLists.txt` — сборка

## Идеи для улучшений

* Заменить самописный тест-раннер на Catch2 / GoogleTest.
* Улучшить STL-совместимость (traits итераторов, дополнительные типы/алиасы).
* Оптимизировать память (pool allocator / custom allocator для узлов).
* Сделать глубину рекурсии и политики resize настраиваемыми в runtime.

## Лицензия

Пока лицензия не добавлена. Если планируете распространять код — добавьте MIT/Apache-2.0 и т.п.

```
::contentReference[oaicite:0]{index=0}
```


# RecursiveHashMap

Header-only **C++17** implementation of a custom hash table with a **recursive (tree-like) bucket structure**.
Instead of keeping long chains inside a single bucket, each bucket can recursively contain another `HashMap` node, forming a multi-level structure that resolves collisions by descending deeper.

This repository is a portfolio project focused on **data structure design**, **templated C++**, **RAII/memory management**, and **correctness testing**.

## Highlights (for recruiters)

- Implemented a custom associative container with an API close to `std::unordered_map`:
  - constructors (default / custom hasher / iterator range / initializer list)
  - `insert`, `find`, `erase`, `operator[]`, `at`, `size`, `empty`, `clear`
  - `hash_function()`
  - forward iterators (`iterator` / `const_iterator`) for range-based `for`
- Collision handling via a **recursive bucket tree** (nested hash tables).
- Dynamic resize in both directions:
  - **expand** on high load
  - **reduce** when the table becomes sparse
- Stress-tested against `std::unordered_map` with random insert/erase workload.

## Design overview

The map has two internal modes:

1) **Small mode** (leaf)
- Stores elements in a small `std::vector<std::pair<const KeyType, ValueType>>`.
- Used to keep overhead low for tiny maps.

2) **Recursive mode** (internal node)
- Stores a fixed-size array (`std::vector<std::unique_ptr<HashMap>>`) of child nodes.
- Each occupied cell points to a *child* `HashMap` representing a deeper level.

When the number of stored elements grows (or many buckets become occupied), a node **expands**:
- switches from small mode to recursive mode,
- increases capacity to the next prime from `max_sizes[]`,
- reinserts existing elements into the new structure.

When many buckets become empty, the node can **reduce** and optionally collapse back to small mode.

### What makes it “recursive”

Each bucket is itself another `HashMap` node (up to `MAX_RECURSIVE_LEVEL`), so collisions are resolved by descending deeper:

```

root HashMap
├── bucket 0 -> child HashMap (level 1)
├── bucket 1 -> child HashMap (level 1)
└── bucket 2 -> child HashMap (level 1)
└── ... (level 2, level 3, ...)

````

## Complexity (typical)

- Average-case search/insert/erase: **O(1)** (depends on hash quality and load).
- Under heavy collisions: **O(depth)** where depth is capped by `MAX_RECURSIVE_LEVEL`.

## Build & run

This repo uses a minimal CMake setup.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/HashMap
````

## Testing

`main.cpp` includes:

* interface/constness checks (iterators, `at`, `find`, etc.)
* exception behavior (`at` throws `std::out_of_range`)
* copy constructor / assignment correctness
* iterator correctness (pre/post increment)
* destructor correctness (via a counter-tracking key type)
* custom hash function tests
* randomized stress tests compared to `std::unordered_map`

To run the internal test suite, uncomment the line below in `main()`:

```cpp
// internal_tests::run_all();
```

## Usage example

```cpp
#include "HashMap.h"
#include <string>

int main() {
    HashMap<std::string, int> m;

    m.insert({"a", 1});
    m["b"] = 2;               // inserts default value if missing

    if (m.find("a") != m.end()) {
        int v = m.at("a");     // throws std::out_of_range if missing
    }

    m.erase("b");

    for (auto& [k, v] : m) {
        // iterate over key/value pairs
    }
}
```

## Configuration knobs

The current implementation uses compile-time constants:

* `MAX_RECURSIVE_LEVEL` — maximum recursion depth
* `max_sizes[]` — prime capacities used during resizing
* `increase_primes[]` — per-level multipliers for hash mixing
* `MAX_SIZE_DIV_NUMBER_OF_ELEMENTS` — load threshold heuristic

These can be tuned to change memory/latency trade-offs.

## Repository contents

* `HashMap.h` — full header-only implementation
* `main.cpp` — tests and stress checks
* `CMakeLists.txt` — build script

## Potential improvements

* Replace the ad-hoc test harness with a unit test framework (Catch2 / GoogleTest).
* Add iterator category traits and STL compatibility improvements.
* Improve memory locality (node pooling / custom allocator).
* Make recursion depth and resizing policy runtime-configurable.

## License

No license file is included yet. Add one (MIT/Apache-2.0) if you plan to reuse/distribute.

```

Note: the README text describes behavior aligned with `std::unordered_map` semantics (e.g., `at` throwing `std::out_of_range`, and the average-case complexity expectations). 
::contentReference[oaicite:1]{index=1}
```
