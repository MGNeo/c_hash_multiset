#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_hash_multiset.h"

// Функция генерации хэша по элементу-строке.
size_t hash_data_s(const void *const _data)
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

// Функция детального сравнения элементов-строк.
size_t comp_data_s(const void *const _key_a,
                   const void *const _key_b)
{
    if ( (_key_a == NULL) || (_key_b == NULL) )
    {
        return 0;
    }

    const char *const key_a = (char*)_key_a;
    const char *const key_b = (char*)_key_b;

    if (strcmp(key_a, key_b) == 0)
    {
        return 1;
    }

    return 0;
}

// Функция вывода элементов-строки.
void print_data_s(const void *const _data)
{
    if (_data == NULL) return;
    const char *const data = (char*)_data;
    printf("%s\n", data);
    return;
}

int main(int argc, char **argv)
{
    size_t error;
    c_hash_multiset *hash_multiset;

    // Попытаемся создать хэш-мультимножество.
    hash_multiset = c_hash_multiset_create(hash_data_s, comp_data_s, 10, 0.5f, &error);
    // Если возникла ошибка, покажем ее.
    if (hash_multiset == NULL)
    {
        printf("create error: %Iu\n", error);
        printf("Program end.\n");
        getchar();
        return -1;
    }

    // Вставим в хэш-мультимножество несколько элементов.
    const char *const string_1 = "One";
    const char *const string_2 = "Two";
    const char *const string_3 = "Three";
    {
        const ptrdiff_t r_code = c_hash_multiset_insert(hash_multiset, string_1);
        // Покажем результат операции.
        printf("insert[%s]: %Id\n", string_1, r_code);
    }
    {
        const ptrdiff_t r_code = c_hash_multiset_insert(hash_multiset, string_2);
        // Покажем результат операции.
        printf("insert[%s]: %Id\n", string_2, r_code);
    }
    {
        const ptrdiff_t r_code = c_hash_multiset_insert(hash_multiset, string_3);
        // Покажем результат операции.
        printf("insert[%s]: %Id\n", string_3, r_code);
    }
    {
        const ptrdiff_t r_code = c_hash_multiset_insert(hash_multiset, string_1);
        // Покажем результат операции.
        printf("insert[%s]: %Id\n", string_1, r_code);
    }

    // При помощи обхода хэш-мультимножества выведем содержимое каждого элемента.
    {
        const ptrdiff_t r_code = c_hash_multiset_for_each(hash_multiset, print_data_s);
        // Если возникла ошибка, покажем причину.
        if (r_code < 0)
        {
            printf("for each error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -2;
        }
    }

    // Удалим элемент string_2.
    {
        const ptrdiff_t r_code = c_hash_multiset_erase(hash_multiset, string_2, NULL);
        // Покажем результат операции.
        printf("erase[%s]: %Id\n", string_2, r_code);
    }

    // При помощи обхода хэш-мультимножества выведем содержимое каждого элемента.
    {
        const ptrdiff_t r_code = c_hash_multiset_for_each(hash_multiset, print_data_s);
        // Если возникла ошибка, покажем причину.
        if (r_code < 0)
        {
            printf("for each error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -3;
        }
    }

    // Удалим все string_1.
    {
        error = 0;
        const size_t d_count = c_hash_multiset_erase_all(hash_multiset, string_1, NULL, &error);
        // Если возникла ошибка, покажем ее.
        if ( (d_count == 0) && (error > 0) )
        {
            printf("erase all error: %Iu\n", error);
            printf("Program end.\n");
            getchar();
            return -4;
        }
        // Покажем, сколько элементов было удалено.
        printf("erase all[%s]: %Iu\n", string_1, d_count);
    }

    // При помощи обхода хэш-мультимножества выведем содержимое каждого элемента.
    {
        const ptrdiff_t r_code = c_hash_multiset_for_each(hash_multiset, print_data_s);
        // Если возникла ошибка, покажем причину.
        if (r_code < 0)
        {
            printf("for each error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -5;
        }
    }

    // Удалим хэш-мультимножество.
    {
        const ptrdiff_t r_code = c_hash_multiset_delete(hash_multiset, NULL);
        // Если возникла ошибка, покажем причину.
        if (r_code < 0)
        {
            printf("delete error, r_code: %Id\n", r_code);
            printf("Program end.\n");
            getchar();
            return -6;
        }
    }

    getchar();
    return 0;
}
