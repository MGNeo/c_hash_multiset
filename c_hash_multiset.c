/*
    Файл реализации хэш-мультимножества c_hash_multiset
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "c_hash_multiset.h"

// Количество слотов, задаваемое хэш-мультимножеству с нулем слотов при автоматическом
// расширении.
#define C_HASH_MULTISET_0 ( (size_t) 1024 )

// Минимально возможное значение max_load_factor.
#define C_HASH_MULTISET_MLF_MIN ( (float) 0.01f )

// Максимально возможное значение max_load_factor.
#define C_HASH_MULTISET_MLF_MAX ( (float) 1.0f )

typedef struct s_c_hash_multiset_node c_hash_multiset_node;

typedef struct s_c_hash_multiset_chain c_hash_multiset_chain;

struct s_c_hash_multiset_node
{
    struct s_c_hash_multiset_node *next_node;
    void *data;
};

struct s_c_hash_multiset_chain
{
    struct s_c_hash_multiset_chain *next_chain;
    c_hash_multiset_node *head;
    size_t count,
           hash;
};

struct s_c_hash_multiset
{
    // Функция, генерирующая хэш на основе данных.
    size_t (*hash_data)(const void *const _data);
    // Функция детального сравнения данных.
    // В случае идентичности данных должна возвращать > 0, иначе 0.
    size_t (*comp_data)(const void *const _data_a,
                        const void *const _data_b);

    size_t slots_count,
           nodes_count,
           uniques_count;

    float max_load_factor;

    c_hash_multiset_chain **slots;
};

// Создает новое хэш-мультимножество.
// Позволяет создавать хэш-мультимножество с нулем слотов.
// В случае успеха возвращает указатель на созданное хэш-мультимножество, иначе NULL.
c_hash_multiset *c_hash_multiset_create(size_t (*const _hash_data)(const void *const _data),
                                        size_t (*const _comp_data)(const void *const _data_a,
                                                                   const void *const _data_b),
                                        const size_t _slots_count,
                                        const float _max_load_factor)
{
    if (_hash_data == NULL) return NULL;
    if (_comp_data == NULL) return NULL;
    if ( (_max_load_factor < C_HASH_MULTISET_MLF_MIN) ||
         (_max_load_factor > C_HASH_MULTISET_MLF_MAX) )
    {
        return NULL;
    }

    c_hash_multiset_chain **new_slots = NULL;

    if (_slots_count > 0)
    {
        const size_t new_slots_size = _slots_count * sizeof(c_hash_multiset_chain*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(c_hash_multiset_chain*)) )
        {
            return NULL;
        }

        new_slots = (c_hash_multiset_chain**)malloc(new_slots_size);
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

    new_hash_multiset->hash_data = _hash_data;
    new_hash_multiset->comp_data = _comp_data;

    new_hash_multiset->slots_count = _slots_count;
    new_hash_multiset->nodes_count = 0;
    new_hash_multiset->uniques_count = 0;

    new_hash_multiset->max_load_factor = _max_load_factor;

    new_hash_multiset->slots = new_slots;

    return new_hash_multiset;
}

// Удаляет хэш-мультимножество.
// В случае успеха возвращает > 0, иначе < 0.
ptrdiff_t c_hash_multiset_delete(c_hash_multiset *const _hash_multiset,
                                 void (*const _del_data)(void *const _data))
{
    if (c_hash_multiset_clear(_hash_multiset, _del_data) < 0)
    {
        return -1;
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
        // Пытаемся расширить слоты.
        if (c_hash_multiset_resize(_hash_multiset, C_HASH_MULTISET_0) <= 0)
        {
            return -4;
        }
    } else {
        // Если слоты есть, то при достижении предела загруженности увеличиваем количество слотов.
        const float load_factor = (float)_hash_multiset->uniques_count / _hash_multiset->slots_count;
        if (load_factor >= _hash_multiset->max_load_factor)
        {
            // Определим новое количество слотов.
            size_t new_slots_count = (size_t)(_hash_multiset->slots_count * 1.75f);
            if (new_slots_count < _hash_multiset->slots_count)
            {
                return -5;
            }
            new_slots_count += 1;
            if (new_slots_count == 0)
            {
                return -6;
            }

            // Пытаемся расширить слоты.
            if (c_hash_multiset_resize(_hash_multiset, new_slots_count) < 0)
            {
                return -7;
            }
        }
    }
    // Вставляем данные в хэш-мультимножество.

    // Неприведенный хэш вставляемых данных.
    const size_t hash = _hash_multiset->hash_data(_data);

    // Приведенный хэш.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    // Попытаемся найти в нужном слоте уникальную цепочку с требуемыми данными.
    c_hash_multiset_chain *select_chain = _hash_multiset->slots[presented_hash];

    while(select_chain != NULL)
    {
        if (hash == select_chain->hash)
        {
            if (_hash_multiset->comp_data(_data, select_chain->head->data) > 0)
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
        c_hash_multiset_chain *const new_chain = (c_hash_multiset_chain*)malloc(sizeof(c_hash_multiset_chain));
        if (new_chain == NULL)
        {
            return -8;
        }

        // Встроим цепочку в слот.
        new_chain->next_chain = _hash_multiset->slots[presented_hash];
        _hash_multiset->slots[presented_hash] = new_chain;

        // Установим параметры цепи.
        new_chain->head = NULL;
        new_chain->count = 0;
        new_chain->hash = hash;

        // Цепей стало больше.
        ++_hash_multiset->uniques_count;

        select_chain = new_chain;
    }

    // Создадим и вставим узел в требуемую цепочку.
    // Если узел не удалось создать, и если мы создавали цепочку, то удаляем ее,
    // потому что пустая цепочка не должна существовать.

    // Попытаемся выделить память под узел.
    c_hash_multiset_node *const new_node = (c_hash_multiset_node*)malloc(sizeof(c_hash_multiset_node));
    if (new_node == NULL)
    {
        if (created == 1)
        {
            _hash_multiset->slots[presented_hash] = select_chain->next_chain;
            free(select_chain);
            --_hash_multiset->uniques_count;
        }
        return -9;
    }

    // Свяжем узел с данными.
    new_node->data = (void*)_data;

    // Вставим узел в нужную цепочку.
    new_node->next_node = select_chain->head;
    select_chain->head = new_node;
    ++select_chain->count;

    // Объектов в хэш-мультимножестве стало больше.
    ++_hash_multiset->nodes_count;

    return 1;
}

// Удаляет из хэш-мультимножества одну единицу заданных данных.
// В случае успешного удаления возвращает > 0.
// В случае, если заданных данных в хэш-мультимножестве нет, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_multiset_erase(c_hash_multiset *const _hash_multiset,
                                const void *const _data,
                                void (*const _del_data)(void *const _data))
{
    if (_hash_multiset == NULL) return -1;
    if (_data == NULL) return -2;

    if (_hash_multiset->uniques_count == 0) return 0;

    // Неприведенный хэш искомых данных.
    const size_t hash = _hash_multiset->hash_data(_data);

    // Приведенный хэш искомых данных.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    // Поиск цепи с заданными данными.
    c_hash_multiset_chain *select_chain = _hash_multiset->slots[presented_hash],
                          *prev_chain = NULL;
    while (select_chain != NULL)
    {
        if (hash == select_chain->hash)
        {
            if (_hash_multiset->comp_data(_data, select_chain->head->data) > 0)
            {
                // Удаляем первый узел из требуемой цепи.
                c_hash_multiset_node *delete_node = select_chain->head;
                select_chain->head = select_chain->head->next_node;

                if (_del_data != NULL)
                {
                    _del_data( delete_node->data );
                }
                free(delete_node);

                --select_chain->count;
                --_hash_multiset->nodes_count;

                // Если цепочка опустела, удаляем ее, сшивая разрыв.
                if (select_chain->count == 0)
                {
                    if (prev_chain != NULL)
                    {
                        prev_chain->next_chain = select_chain->next_chain;
                    } else {
                        _hash_multiset->slots[presented_hash] = select_chain->next_chain;
                    }
                    free(select_chain);

                    --_hash_multiset->uniques_count;
                }
                return 1;
            }
        }
        prev_chain = select_chain;
        select_chain = select_chain->next_chain;
    }

    return 0;
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
        if (_hash_multiset->uniques_count != 0)
        {
            return -2;
        }

        free(_hash_multiset->slots);
        _hash_multiset->slots = NULL;

        _hash_multiset->slots_count = 0;

        return 1;
    } else {
        const size_t new_slots_size = _slots_count * sizeof(c_hash_multiset_chain*);
        if ( (new_slots_size == 0) ||
             (new_slots_size / _slots_count != sizeof(c_hash_multiset_chain*)) )
        {
            return -3;
        }

        c_hash_multiset_chain **const new_slots = (c_hash_multiset_chain**)malloc(new_slots_size);
        if (new_slots == NULL)
        {
            return -4;
        }

        memset(new_slots, 0, new_slots_size);

        // Если есть уникальные цепочки, которые необходимо перенести.
        if (_hash_multiset->uniques_count > 0)
        {
            size_t count = _hash_multiset->uniques_count;
            for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
            {
                if (_hash_multiset->slots[s] != NULL)
                {
                    c_hash_multiset_chain *select_chain = _hash_multiset->slots[s],
                                          *relocate_chain;
                    while (select_chain != NULL)
                    {
                        relocate_chain = select_chain;
                        select_chain = select_chain->next_chain;

                        // Хэш цепочки, приведенный к новому количеству слотов.
                        const size_t presented_hash = relocate_chain->hash % _slots_count;

                        // Перенос цепочки.
                        relocate_chain->next_chain = new_slots[presented_hash];
                        new_slots[presented_hash] = relocate_chain;

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

// Проверяет наличие заданных данных в хэш-мультимножестве.
// Если данные есть, возвращает > 0.
// Если данных нет, возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_multiset_check(const c_hash_multiset *const _hash_multiset,
                                const void *const _data)
{
    if (_hash_multiset == NULL) return -1;
    if (_data == NULL) return -2;

    if (_hash_multiset->uniques_count == 0) return 0;

    // Неприведенный хэш данных.
    const size_t hash = _hash_multiset->hash_data(_data);

    // Приведенный хэш.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    const c_hash_multiset_chain *select_chain = _hash_multiset->slots[presented_hash];
    while (select_chain != NULL)
    {
        if (hash == select_chain->hash)
        {
            if (_hash_multiset->comp_data(_data, select_chain->head->data) > 0)
            {
                return 1;
            }
        }
        select_chain = select_chain->next_chain;
    }

    return 0;
}

// Проверяет количество заданных данных в хэш-мультимножестве.
// В случае успеха возвращает количество заданных данных (включая 0, если данных нет).
// В случае ошибки возвращае 0.
size_t c_hash_multiset_count(const c_hash_multiset *const _hash_multiset,
                             const void *const _data)
{
    if (_hash_multiset == NULL) return 0;
    if (_data == NULL) return 0;

    if (_hash_multiset->uniques_count == 0) return 0;

    // Неприведенный хэш искомых данных.
    const size_t hash = _hash_multiset->hash_data(_data);

    // Приведенный хэш.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    const c_hash_multiset_chain *select_chain = _hash_multiset->slots[presented_hash];
    while (select_chain != NULL)
    {
        if (hash == select_chain->hash)
        {
            if (_hash_multiset->comp_data(_data, select_chain->head->data) > 0)
            {
                return select_chain->count;
            }
        }

        select_chain = select_chain->next_chain;
    }

    return 0;
}

// Проходит по всем данным хэш-мультимножества и выполняет над ними заданные действия.
// В случае успешного выполнения возвращает > 0.
// В случае, если в хэш-мультимножестве нет элементов, возвращает 0.
// В случае ошибки < 0.
ptrdiff_t c_hash_multiset_for_each(const c_hash_multiset *const _hash_multiset,
                                   void (*const _action_data)(const void *const _data))
{
    if (_hash_multiset == NULL) return -1;
    if (_action_data == NULL) return -2;

    if (_hash_multiset->uniques_count == 0) return 0;

    size_t count = _hash_multiset->uniques_count;
    for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
    {
        if (_hash_multiset->slots[s] != NULL)
        {
            const c_hash_multiset_chain *select_chain = _hash_multiset->slots[s];
            while (select_chain != NULL)
            {
                const c_hash_multiset_node *select_node = select_chain->head;
                while (select_node != NULL)
                {
                    _action_data( select_node->data );
                    select_node = select_node->next_node;
                }
                select_chain = select_chain->next_chain;
                --count;
            }
        }
    }

    return 1;
}

// Очищает хэш-мультимножество ото всех данных, количество слотов сохраняется.
// В случае успешного очищения возвращает > 0.
// Если очищать не от чего, возвращает 0.
// В случае ошибвки возвращает < 0.
ptrdiff_t c_hash_multiset_clear(c_hash_multiset *const _hash_multiset,
                                void (*const _del_data)(void *const _data))
{
    if (_hash_multiset == NULL) return -1;

    if (_hash_multiset->uniques_count == 0) return 0;

    size_t count = _hash_multiset->uniques_count;
    if (_del_data != NULL)
    {
        for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
        {
            if (_hash_multiset->slots[s] != NULL)
            {
                c_hash_multiset_chain *select_chain = _hash_multiset->slots[s],
                                      *delete_chain;

                while (select_chain != NULL)
                {
                    delete_chain = select_chain;
                    select_chain = select_chain->next_chain;

                    c_hash_multiset_node *select_node = delete_chain->head,
                                         *delete_node;
                    while (select_node != NULL)
                    {
                        delete_node = select_node;
                        select_node = select_node->next_node;

                        _del_data( delete_node->data );
                        free(delete_node);
                    }
                    free(delete_chain);
                    --count;
                }
                _hash_multiset->slots[s] = NULL;
            }
        }
    } else {
        // Дублирование кода для того, чтобы на каждом узле не проверять,
        // задана ли функция удаления данных.
        for (size_t s = 0; (s < _hash_multiset->slots_count)&&(count > 0); ++s)
        {
            if (_hash_multiset->slots[s] != NULL)
            {
                c_hash_multiset_chain *select_chain = _hash_multiset->slots[s],
                                      *delete_chain;
                while (select_chain != NULL)
                {
                    delete_chain = select_chain;
                    select_chain = select_chain->next_chain;

                    c_hash_multiset_node *select_node = delete_chain->head,
                                         *delete_node;
                    while (select_node != NULL)
                    {
                        delete_node = select_node;
                        select_node = select_node->next_node;

                        free(delete_node);
                    }
                    free(delete_chain);
                    --count;
                }
                _hash_multiset->slots[s] = NULL;
            }
        }
    }
    return 1;
}

// Удаляет из хэш-мультимножества все единицы заданных данных.
// В случае успешного удаления возвращает > 0.
// Если заданных данных в хэш-мультимножестве не оказалось, то возвращает 0.
// В случае ошибки возвращает < 0.
ptrdiff_t c_hash_multiset_erase_all(c_hash_multiset *const _hash_multiset,
                                    const void *const _data,
                                    void (* const _del_data)(void *const _data))
{
    if (_hash_multiset == NULL) return -1;
    if (_data == NULL) return -2;

    if (_hash_multiset->uniques_count == 0) return 0;

    // Неприведенный хэш заданных данных.
    const size_t hash = _hash_multiset->hash_data(_data);

    // Приведенный хэш заданных данных.
    const size_t presented_hash = hash % _hash_multiset->slots_count;

    if (_hash_multiset->slots[presented_hash] != NULL)
    {
        c_hash_multiset_chain *select_chain = _hash_multiset->slots[presented_hash],
                              *prev_chain = NULL;

        while (select_chain != NULL)
        {
            if (hash == select_chain->hash)
            {
                if (_hash_multiset->comp_data(_data, select_chain->head->data) > 0)
                {
                    // Удаляем заданную цепь из хэш-мультимножества.
                    c_hash_multiset_node *select_node = select_chain->head,
                                         *delete_node;
                    // Сперва удаляем все узлы цепи.
                    if (_del_data != NULL)
                    {
                        while (select_node != NULL)
                        {
                            delete_node = select_node;
                            select_node = select_node->next_node;

                            _del_data( delete_node->data );
                            free(delete_node);
                        }
                    } else {
                        // Дублирование кода, чтобы на каждом узле не проверять,
                        // задана ли функция удаления.
                        while (select_node != NULL)
                        {
                            delete_node = select_node;
                            select_node = select_node->next_node;

                            free(delete_node);
                        }
                    }

                    // Уникальных цепей стало меньше на одну.
                    --_hash_multiset->uniques_count;
                    // Элементов в хэш-мультимножестве стало меньше на количество элементов удаляемой цепи.
                    _hash_multiset->nodes_count -= select_chain->count;

                    // Ампутация цепи.
                    if (prev_chain != NULL)
                    {
                        prev_chain->next_chain = select_chain->next_chain;
                    } else {
                        _hash_multiset->slots[presented_hash] = select_chain->next_chain;
                    }

                    free(select_chain);

                    return 1;
                }
            }
            prev_chain = select_chain;
            select_chain = select_chain->next_chain;
        }
    }

    return 0;
}

// Возвращает количество слотов в хэш-мультимножестве.
// В случае ошибки возвращает 0.
size_t c_hash_multiset_slots_count(const c_hash_multiset *const _hash_multiset)
{
    if (_hash_multiset == NULL)
    {
        return 0;
    }

    return _hash_multiset->slots_count;
}

// Возвращает количество узлов (объектов) в хэш-мультимножестве.
// В случае ошибки возвращает 0.
size_t c_hash_multiset_nodes_count(const c_hash_multiset *const _hash_multiset)
{
    if (_hash_multiset == NULL)
    {
        return 0;
    }

    return _hash_multiset->nodes_count;
}

// Возвращает количество уникальных объектов в хэш-мультимножестве.
// В случае ошибки возвращает 0.
size_t c_hash_multiset_uniques_count(const c_hash_multiset *const _hash_multiset)
{
    if (_hash_multiset == NULL)
    {
        return 0;
    }

    return _hash_multiset->uniques_count;
}

// Возвращает коэф. максимальной загрузки хэш-мультимножества.
// В случае ошибки возвращает 0.0f.
float c_hash_multiset_max_load_factor(const c_hash_multiset *const _hash_multiset)
{
    if (_hash_multiset == NULL)
    {
        return 0.0f;
    }

    return _hash_multiset->max_load_factor;
}
