#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_multiset.h"

size_t hash_func_s(const void *const _data)
{
    const char *c = *((char**)_data);
    size_t hash = 0;
    while(*c != 0)
    {
        hash += *(c++);
    }

    return hash;
}

size_t comp_func_s(const void *const _a,
                   const void *const _b)
{
    const char *const a = *((char**)_a);
    const char *const b = *((char**)_b);

    if (strcmp(a, b) == 0)
    {
        return 1;
    }

    return 0;
}

void print_func_s(const void *const _data)
{
    const char *const data = *((char**)_data);
    printf("%s\n", data);
    return;
}

void print_hash_multiset(const c_hash_multiset *const _hash_multiset)
{
    if (_hash_multiset == NULL) return;

    printf("slots_count: %Iu\nnodes_count: %Iu\nunique_count: %Iu\nslots: %Iu\n",
           _hash_multiset->slots_count,
           _hash_multiset->nodes_count,
           _hash_multiset->unique_count,
           _hash_multiset->slots);

    printf("elements:\n");
    c_hash_multiset_for_each(_hash_multiset, print_func_s);
    printf("\n\n");
    return;
}

int main()
{
    // Создание хэш-мультимножества.
    c_hash_multiset *hash_multiset = c_hash_multiset_create(hash_func_s,
                                                            comp_func_s,
                                                            sizeof(char*),
                                                            11,
                                                            0.5f);

    const char *const data_one = "data one";
    const char *const data_two = "data two";
    const char *const data_three = "data three";

    // Вставляем много данных.
    c_hash_multiset_insert(hash_multiset, &data_one);
    c_hash_multiset_insert(hash_multiset, &data_two);
    c_hash_multiset_insert(hash_multiset, &data_three);
    c_hash_multiset_insert(hash_multiset, &data_one);
    c_hash_multiset_insert(hash_multiset, &data_one);

    // Печать содержимого.
    print_hash_multiset(hash_multiset);

    // Удаляем все экземпляры data_one.
    c_hash_multiset_erase_all(hash_multiset, &data_one, NULL);

    // Печать содержимого.
    print_hash_multiset(hash_multiset);

    // Вырезаем один элемент.
    c_hash_multiset_erase(hash_multiset, &data_two, NULL);

    // Печать содержимого.
    print_hash_multiset(hash_multiset);

    // Проверка на наличие заданных данных.
    printf("check data_three: %Id\n",
           c_hash_multiset_check(hash_multiset, &data_three));

    const char *const data_four = "data four";
    printf("check data_four: %Id\n",
           c_hash_multiset_check(hash_multiset, &data_four));

    // Удаление хэш-мультимножества.
    c_hash_multiset_delete(hash_multiset, NULL);

    getchar();
    return 0;
}
