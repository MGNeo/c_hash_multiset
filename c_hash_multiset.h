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

#include <stddef.h>

typedef struct s_c_hash_multiset c_hash_multiset;

c_hash_multiset *c_hash_multiset_create(size_t (*const _hash_data)(const void *const _data),
                                        size_t (*const _comp_data)(const void *const _data_a,
                                                                   const void *const _data_b),
                                        const size_t _slots_count,
                                        const float _max_load_factor,
                                        size_t *const _error);

ptrdiff_t c_hash_multiset_delete(c_hash_multiset *const _hash_multiset,
                                 void (*const _del_data)(void *const _data));

ptrdiff_t c_hash_multiset_insert(c_hash_multiset *const _hash_multiset,
                                 const void *const _data);

ptrdiff_t c_hash_multiset_erase(c_hash_multiset *const _hash_multiset,
                                const void *const _data,
                                void (*const _del_data)(void *const _data));

ptrdiff_t c_hash_multiset_resize(c_hash_multiset *const _hash_multiset,
                                 const size_t _slots_count);

ptrdiff_t c_hash_multiset_check(const c_hash_multiset *const _hash_multiset,
                                const void *const _data);

size_t c_hash_multiset_data_count(const c_hash_multiset *const _hash_multiset,
                                  const void *const _data,
                                  size_t *const _error);

ptrdiff_t c_hash_multiset_for_each(const c_hash_multiset *const _hash_multiset,
                                   void (*const _action_data)(const void *const _data));

ptrdiff_t c_hash_multiset_clear(c_hash_multiset *const _hash_multiset,
                                void (*const _del_data)(void *const _data));

ptrdiff_t c_hash_multiset_erase_all(c_hash_multiset *const _hash_multiset,
                                    const void *const _data,
                                    void (*const _del_data)(void *const _data));

size_t c_hash_multiset_slots_count(const c_hash_multiset *const _hash_multiset,
                                   size_t *const _error);

size_t c_hash_multiset_count(const c_hash_multiset *const _hash_multiset,
                             size_t *const _error);

size_t c_hash_multiset_uniques_count(const c_hash_multiset *const _hash_multiset,
                                     size_t *const _error);

float c_hash_multiset_max_load_factor(const c_hash_multiset *const _hash_multiset);

#endif
