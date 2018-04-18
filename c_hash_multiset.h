/*
    Заголовочный файл хэш-мультимножества c_hash_multiset
    Разработка, отладка и сборка производилась в:
    ОС: Windows 10/x64
    IDE: Code::Blocks 17.12
    Компилятор: default Code::Blocks 17.12 MinGW

    Разработчик: Глухманюк Максим
    Эл. почта: mgneo@yandex.ru
    Место: Российская Федерация, Самарская область, Сызрань
    Дата: 13.04.2018
    Лицензия: GPLv3
*/

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
#define C_HASH_MULTISET_0 ( (size_t) 1024 )

typedef struct s_c_hash_multiset_node
{
    struct s_c_hash_multiset_node *next;
    void *data;
} c_hash_multiset_node;

typedef struct s_c_hash_multiset_chain
{
    struct s_c_hash_multiset_chain *next_chain;
    c_hash_multiset_node *head;
    size_t count,
           hash;
} c_hash_multiset_chain;

typedef struct s_c_hash_multiset
{
    // Функция, генерирующая хэш на основе данных.
    size_t (*hash_func)(const void *const _data);
    // Функция детального сравнения данных.
    // В случае идентичности данных должна возвращать > 0, иначе 0.
    size_t (*comp_func)(const void *const _a,
                        const void *const _b);

    size_t slots_count,
           nodes_count,
           unique_count;

    float max_load_factor;

    c_hash_multiset_chain **slots;
} c_hash_multiset;

c_hash_multiset *c_hash_multiset_create(size_t (*const _hash_func)(const void *const _data),
                                        size_t (*const _comp_func)(const void *const _a,
                                                                   const void *const _b),
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

ptrdiff_t c_hash_multiset_clear(c_hash_multiset *const _hash_multiset,
                                void (*const _del_func)(void *const _data));

ptrdiff_t c_hash_multiset_erase_all(c_hash_multiset *const _hash_multiset,
                                    const void *const _data,
                                    void (*const _del_func)(void *const _data));

#endif
