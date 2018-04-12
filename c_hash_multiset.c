﻿#include "c_hash_multiset.h"

// Создает новое хэш-мультимножество.
// Позволяет создавать хэш-мультимножество с нулем слотов.
// В случае успеха возвращает указатель на созданное хэш-мультимножество, иначе NULL.
c_hash_multiset *c_hash_multiset_create(size_t (*const _hash_func)(const void *const _data),
                                        size_t (*const _comp_func)(const void *const _a,
                                                                   const void *const _b),
                                        const size_t _data_size,
                                        const size_t _slots_count,
                                        const float _max_load_factor)
{
    if (_hash_func == NULL) return NULL;
    if (_comp_func == NULL) return NULL;
    if (_data_size == 0) return NULL;
    if (_max_load_factor <= 0.0f) return NULL;

    void *new_slots = NULL;

    if (_slots_count > 0)
    {
        const size_t new_slots_size = _slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(void*)) )
        {
            return NULL;
        }

        new_slots = malloc(new_slots_size);
        if (new_slots == NULL)
        {
            return NULL;
        }

        memset(new_slots, 0, new_slots_size);
    }

    c_hash_multiset *const new_hash_multiset = (c_hash_multiset*)malloc(sizeof(c_hash_multiset));
    if (new_hash_multiset == NULL)
    {
        free(new_slots);
        return NULL;
    }

    new_hash_multiset->hash_func = _hash_func;
    new_hash_multiset->comp_func = _comp_func;

    new_hash_multiset->data_size = _data_size;
    new_hash_multiset->slots_count = _slots_count;
    new_hash_multiset->nodes_count = 0;
    new_hash_multiset->unique_count = 0;

    new_hash_multiset->max_load_factor = _max_load_factor;

    new_hash_multiset->slots = new_slots;

    return new_hash_multiset;
}

// Удаляет хэш-мультимножество.
// В случае успеха возвращает > 0, иначе < 0.
ptrdiff_t c_hash_multiset_delete(c_hash_multiset *const _hash_multiset,
                                 void (*const _del_func)(void *const _data))
{
    if (_hash_multiset == NULL) return -1;

    if (_hash_multiset->unique_count > 0)
    {
        size_t count = _hash_multiset->unique_count;
        if (_del_func != NULL)
        {
            for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
            {
                // Если в слоте есть уникальные цепочки.
                if (((void**)_hash_multiset->slots)[s] != NULL)
                {
                    c_unique_chain *select_chain = ((c_unique_chain**)_hash_multiset->slots)[s],
                                   *delete_chain;
                    // Обходим все цепочки.
                    while (select_chain != NULL)
                    {
                        delete_chain = select_chain;
                        select_chain = select_chain->next_chain;

                        void *select_node = select_chain->head,
                             *delete_node;
                        // Обходим все узлы цепочки.
                        while (select_node != NULL)
                        {
                            delete_node = select_node;
                            select_node = *((void**)select_node);

                            _del_func( (uint8_t*)delete_node + sizeof(void*) );

                            free(delete_node);
                        }

                        free(delete_chain);

                        --count;
                    }
                }
            }
        } else {
            // Дублирование кода, чтобы на каждом узле не проверять,
            // задана ли функция удаления данных узла.
            for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
            {
                // Если в слоте есть уникальные цепочки.
                if (((void**)_hash_multiset->slots)[s] != NULL)
                {
                    c_unique_chain *select_chain = ((c_unique_chain**)_hash_multiset->slots)[s],
                                   *delete_chain;
                    // Обходим все цепочки.
                    while (select_chain != NULL)
                    {
                        delete_chain = select_chain;
                        select_chain = select_chain->next_chain;

                        void *select_node = select_chain->head,
                             *delete_node;
                        // Обходим все узлы цепочки.
                        while (select_node != NULL)
                        {
                            delete_node = select_node;
                            select_node = *((void**)select_node);

                            free(delete_node);
                        }

                        free(delete_chain);

                        --count;
                    }
                }
            }
        }
    }
    free(_hash_multiset->slots);

    free(_hash_multiset);

    return 1;
}

