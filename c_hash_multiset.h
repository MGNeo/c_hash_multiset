#ifndef C_HASH_MULTISET_H
#define C_HASH_MULTISET_H

/*отладочное*/#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stddef.h>

// Количество слотов, задаваемое хэш-мультимножеству с нулем слотов при автоматическом
// расширении.
#define EXTENSION_FROM_ZERO ( (size_t) 1024 )

/*
 * Слот - это односвязный список, который содержит односвязные списки
 * одинаковых элементов (уникальные цепи).
 *
 * Узел, содержащий данные:
 * Что за данные:                   |___next___|_________data_________|
 * Представление:                   |___void*__|__uint8_t[data_size]__|
 *
 * Указатель на узел указывает сюда ^
 */

typedef struct s_c_unique_chain
{
    void *next_chain;
    void *head;
    size_t count;
    size_t hash;
} c_unique_chain;

typedef struct s_c_hash_multiset
{
    // Функция, генерирующая хэш на основе данных.
    size_t (*hash_func)(const void *const _data);
    // Функция детального сравнения данных.
    // В случае идентичности данных должна возвращать > 0, иначе 0.
    size_t (*comp_func)(const void *const _a,
                        const void *const _b);

    size_t data_size;
    size_t slots_count;
    size_t nodes_count;
    size_t unique_count;

    float max_load_factor;

    void *slots;
} c_hash_multiset;

c_hash_multiset *c_hash_multiset_create(size_t (*const _hash_func)(const void *const _data),
                                        size_t (*const _comp_func)(const void *const _a,
                                                                   const void *const _b),
                                        const size_t _data_size,
                                        const size_t _slots_count,
                                        const float _max_load_factor);

ptrdiff_t c_hash_multiset_delete(c_hash_multiset *const _hash_multiset,
                                 void (*const _del_func)(void *const _data));

ptrdiff_t c_hash_multiset_insert(c_hash_multiset *const _hash_multiset,
                                 const void *const _data);

ptrdiff_t c_hash_multiset_erase(c_hash_multiset *const _hash_multiset,
                                const void *const _data,
                                void (*const _del_func)(void *const _data));

ptrdiff_t c_hash_multiset_resize(c_hash_multiset *const _hash_multiset,
                                 const size_t _slots_count);

ptrdiff_t c_hash_multiset_check(const c_hash_multiset *const _hash_multiset,
                                const void *const _data);

size_t c_hash_multiset_count(const c_hash_multiset *const _hash_multiset,
                             const void *const _data);

ptrdiff_t c_hash_multiset_for_each(const c_hash_multiset *const _hash_multiset,
                                   void (*const _func)(const void *const _data));

// clear

// erase_all

#endif
