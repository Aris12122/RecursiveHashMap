//
// Created by aris on 01/21/23.
// Recursive Hash Map
//
#pragma once

#include <iterator>
#include <memory>
#include <vector>

const uint8_t MAX_RECURSIVE_LEVEL = 5; //0..9
const uint8_t MAX_SIZE_ID = 16;
const uint8_t MAX_SIZE_DIV_NUMBER_OF_ELEMENTS = 4; // the number of elements is 10 times less than the max_size

const size_t increase_primes[MAX_RECURSIVE_LEVEL] {
        34583,
        24239,
        131,
        1031,
        6761,
//        3461,
//        91243,
//        29,
//        7243,
//        9391,
};

const size_t max_sizes[MAX_SIZE_ID] {
        13,
        23,
        73,
        173,
        401,
        929,
        2137,
        4931,
        11351,
        26113,
        60103,
        138239,
        318023,
        731531,
        1463113,
        3365161,
};

template<typename KeyType, typename ValueType,
        typename Hash = std::hash<KeyType>>
class HashMap {
public:
    explicit HashMap(const Hash& hash, uint8_t level, size_t from, HashMap* par) :
            hasher(hash), recursive_level(level), id_max_size(0),
            number_of_elements(0), stupid(true), from_index(from), parent(par), open_cells(0),
            increase(increase_primes[recursive_level]), max_size(max_sizes[id_max_size]) {}

    explicit HashMap(const Hash& hash = Hash()) : HashMap(hash, 0, 0, NULL) {}