// Вставка данных в хэш-мультимножество.
// В случае успешной вставки возвращает > 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_multiset_insert(c_hash_multiset *const _hash_multiset,
                                 const void *const _data)
{
    if (_hash_multiset == NULL) return -1;
    if (_data == NULL) return -2;

    // Первым делом контролируем процесс увеличения количества слотов.

    // Если слотов нет вообще.
    if (_hash_multiset->slots_count == 0)
    {
        // Задаем новое количество слотов с некоторым запасом.
        const size_t new_slots_count = EXTENSION_FROM_ZERO;

        // Пытаемся расширить слоты.
        if (c_hash_multiset_resize(_hash_multiset, new_slots_count) < 0)
        {
            return -3;
        }
    } else {
        // Если слоты есть, то при достижении предела загруженности увеличиваем количество слотов.
        const float load_factor = (float)_hash_multiset->unique_count / _hash_multiset->slots_count;
        if (load_factor >= _hash_multiset->max_load_factor)
        {
            // Определим новое количество слотов.
            size_t new_slots_count = _hash_multiset->slots_count * 1.75f;
            if (new_slots_count < _hash_multiset->slots_count)
            {
                return -4;
            }
            new_slots_count += 1;
            if (new_slots_count == 0)
            {
                return -5;
            }

            // Пытаемся расширить слоты.
            if (c_hash_multiset_resize(_hash_multiset, new_slots_count) < 0)
            {
                return -6;
            }
        }
    }
    // Вставляем данные в хэш-мультимножество.

    // Неприведенный хэш вставляемых данных.
    const size_t hash = _hash_multiset->hash_func(_data);

    // Приведенный хэш.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    // Попытаемся найти в нужном слоте уникальную цепочку с требуемыми данными.
    c_unique_chain *select_chain = ((c_unique_chain**)_hash_multiset->slots)[presented_hash];

    while(select_chain != NULL)
    {
        if (hash == select_chain->hash)
        {
            const void *const data_c = (uint8_t*)(select_chain->head) + sizeof(void*);
            if (_hash_multiset->comp_func(_data, data_c) > 0)
            {
                break;
            }
        }
        select_chain = select_chain->next_chain;
    }

    // Если цепочки не существует, то создаем ее.
    size_t created = 0;
    if (select_chain == NULL)
    {
        created = 1;
        // Попытаемся создать цепочку.
        c_unique_chain *const new_chain = (c_unique_chain*)malloc(sizeof(c_unique_chain));
        if (new_chain == NULL)
        {
            return -7;
        }

        new_chain->next_chain = ((c_unique_chain**)_hash_multiset->slots)[presented_hash];
        ((void**)_hash_multiset->slots)[presented_hash] = new_chain;

        new_chain->head = NULL;

        new_chain->count = 0;
        new_chain->hash = hash;

        select_chain = new_chain;
    }

    // Создадим и вставим узел в требуемую цепочку.

    // Определим размер создаваемого узла.
    const size_t new_node_size = sizeof(void*) + _hash_multiset->data_size;
    if (new_node_size < _hash_multiset->data_size)
    {
        // Если была вставлена новая цепочка, удаляем ее.
        if (created == 1)
        {
            ((c_unique_chain**)_hash_multiset)[presented_hash] = select_chain->next_chain;
            free(select_chain);
        }
        return -8;
    }

    // Попытаемся выделить память под узел.
    void *const new_node = malloc(new_node_size);
    if (new_node == NULL)
    {
        // Если была вставлена новая цепочка, удаляем ее.
        if (created == 1)
        {
            ((c_unique_chain**)_hash_multiset)[presented_hash] = select_chain->next_chain;
            free(select_chain);
        }
        return -9;
    }

    // Скопируем данные в узел.
    memcpy((uint8_t*)new_node + sizeof(void*), _data, _hash_multiset->data_size);

    // Вставим узел в нужную цепочку.
    *((void**)new_node) = select_chain->head;
    select_chain->head = new_node;

    ++select_chain->count;

    return 1;
}

// Задает хэш-мультимножеству новое количество слотов.
// Позволяет расширить хэш-мультимножество с нулем слотов.
// Если в хэш-мультимножестве есть хотя бы один элемент, то попытка задать нулевое количество слотов считается
// ошибкой.
// Если хэш-мультимножество перестраивается, функция возвращает > 0.
// Если не перестраивается, функция возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_multiset_resize(c_hash_multiset *const _hash_multiset,
                                 const size_t _slots_count)
{
    if (_hash_multiset == NULL) return -1;

    if (_slots_count == _hash_multiset->slots_count) return 0;

    if (_slots_count == 0)
    {
        if (_hash_multiset->nodes_count != 0)
        {
            return -2;
        }

        free(_hash_multiset->slots);
        _hash_multiset->slots = NULL;

        _hash_multiset->slots_count = 0;

        return 1;
    } else {
        const size_t new_slots_size = _slots_count * sizeof(void*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(void*)) )
        {
            return -3;
        }

        void *const new_slots = malloc(new_slots_size);
        if (new_slots == NULL)
        {
            return -4;
        }

        memset(new_slots, 0, new_slots_size);

        // Если есть уникальные цепочки, которые необходимо перенести.
        if (_hash_multiset->unique_count > 0)
        {
            size_t count = _hash_multiset->unique_count;
            for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
            {
                if (((void**)_hash_multiset->slots)[s] != NULL)
                {
                    c_unique_chain *select_chain = ((c_unique_chain**)_hash_multiset->slots)[s],
                                   *relocate_chain;
                    while (select_chain != NULL)
                    {
                        relocate_chain = select_chain;
                        select_chain = select_chain->next_chain;

                        // Хэш цепочки, приведенный к новому количеству слотов.
                        const size_t presented_hash = relocate_chain->hash % _slots_count;

                        relocate_chain->next_chain = ((void**)new_slots)[presented_hash];
                        ((void**)new_slots)[presented_hash] = relocate_chain;

                        --count;
                    }
                }
            }

        }

        free(_hash_multiset->slots);

        // Используем новые слоты.
        _hash_multiset->slots = new_slots;
        _hash_multiset->slots_count = _slots_count;

        return 2;
    }
}