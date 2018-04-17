#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_multiset.h"

// Проверка возвращаемых значений не выполняется для упрощения.

// Функция генерации хэша по строке.
size_t hash_s(const void *const _data)
{
    if (_data == NULL)return 0;
    const char *c = (char*)_data;
    size_t hash = 0;
    while (*c != 0)
    {
        hash += *(c++);
    }
    return hash;
}

// Функция детального сравнения строк.
size_t comp_s(const void *const _a,
              const void *const _b)
{
    if ( (_a == NULL) || (_b == NULL) )
    {
        return 0;
    }

    const char *const a = (char*)_a;
    const char *const b = (char*)_b;

    if (strcmp(a, b) == 0)
    {
        return 1;
    }

    return 0;
}

// Функция вывода строки.
void print_s(const void *const _data)
{
    if (_data == NULL) return;
    const char *const data = (char*)_data;
    printf("%s\n", data);
    return;
}

int main(int argc, char **argv)
{
    // Создание хэш-мультимножества.
    c_hash_multiset *hash_multiset = c_hash_multiset_create(hash_s,
                                                            comp_s,
                                                            11,
                                                            0.5f);

    // Вставка в хэш-мультимножества нескольких элементов.
    // Некоторые повторяются.
    const char *const string_a = "Good";
    const char *const string_b = "Wall";
    const char *const string_c = "Area";

    c_hash_multiset_insert(hash_multiset, string_a);
    c_hash_multiset_insert(hash_multiset, string_b);
    c_hash_multiset_insert(hash_multiset, string_b);
    c_hash_multiset_insert(hash_multiset, string_c);
    c_hash_multiset_insert(hash_multiset, string_c);
    c_hash_multiset_insert(hash_multiset, string_c);

    // Вывод содержимого хэш-мультимножества.
    c_hash_multiset_for_each(hash_multiset, print_s);
    printf("\n");

    // Удаление одного (уникального всвоем роде) элемента.
    c_hash_multiset_erase(hash_multiset, string_a, NULL);

    // Вывод содержимого хэш-мультимножества.
    c_hash_multiset_for_each(hash_multiset, print_s);
    printf("\n");

    // Удаление всех копий заданного элемента.
    c_hash_multiset_erase_all(hash_multiset, string_c, NULL);

    // Вывод содержимого хэш-мультимножества.
    c_hash_multiset_for_each(hash_multiset, print_s);
    printf("\n");

    // Проверка наличия и количества элементов в хэш-мультимножестве.
    ptrdiff_t have;
    size_t count;

    have = c_hash_multiset_check(hash_multiset, string_a);
    count = c_hash_multiset_count(hash_multiset, string_a);
    printf("string_a have/count: %Iu/%Iu\n", have, count);

    have = c_hash_multiset_check(hash_multiset, string_b);
    count = c_hash_multiset_count(hash_multiset, string_b);
    printf("string_b have/count: %Iu/%Iu\n", have, count);

    have = c_hash_multiset_check(hash_multiset, string_c);
    count = c_hash_multiset_count(hash_multiset, string_c);
    printf("string_c have/count: %Iu/%Iu\n", have, count);


    // Удаление хэш-мультимножества.
    c_hash_multiset_delete(hash_multiset, NULL);

    getchar();
    return 0;
}