    template<class Iterator>
    HashMap(Iterator it_begin, Iterator it_end,
            const Hash& hash = Hash()) : HashMap(hash) {
        for (Iterator it = it_begin; it != it_end; ++it) {
            insert(*it);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list,
            const Hash& hash = Hash()) : HashMap(hash) {
        for (const std::pair<KeyType, ValueType>& element : list) {
            insert(element);
        }
    }

    HashMap(const HashMap& other) : HashMap(other.hasher) {
        *this = other;
    }



    struct iterator {
        using ItValueType = std::pair<const KeyType, ValueType>;
        explicit iterator() = default;

        explicit iterator(ItValueType* _value, HashMap* _from, uint8_t id, uint8_t rec_level = 0) : value(_value), from(_from), index(id), local_recursive_level(rec_level) {}

        iterator(const iterator& other) : iterator(other.value, other.from, other.index, other.local_recursive_level) {}
        iterator(const iterator& other, uint8_t rec_level) : iterator(other.value, other.from, other.index, rec_level) {}


        bool operator==(const iterator &other) {
            return value == other.value && local_recursive_level == other.local_recursive_level;
        }
        bool operator!=(const iterator &other) {
            return !(*this == other);
        }


        ItValueType& operator*() const {
            return *value;
        }
        ItValueType* operator->() const {
            return value;
        }

        iterator& operator=(const iterator& other) {
            if (&other == this) {
                return *this;
            }
            value = other.value;
            from = other.from;
            index = other.index;
            local_recursive_level = other.local_recursive_level;
            return *this;
        }

        iterator end() {
            return iterator();
        }

        iterator& operator++() {
            if (*this == end()) {
                return *this;
            }
            HashMap* map = from;
            if (!map->stupid) {
                *this = end();
                return *this;
            }
            if (static_cast<size_t>(index + 1) < map->size()) {
                *this = iterator(&(map->small_data[index+1]), from, index + 1, local_recursive_level);
                return *this;
            }

            size_t id;
            while(map->recursive_level > local_recursive_level) {
                id = map->from_index + 1;
                map = map->parent;
                while (id < map->max_size)  {

                    if (map->data[id].get() && !map->data[id].get()->empty()) {
                        *this = iterator(map->data[id].get()->begin(), local_recursive_level);
                        return *this;
                    }
                    ++id;
                }
            }
            *this = end();
            return *this;
        }
        iterator operator++(int) {
            iterator result = *this;
            ++(*this);
            return result;
        }
    private:
        ItValueType* value;
        HashMap* from;
        uint8_t index{};
        uint8_t local_recursive_level{};
    };

    struct const_iterator {
        using ItValueType = const std::pair<const KeyType, ValueType>;
        explicit const_iterator() = default;

        explicit const_iterator(ItValueType* _value, const HashMap* _from, uint8_t id, uint8_t rec_level = 0) : value(_value), from(_from), index(id), local_recursive_level(rec_level) {}

        const_iterator(const const_iterator& other) : const_iterator(other.value, other.from, other.index, other.local_recursive_level) {}
        const_iterator(const const_iterator& other, uint8_t rec_level) : const_iterator(other.value, other.from, other.index, rec_level) {}

        bool operator==(const const_iterator &other) {
            return value == other.value && local_recursive_level == other.local_recursive_level;
        }
        bool operator!=(const const_iterator &other) {
            return !(*this == other);
        }


        ItValueType& operator*() const {
            return *value;
        }
        ItValueType* operator->() const {
            return value;
        }

        const_iterator& operator=(const const_iterator& other) {
            if (&other == this) {
                return *this;
            }
            value = other.value;
            from = other.from;
            index = other.index;
            local_recursive_level = other.local_recursive_level;
            return *this;
        }

        const_iterator end() {
            return const_iterator();
        }

        const_iterator& operator++() {
            if (*this == end()) {
                return *this;
            }
            const HashMap* map = from;
            if (!map->stupid) {
                *this = end();
                return *this;
            }
            if (static_cast<size_t>(index + 1) < map->size()) {
                *this = const_iterator(&(map->small_data[index+1]), from, index + 1, local_recursive_level);
                return *this;
            }

            size_t id;
            while(map->recursive_level > local_recursive_level) {
                id = map->from_index + 1;
                map = map->parent;
                while (id < map->max_size)  {

                    if (map->data[id].get() && !map->data[id].get()->empty()) {
                        *this = const_iterator(map->data[id].get()->begin(true), local_recursive_level);
                        return *this;
                    }
                    ++id;
                }
            }
            *this = end();
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator result = *this;
            ++(*this);
            return result;
        }
    private:
        ItValueType* value;
        const HashMap* from;
        uint8_t index{};
        uint8_t local_recursive_level{};
    };

    iterator begin() {
        if (number_of_elements == 0) {
            return end();
        }
        if (stupid) {
            return iterator(&small_data[0], this, 0);
        } else {
            for (size_t i = 0;; ++i) {
                if (data[i].get() && !data[i].get()->empty()) {
                    return iterator(data[i].get()->begin(), recursive_level);
                }
            }
        }
    }
    const_iterator begin(bool inner = false) const {
        if (number_of_elements == 0) {
            return end();
        }
        if (stupid) {
            return const_iterator(&small_data[0], this, 0);
        } else {
            for (size_t i = 0;; ++i) {
                if (data[i].get() && !data[i].get()->empty()) {
                    return const_iterator(data[i].get()->begin(true), recursive_level);
                }
            }
        }
    }

    iterator end() {
        return iterator();
    }
    const_iterator end() const {
        return const_iterator();
    }

    Hash hash_function() const {
        return hasher;
    }

    iterator find(const KeyType& key) {
        if (stupid) {
            for (size_t i = 0; i < small_data.size(); ++i) {
                if (small_data[i].first == key) {
                    return iterator(&small_data[i], this, i);
                }
            }
            return end();
        } else {
            size_t pos = GetPos(key);
            if (!data[pos]) {
                return end();
            }
            return data[pos].get()->find(key);
        }
    }

    const_iterator find(const KeyType& key, bool flag = true) const {
        if (stupid) {
            for (size_t i = 0; i < small_data.size(); ++i) {
                if (small_data[i].first == key) {
                    return const_iterator(&small_data[i], this, i);
                }
            }
            return end();
        } else {
            size_t pos = GetPos(key);
            if (!data[pos]) {
                return end();
            }
            return data[pos].get()->find(key, true);
        }
    }

    ValueType& at(const KeyType& key) {
        iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("Out of Range error with at");
        }
        return it->second;
    }

