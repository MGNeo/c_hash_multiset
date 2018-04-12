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

int main()
{
    c_hash_multiset *hash_multiset = c_hash_multiset_create(hash_func_s,
                                                            comp_func_s,
                                                            sizeof(char*),
                                                            1024,
                                                            0.5f);

    c_hash_multiset_delete(hash_multiset, NULL);

    c_hash_multiset_resize(hash_multiset, 0);

    c_hash_multiset_resize(hash_multiset, 1024);

    // ...

    getchar();
    return 0;
}

// Функция c_hash_multiset_count() должна возвращать количество элементов с заданным ключом.