    const ValueType& at(const KeyType& key) const {
        const_iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("Out of Range error with at");
        }
        return it->second;
    }

    bool empty() const {
        return size() == 0;
    }
    size_t size() const {
        return number_of_elements;
    }

    bool insert(const std::pair<const KeyType, ValueType>& add) {
        if (stupid) {
            for (const auto& [key, value] : small_data) {
                if (key == add.first) return false;
            }
            small_data.emplace_back(add);
            number_of_elements++;
            if (!LastLevel() && number_of_elements * MAX_SIZE_DIV_NUMBER_OF_ELEMENTS >= max_sizes[id_max_size]) {
                Expand();
            }
            return true;
        } else {
            const auto& [key, value] = add;
            size_t pos = GetPos(key);
            if (!data[pos]) {
                ++open_cells;
                data[pos] = std::make_unique<HashMap<KeyType, ValueType, Hash>>(hasher, recursive_level + 1, pos, this);
            }
            if (data[pos].get()->insert(add)) {
                ++number_of_elements;
                if (open_cells * MAX_SIZE_DIV_NUMBER_OF_ELEMENTS >= max_size) {
                    Expand();
                }
                return true;
            }
            return false;
        }

    }

    bool erase(const KeyType& key) {
        if (stupid) {
            bool has = false;
            for (size_t i = 0; i < small_data.size(); ++i) {
                if (small_data[i].first == key) {
                    has = true;
                }
            }
            if (!has) {
                return false;
            }
            --number_of_elements;
            std::vector<std::pair<const KeyType, ValueType>> temp = small_data;
            small_data.clear();
            for (const auto& element : temp) if (element.first != key) {
                small_data.emplace_back(element);
            }
            return true;
        } else {
            size_t pos = GetPos(key);
            if (data[pos] && data[pos].get()->erase(key)) {
                --number_of_elements;
                if (data[pos].get()->empty()) {
                    --open_cells;
                    data[pos] = nullptr;
                    if (open_cells * MAX_SIZE_DIV_NUMBER_OF_ELEMENTS * MAX_SIZE_DIV_NUMBER_OF_ELEMENTS <= max_size) {
                        Reduce();
                    }
                }
                return true;
            }
            return false;
        }
    }

    ValueType& operator[](const KeyType& key) {
        auto it = find(key);
        if (it == end()) {
            insert({key, ValueType{}});
            it = find(key);
        }
        return it->second;
    }

    ValueType& operator[](const KeyType& key) const {
        auto it = find(key);
        if (it == end()) {
            insert({key, ValueType{}});
            it = find(key);
        }
        return it->second;
    }


private:
    bool LastLevel() {
        return recursive_level + 1 == MAX_RECURSIVE_LEVEL;
    }

    size_t GetPos(const KeyType& key) const {
        return static_cast<size_t>((static_cast<long long>(hasher(key) % max_size) * (increase % max_size)) % max_size);
    }

    void Expand() {
        if (id_max_size + 1 == MAX_SIZE_ID) return;
        std::vector<std::pair<KeyType, ValueType>> to_add;
        if (stupid) {
            for (const auto& element : small_data) {
                to_add.emplace_back(element);
            }
            small_data.clear();
            stupid = false;
        } else {
            for (const auto& element : *this) {
                to_add.emplace_back(element);
            }
        }
        ++id_max_size;
        max_size = max_sizes[id_max_size];
        data.clear();
        data = std::vector<std::unique_ptr<HashMap>>(max_size);
        number_of_elements = 0;
        open_cells = 0;
        for (auto& element : to_add) {
            insert(element);
        }
    }

    void Reduce() {
        if (id_max_size == 0) {
            return;
        }
        std::vector<std::pair<KeyType, ValueType>> to_add;
        for (const auto& element : *this) {
            to_add.emplace_back(element);
        }
        if (id_max_size == 1 && number_of_elements * MAX_SIZE_DIV_NUMBER_OF_ELEMENTS < max_sizes[0]) {
            stupid = true;
            max_size = max_sizes[id_max_size = 0];
            data.clear();
            small_data.clear();
            for (const auto& element : to_add) {
                insert(element);
            }
            return;
        }

        max_size = max_sizes[--id_max_size];
        data.clear();
        data = std::vector<std::unique_ptr<HashMap>>(max_size);
        number_of_elements = 0;
        open_cells = 0;
        for (auto& element : to_add) {
            insert(element);
        }
    }

public:
    ~HashMap() {
        data.clear();
        small_data.clear();
    }

    void clear() {
        data.clear();
        small_data.clear();

        max_size = max_sizes[id_max_size = 0];
        increase = increase_primes[recursive_level = 0];
        open_cells = 0;
        number_of_elements = 0;
        stupid = true;
        parent = nullptr;
        from_index = 0;
    }

    HashMap& operator=(const HashMap& other) {
        if (&other == this) {
            return *this;
        }
        clear();
        if (other.stupid) {
            for (const auto& element : other) {
                insert(element);
            }
            return *this;
        }
        stupid = false;
        hasher = other.hasher;
        id_max_size = other.id_max_size;
        max_size = other.max_size;
        data = std::vector<std::unique_ptr<HashMap>>(max_size);
        for (const auto& element : other) {
            insert(element);
        }
        return *this;
    }

private:
    Hash hasher;
    uint8_t recursive_level;
    uint8_t id_max_size;
    size_t number_of_elements;
    bool stupid;
    size_t from_index;
    HashMap* parent;
    size_t open_cells;
    size_t increase; // prime, hash -> hash * increase
    size_t max_size; // prime, num of cells for elements

private:
    std::vector<std::pair<const KeyType, ValueType>> small_data;
    std::vector<std::unique_ptr<HashMap<KeyType, ValueType, Hash>>> data;
};
