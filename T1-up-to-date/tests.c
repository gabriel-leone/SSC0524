#include "tests.h"
#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

struct cc_array_s
{
  size_t size;
  size_t capacity;
  float exp_factor;
  void **buffer;
  void *(*mem_alloc)(size_t size);
  void *(*mem_calloc)(size_t blocks, size_t size);
  void (*mem_free)(void *block);
};

#define DEFAULT_CAPACITY 8
#define DEFAULT_EXPANSION_FACTOR 2

#define expand_capacity mock_expand_capacity

// Mock for expand_capacity to simulate various behaviors
static bool simulate_expand_failure = false;

static enum cc_stat mock_expand_capacity(CC_Array *ar) {
    if (simulate_expand_failure) {
        return CC_ERR_ALLOC; // Simulate allocation failure
    }
    // Normal behavior (placeholder, adjust as necessary)
    void **new_buffer = realloc(ar->buffer, ar->capacity * 2 * sizeof(void *));
    if (!new_buffer) {
        return CC_ERR_ALLOC;
    }
    ar->buffer = new_buffer;
    ar->capacity *= 2;
    return CC_OK;
}

// Mocks for memory allocation functions
static void* mock_alloc(size_t size) {
    return malloc(size);
}

static void* mock_calloc(size_t blocks, size_t size) {
    return calloc(blocks, size);
}

static void mock_free(void *block) {
    free(block);
}

bool test_cc_array_new_conf_valid_conf() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;

    ASSERT_CC_OK(cc_array_new_conf(&conf, &a));
    ASSERT_EQ(a->capacity, 8);
    ASSERT_EQ(a->exp_factor, 2.0);
    ASSERT_EQ(a->mem_alloc, mock_alloc);
    ASSERT_EQ(a->mem_calloc, mock_calloc);
    ASSERT_EQ(a->mem_free, mock_free);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_new_conf_exp_factor_default() {
    CC_ArrayConf conf = {8, 1.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;

    ASSERT_CC_OK(cc_array_new_conf(&conf, &a));
    ASSERT_EQ(a->exp_factor, DEFAULT_EXPANSION_FACTOR);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_new_conf_invalid_capacity_zero() {
    CC_ArrayConf conf = {0, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;

    ASSERT_EQ(cc_array_new_conf(&conf, &a), CC_ERR_INVALID_CAPACITY);
    return true;
}

bool test_cc_array_new_conf_invalid_capacity_large_exp_factor() {
    CC_ArrayConf conf = {8, FLT_MAX, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;

    ASSERT_EQ(cc_array_new_conf(&conf, &a), CC_ERR_INVALID_CAPACITY);
    return true;
}

bool test_cc_array_new_conf_alloc_failure_array() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, NULL, mock_free}; // NULL calloc will cause failure
    CC_Array *a = NULL;

    ASSERT_EQ(cc_array_new_conf(&conf, &a), CC_ERR_ALLOC);
    return true;
}

bool test_cc_array_new_conf_alloc_failure_buffer() {
    CC_ArrayConf conf = {8, 2.0, NULL, mock_calloc, mock_free}; // NULL alloc will cause failure
    CC_Array *a = NULL;

    ASSERT_EQ(cc_array_new_conf(&conf, &a), CC_ERR_ALLOC);
    return true;
}

bool test_cc_array_add_with_sufficient_capacity() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    
    ASSERT_CC_OK(cc_array_add(a, (void*)1));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);
    
    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_with_expansion() {
    CC_ArrayConf conf = {1, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    
    ASSERT_CC_OK(cc_array_add(a, (void*)1));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);
    
    ASSERT_CC_OK(cc_array_add(a, (void*)2));
    ASSERT_EQ(a->size, 2);
    ASSERT_EQ(a->buffer[1], (void*)2);
    
    cc_array_destroy(a);
    return true;
}

static enum cc_stat expand_capacity(CC_Array *ar);

bool test_cc_array_add_expand_failure() {
    CC_ArrayConf conf = {1, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    simulate_expand_failure = true; // Simulate failure

    ASSERT_CC_OK(cc_array_add(a, (void*)1));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);

    ASSERT_EQ(cc_array_add(a, (void*)2), CC_ERR_ALLOC);
    ASSERT_EQ(a->size, 1); // Size should not change

    simulate_expand_failure = false; // Reset to normal behavior

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_max_capacity() {
    CC_ArrayConf conf = {1, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Manually set size to maximum capacity
    a->size = CC_MAX_ELEMENTS;
    
    ASSERT_EQ(cc_array_add(a, (void*)1), CC_ERR_MAX_CAPACITY);
    ASSERT_EQ(a->size, CC_MAX_ELEMENTS); // Size should not change
    
    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_empty_array_start() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)1, 0));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_empty_array_end() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)1, 0));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)2, 1));
    ASSERT_EQ(a->size, 2);
    ASSERT_EQ(a->buffer[1], (void*)2);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_empty_array_middle() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_EQ(cc_array_add_at(a, (void*)1, 1), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_out_of_range_upper() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)1, 0));
    ASSERT_EQ(cc_array_add_at(a, (void*)2, 2), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_out_of_range_lower() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_EQ(cc_array_add_at(a, (void*)1, -1), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_with_sufficient_capacity() {
    CC_ArrayConf conf = {1, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)1, 0));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(a->buffer[0], (void*)1);

    ASSERT_CC_OK(cc_array_add_at(a, (void*)2, 1));
    ASSERT_EQ(a->size, 2);
    ASSERT_EQ(a->buffer[1], (void*)2);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_add_at_expand_failure() {
    CC_ArrayConf conf = {1, 2.0, mock_alloc, mock_calloc, mock_free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    simulate_expand_failure = true; // Simulate failure

    ASSERT_EQ(cc_array_add_at(a, (void*)1, 0), CC_ERR_ALLOC);

    simulate_expand_failure = false; // Reset to normal behavior

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_existing_element() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element_to_remove = 42;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element_to_remove));

    int removed_element;
    ASSERT_CC_OK(cc_array_remove(a, (void*)&element_to_remove, (void**)&removed_element));
    ASSERT_EQ(removed_element, 42);
    ASSERT_EQ(a->size, 0);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_nonexistent_element() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element_to_remove = 42;
    ASSERT_EQ(cc_array_remove(a, (void*)&element_to_remove, NULL), CC_ERR_VALUE_NOT_FOUND);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_from_empty_array() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element_to_remove = 42;
    ASSERT_EQ(cc_array_remove(a, (void*)&element_to_remove, NULL), CC_ERR_VALUE_NOT_FOUND);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_from_array_with_multiple_elements() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    int element2 = 2;
    int element3 = 3;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element2));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element3));

    ASSERT_CC_OK(cc_array_remove(a, (void*)&element2, NULL));
    ASSERT_EQ(a->size, 2);
    ASSERT_EQ(*(int*)a->buffer[0], 1);
    ASSERT_EQ(*(int*)a->buffer[1], 3);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_last_element() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    int element2 = 2;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element2));

    ASSERT_CC_OK(cc_array_remove(a, (void*)&element2, NULL));
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(*(int*)a->buffer[0], 1);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_at_valid_index() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    int element2 = 2;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element2));

    void* removed_element;
    ASSERT_CC_OK(cc_array_remove_at(a, 1, &removed_element));
    ASSERT_EQ(*(int*)removed_element, 2);
    ASSERT_EQ(a->size, 1);
    ASSERT_EQ(*(int*)a->buffer[0], 1);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_remove_at_index_out_of_range() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    ASSERT_EQ(cc_array_remove_at(a, 1, NULL), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

void* mock_alloc_failure(size_t size) {
    // Forçar uma falha na alocação de memória retornando NULL
    return NULL;
}

bool test_cc_array_subarray_buffer_allocation_failure() {
    CC_ArrayConf conf = {8, 2.0, mock_alloc_failure, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    ASSERT_EQ(cc_array_add(a, (void*)&element1), CC_OK);

    CC_Array *subarray = NULL;
    ASSERT_EQ(cc_array_subarray(a, 0, 0, &subarray), CC_ERR_ALLOC);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_subarray_buffer_allocation_success() {
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    int element2 = 2;
    int element3 = 3;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element2));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element3));

    CC_Array *subarray = NULL;
    ASSERT_EQ(cc_array_subarray(a, 0, 1, &subarray), CC_OK);
    ASSERT_EQ(subarray->size, 2);
    ASSERT_EQ(*(int*)subarray->buffer[0], 1);
    ASSERT_EQ(*(int*)subarray->buffer[1], 2);

    cc_array_destroy(subarray);
    cc_array_destroy(a);
    return true;
}

// Função de filtro simples que mantém apenas números pares
bool pred1(const void *element) {
    int value = *((int*)element);
    return value % 2 == 0;
}

// Função de filtro que sempre retorna verdadeiro (nenhum elemento é removido)
bool pred2(const void *element) {
    (void)element; // Evitar aviso de não utilizado
    return true;
}

// Função de filtro que sempre retorna verdadeiro (nenhum elemento é removido)
bool pred3(const void *element) {
    (void)element; // Evitar aviso de não utilizado
    return true;
}

bool test_cc_array_filter_mut_non_empty_array() {
    // Criar e preencher um CC_Array com alguns elementos
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    int element2 = 2;
    int element3 = 3;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element2));
    ASSERT_CC_OK(cc_array_add(a, (void*)&element3));



    // Chamar a função de filtro_mut e verificar se o tamanho do CC_Array permanece inalterado
    ASSERT_EQ(cc_array_filter_mut(a, pred1), CC_OK);
    ASSERT_EQ(cc_array_size(a), 2); // Deve permanecer inalterado

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_filter_mut_empty_array() {
    // Criar um CC_Array vazio
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Chamar a função de filtro_mut em um array vazio e verificar se retorna CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(cc_array_filter_mut(a, pred2), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_filter_mut_single_element_array() {
    // Criar um CC_Array com um único elemento
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int element1 = 1;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element1));

    // Chamar a função de filtro_mut em um array com um único elemento e verificar se retorna CC_OK
    ASSERT_EQ(cc_array_filter_mut(a, pred3), CC_OK);
    ASSERT_EQ(cc_array_size(a), 1); // O tamanho do array deve permanecer inalterado

    cc_array_destroy(a);
    return true;
}

// Predicado: manter apenas números pares
bool pred4(const void *element) {
    return (*(int*)element) % 2 == 0;
}

bool test_cc_array_filter_non_empty_array() {
    // Criar um CC_Array não vazio
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    int elements[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); ++i) {
        ASSERT_CC_OK(cc_array_add(a, (void*)&elements[i]));
    }

    // Chamar a função de filtro e verificar se retorna CC_OK
    CC_Array *filtered = NULL;
    ASSERT_EQ(cc_array_filter(a, pred4, &filtered), CC_OK);
    ASSERT_EQ(cc_array_size(filtered), 2); // Apenas 2 números pares devem ser mantidos

    // Verificar se os elementos filtrados estão corretos
    void *result = NULL;
    ASSERT_CC_OK(cc_array_get_at(filtered, 0, &result));
    ASSERT_EQ(*(int*)result, 2);
    ASSERT_CC_OK(cc_array_get_at(filtered, 1, &result));
    ASSERT_EQ(*(int*)result, 4);

    cc_array_destroy(a);
    cc_array_destroy(filtered);
    return true;
}

// Predicado: manter apenas números pares
bool pred5(const void *element) {
    (void)element; // Evitar aviso de não utilizado
    return false;
}

bool test_cc_array_filter_empty_array() {
    // Criar um CC_Array vazio
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);



    // Chamar a função de filtro em um array vazio e verificar se retorna CC_ERR_OUT_OF_RANGE
    CC_Array *filtered = NULL;
    ASSERT_EQ(cc_array_filter(a, pred5, &filtered), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return true;
}

// Predicado: manter apenas números ímpares
bool pred6(const void *element) {
    return (*(int*)element) % 2 != 0;
}

bool test_cc_array_filter_single_element() {
    // Criar um CC_Array com um único elemento
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int element = 5;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element));

    // Chamar a função de filtro e verificar se retorna CC_OK
    CC_Array *filtered = NULL;
    ASSERT_EQ(cc_array_filter(a, pred6, &filtered), CC_OK);
    ASSERT_EQ(cc_array_size(filtered), 1); // Apenas 1 número ímpar deve ser mantido

    // Verificar se o elemento filtrado está correto
    void *result = NULL;
    ASSERT_CC_OK(cc_array_get_at(filtered, 0, &result));
    ASSERT_EQ(*(int*)result, 5);

    cc_array_destroy(a);
    cc_array_destroy(filtered);
    return true;
}

bool test_cc_array_reverse_non_empty_array() {
    // Criar um CC_Array com elementos
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int elements[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*)&elements[i]));
    }

    // Reverter a ordem dos elementos
    cc_array_reverse(a);

    // Verificar se a ordem dos elementos foi revertida corretamente
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++) {
        void *result = NULL;
        ASSERT_CC_OK(cc_array_get_at(a, i, &result));
        ASSERT_EQ(*(int*)result, elements[sizeof(elements) / sizeof(elements[0]) - i - 1]);
    }

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_reverse_empty_array() {
    // Criar um CC_Array vazio
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Reverter a ordem dos elementos
    cc_array_reverse(a);

    // Verificar se não houve erro
    ASSERT_EQ(cc_array_size(a), 0);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_reverse_single_element() {
    // Criar um CC_Array com um único elemento
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int element = 5;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element));

    // Reverter a ordem dos elementos
    cc_array_reverse(a);

    // Verificar se não houve erro e se o elemento único permanece o mesmo
    ASSERT_EQ(cc_array_size(a), 1);
    void *result = NULL;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &result));
    ASSERT_EQ(*(int*)result, element);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_trim_capacity_capacity_greater_than_size() {
    // Criar um CC_Array com capacidade maior que o tamanho
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int elements[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < sizeof(elements) / sizeof(elements[0]); i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*)&elements[i]));
    }

    // Reduzir a capacidade para corresponder ao tamanho
    ASSERT_CC_OK(cc_array_trim_capacity(a));

    // Verificar se a capacidade foi reduzida corretamente
    ASSERT_EQ(cc_array_capacity(a), cc_array_size(a));

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_trim_capacity_capacity_equal_to_size() {
    // Criar um CC_Array com capacidade igual ao tamanho
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int element = 5;
    ASSERT_CC_OK(cc_array_add(a, (void*)&element));

    // Reduzir a capacidade para corresponder ao tamanho
    ASSERT_CC_OK(cc_array_trim_capacity(a));

    // Verificar se a capacidade foi reduzida corretamente
    ASSERT_EQ(cc_array_capacity(a), cc_array_size(a));

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_trim_capacity_empty_array() {
    // Criar um CC_Array vazio
    CC_ArrayConf conf = {8, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Reduzir a capacidade para corresponder ao tamanho (deve permanecer 1)
    ASSERT_CC_OK(cc_array_trim_capacity(a));

    // Verificar se a capacidade foi reduzida corretamente
    ASSERT_EQ(cc_array_capacity(a), 1);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_trim_capacity_size_greater_than_capacity() {
    // Criar um CC_Array com capacidade menor que o tamanho
    CC_ArrayConf conf = {3, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Adicionar elementos ao array até que a capacidade seja excedida
    for (size_t i = 0; i < 5; i++) {
        int element = i;
        ASSERT_CC_OK(cc_array_add(a, (void*)&element));
    }

    // Reduzir a capacidade para corresponder ao tamanho
    ASSERT_CC_OK(cc_array_trim_capacity(a));

    // Verificar se a capacidade foi reduzida corretamente
    ASSERT_EQ(cc_array_capacity(a), cc_array_size(a));

    cc_array_destroy(a);
    return true;
}

// Definição da função de soma
void sum_fn(void *a, void *b, void *result) {
    int *num_a = (int *)a;
    int *num_b = (int *)b;
    int *res = (int *)result;

    if (b == NULL) {
        // Caso em que o array tem tamanho 1
        *res = *num_a;
    } else {
        // Caso em que o array tem tamanho maior que 1
        *res = *num_a + *num_b;
    }
}

bool test_cc_array_reduce_size_0() {
    // Criar um CC_Array vazio
    CC_ArrayConf conf = {1, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);

    // Definir um resultado inicial
    int result = 0;

    // Reduzir o array
    cc_array_reduce(a, sum_fn, &result);

    // Verificar se o resultado não foi alterado
    ASSERT_EQ(result, 0);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_reduce_size_1() {
    // Criar um CC_Array com um elemento
    CC_ArrayConf conf = {1, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    int element = 5;
    cc_array_add(a, &element);

    // Definir um resultado inicial
    int result = 0;

    // Reduzir o array
    cc_array_reduce(a, sum_fn, &result);

    // Verificar se o resultado é igual ao elemento único
    ASSERT_EQ(result, element);

    cc_array_destroy(a);
    return true;
}

bool test_cc_array_reduce_size_greater_than_1() {
    // Criar um CC_Array com vários elementos
    CC_ArrayConf conf = {5, 2.0, malloc, calloc, free};
    CC_Array *a = NULL;
    cc_array_new_conf(&conf, &a);
    for (int i = 1; i <= 5; i++) {
        cc_array_add(a, &i);
    }

    // Definir um resultado inicial
    int result = 0;

    // Reduzir o array
    cc_array_reduce(a, sum_fn, &result);

    // Verificar se o resultado é a soma dos elementos
    ASSERT_EQ(result, 15);

    cc_array_destroy(a);
    return true;
}

// Função de teste para cc_array_reduce com tamanho igual a 1
bool test_cc_array_reduce_size_equal_1() {
    CC_Array *a = (CC_Array *)malloc(sizeof(CC_Array));
    memset(a, 0, sizeof(CC_Array));
    a->size = 1;
    int element = 5;
    a->buffer = malloc(sizeof(int));
    *(int*)(a->buffer) = element;

    int result = 0;
    cc_array_reduce(a, sum_fn, &result);

    cc_array_destroy(a);

    // Verificar se o resultado é igual ao elemento único
    return result == element;
}

// Função de teste para cc_array_reduce com tamanho igual a 2
bool test_cc_array_reduce_size_equal_2() {
    CC_Array *a = (CC_Array *)malloc(sizeof(CC_Array));
    memset(a, 0, sizeof(CC_Array));
    a->size = 2;
    int elements[] = {1, 2};
    a->buffer = malloc(sizeof(int) * 2);
    memcpy(a->buffer, elements, sizeof(int) * 2);

    int result = 0;
    cc_array_reduce(a, sum_fn, &result);

    cc_array_destroy(a);

    // Verificar se o resultado é a soma dos elementos
    return result == 3;
}

// Função de teste para cc_array_reduce com tamanho maior que 2
bool test_cc_array_reduce_size_greater_than_2() {
    CC_Array *a = (CC_Array *)malloc(sizeof(CC_Array));
    memset(a, 0, sizeof(CC_Array));
    a->size = 5;
    int elements[] = {1, 2, 3, 4, 5};
    a->buffer = malloc(sizeof(int) * 5);
    memcpy(a->buffer, elements, sizeof(int) * 5);

    int result = 0;
    cc_array_reduce(a, sum_fn, &result);

    cc_array_destroy(a);

    // Verificar se o resultado é a soma dos elementos
    return result == 15;
}


bool test_cc_array_iter_replace_same_element() {
    // Caso em que o novo elemento é o mesmo que o elemento atual
    CC_Array* array;
    CC_ArrayIter iter;
    ASSERT_CC_OK(cc_array_new(&array));
    ASSERT_CC_OK(cc_array_add(array, (void*)1));
    cc_array_iter_init(&iter, array);
    void* ignored;
    ASSERT_CC_OK(cc_array_iter_next(&iter, &ignored)); // Move para o primeiro elemento
    void* replaced_element;
    // Substitui o elemento atual pelo mesmo elemento
    enum cc_stat status = cc_array_iter_replace(&iter, (void*)1, &replaced_element);
    ASSERT_EQ(status, CC_OK);
    ASSERT_EQ(*((int*)replaced_element), 1);
    cc_array_destroy(array);
    return true;
}

bool test_cc_array_iter_replace_different_element() {
    // Caso em que o novo elemento é diferente do elemento atual
    CC_Array* array;
    CC_ArrayIter iter;
    ASSERT_CC_OK(cc_array_new(&array));
    ASSERT_CC_OK(cc_array_add(array, (void*)1));
    cc_array_iter_init(&iter, array);
    void* ignored;
    ASSERT_CC_OK(cc_array_iter_next(&iter, &ignored)); // Move para o primeiro elemento
    void* replaced_element;
    // Substitui o elemento atual por um elemento diferente
    enum cc_stat status = cc_array_iter_replace(&iter, (void*)2, &replaced_element);
    ASSERT_EQ(status, CC_OK);
    ASSERT_EQ(*((int*)replaced_element), 1);
    cc_array_destroy(array);
    return true;
}

bool test_cc_array_iter_replace_out_of_range() {
    // Caso em que o iterador está em uma posição inválida
    CC_Array* array;
    CC_ArrayIter iter;
    ASSERT_CC_OK(cc_array_new(&array));
    ASSERT_CC_OK(cc_array_add(array, (void*)1));
    cc_array_iter_init(&iter, array);
    // Não chamamos cc_array_iter_next(), então o iterador está em um estado inválido
    void* replaced_element;
    // Tentativa de substituir o elemento com um iterador inválido
    enum cc_stat status = cc_array_iter_replace(&iter, (void*)2, &replaced_element);
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);
    cc_array_destroy(array);
    return true;
}

// Teste para remoção bem-sucedida do último par de elementos de duas arrays quando ambas as arrays têm elementos
bool test_cc_array_zip_iter_remove_success() {
    // Criar duas arrays com elementos
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);
    cc_array_add(ar1, (void*)1);
    cc_array_add(ar2, (void*)2);

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avançar o iterador para o último par de elementos
    while (cc_array_zip_iter_next(&iter, NULL, NULL) == CC_OK);

    // Remover o último par de elementos
    int out1_value, out2_value;
    enum cc_stat status = cc_array_zip_iter_remove(&iter, (void **)&out1_value, (void **)&out2_value);

    // Verificar se a remoção foi bem-sucedida
    bool result = (status == CC_OK && out1_value == 1 && out2_value == 2);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Teste para remoção bem-sucedida do último par de elementos de duas arrays quando ambas as arrays têm apenas um elemento
bool test_cc_array_zip_iter_remove_success_one_element() {
    // Criar duas arrays com um elemento cada
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);
    cc_array_add(ar1, (void*)1);
    cc_array_add(ar2, (void*)2);

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Remover o único par de elementos
    int out1_value, out2_value;
    enum cc_stat status = cc_array_zip_iter_remove(&iter, (void **)&out1_value, (void **)&out2_value);

    // Verificar se a remoção foi bem-sucedida
    bool result = (status == CC_OK && out1_value == 1 && out2_value == 2);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}


// Teste para remoção bem-sucedida do último par de elementos de duas arrays quando uma das arrays tem tamanho 0
bool test_cc_array_zip_iter_remove_success_empty_array() {
    // Criar uma array com um elemento e outra array vazia
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);
    cc_array_add(ar1, (void*)1);

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avançar o iterador para o último par de elementos
    while (cc_array_zip_iter_next(&iter, NULL, NULL) == CC_OK);

    // Remover o último par de elementos
    void *out1, *out2;
    enum cc_stat status = cc_array_zip_iter_remove(&iter, &out1, &out2);

    // Verificar se a remoção foi bem-sucedida
    int out1_value = *((int *)out1);
    bool result = (status == CC_OK && out1_value == 1 && out2 == NULL);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Teste para tentativa de remoção quando o iterador está fora dos limites de uma das arrays
bool test_cc_array_zip_iter_remove_out_of_range() {
    // Criar duas arrays vazias
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Tentar remover o último par de elementos
    void *out1, *out2;
    enum cc_stat status = cc_array_zip_iter_remove(&iter, &out1, &out2);

    // Verificar se a remoção falhou devido ao iterador estar fora dos limites de uma das arrays
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Teste para adição bem-sucedida de um novo par de elementos às arrays
bool test_cc_array_zip_iter_add_success() {
    // Criar duas arrays vazias
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Adicionar um novo par de elementos
    int e1 = 1, e2 = 2;
    enum cc_stat status = cc_array_zip_iter_add(&iter, &e1, &e2);

    // Verificar se a adição foi bem-sucedida
    void *element1, *element2;
    cc_array_get_at(ar1, 0, &element1);
    cc_array_get_at(ar2, 0, &element2);
    bool result = (status == CC_OK && ar1->size == 1 && ar2->size == 1 &&
                   *((int *)element1) == 1 && *((int *)element2) == 2);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Teste para falha ao adicionar um novo par de elementos às arrays devido à alocação de memória falhar
bool test_cc_array_zip_iter_add_failure_alloc() {
    // Criar duas arrays vazias
    CC_Array *ar1, *ar2;
    cc_array_new(&ar1);
    cc_array_new(&ar2);

    // Simular falha de alocação de memória
    ar1->capacity = 0;
    ar2->capacity = 0;

    // Inicializar o iterador
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Tentar adicionar um novo par de elementos
    int e1 = 1, e2 = 2;
    enum cc_stat status = cc_array_zip_iter_add(&iter, &e1, &e2);

    // Verificar se a adição falhou devido à falha de alocação de memória
    bool result = (status == CC_ERR_ALLOC && ar1->size == 0 && ar2->size == 0);

    // Destruir as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Testa a substituição bem-sucedida de um par de elementos nas arrays
bool test_cc_array_zip_iter_replace_success() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1))
    ASSERT_CC_OK(cc_array_new(&ar2))

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1))
    ASSERT_CC_OK(cc_array_add(ar2, (void *)2))

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    ASSERT_CC_OK(cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2));

    // Verifica se os elementos substituídos são os esperados
    bool result = (replaced1 == (void *)1 && replaced2 == (void *)2);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Testa a substituição falha de um par de elementos nas arrays devido a um índice inválido
bool test_cc_array_zip_iter_replace_failure_out_of_range() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1))
    ASSERT_CC_OK(cc_array_new(&ar2))

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1))
    ASSERT_CC_OK(cc_array_add(ar2, (void *)2))

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Move o iterador para o final do array
    while (cc_array_zip_iter_next(&iter, NULL, NULL) == CC_OK);

    // Tenta substituir o último par de elementos retornados (índice fora do alcance)
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se a substituição falhou devido a um índice inválido
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Função de teste para substituição falha de um par de elementos nas arrays devido a um índice inválido
bool test_cc_array_zip_iter_replace_failure_invalid_iterator() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1))
    ASSERT_CC_OK(cc_array_new(&ar2))

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1))
    ASSERT_CC_OK(cc_array_add(ar2, (void *)2))

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Tenta substituir o último par de elementos retornados sem avançar o iterador
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se a substituição falhou devido ao iterador inválido
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Testa a substituição bem-sucedida de um par de elementos nas arrays, verificando os elementos substituídos
bool test_cc_array_zip_iter_replace_success_check_replacement_elements() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1))
    ASSERT_CC_OK(cc_array_new(&ar2))

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1))
    ASSERT_CC_OK(cc_array_add(ar2, (void *)2))

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    ASSERT_CC_OK(cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2));

    // Verifica se os elementos substituídos são os esperados
    bool result = (replaced1 == (void *)1 && replaced2 == (void *)2);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

// Testa a substituição bem-sucedida de um par de elementos nas arrays, verificando os elementos substituídos
bool test_cc_array_zip_iter_replace_success_check_elements() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1))
    ASSERT_CC_OK(cc_array_new(&ar2))

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1))
    ASSERT_CC_OK(cc_array_add(ar2, (void *)2))

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    ASSERT_CC_OK(cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2));

    // Verifica se os elementos substituídos são os esperados
    bool result = (replaced1 == (void *)1 && replaced2 == (void *)2);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return result;
}

bool test_cc_array_new_conf_exp_factor_equal_1() {
    CC_ArrayConf conf;
    conf.exp_factor = 1;  // Set exp_factor to 1
    conf.capacity = 10;   // Set capacity to any value
    conf.mem_alloc = malloc;
    conf.mem_calloc = calloc;
    conf.mem_free = free;

    CC_Array *ar;
    enum cc_stat status = cc_array_new_conf(&conf, &ar);

    // Verificar se o status retornado é CC_ERR_INVALID_CAPACITY
    // porque o fator de expansão é 1
    return status == CC_ERR_INVALID_CAPACITY && ar == NULL;
}

bool test_cc_array_new_conf_exp_factor_equal_0() {
    CC_ArrayConf conf;
    conf.exp_factor = 0;  // Set exp_factor to 0
    conf.capacity = 10;   // Set capacity to any value
    conf.mem_alloc = malloc;
    conf.mem_calloc = calloc;
    conf.mem_free = free;

    CC_Array *ar;
    enum cc_stat status = cc_array_new_conf(&conf, &ar);

    // Verificar se o status retornado é CC_ERR_INVALID_CAPACITY
    // porque o fator de expansão é 0
    return status == CC_ERR_INVALID_CAPACITY;
}

enum cc_stat test_expand_capacity(CC_Array *ar) {
    return CC_ERR_ALLOC; // Simulate allocation failure
}

bool test_cc_array_add_status_not_greater_than_CC_OK() {
    // Setup array
    CC_Array array;
    array.size = 1;
    array.capacity = 1;
    void *buffer[1];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Try to add an element
    void *element = (void *)2; // Some test element
    enum cc_stat status = cc_array_add(&array, element);

    // Check if the status is CC_ERR_ALLOC, indicating expansion failure
    return status == CC_ERR_ALLOC;
}

bool test_cc_array_add_pre_increment_size() {
    // Setup array
    CC_Array array;
    array.size = 1;
    array.capacity = 2;
    void *buffer[2];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Inicializar elementos no buffer
    array.buffer[0] = (void *)1;
    array.buffer[1] = NULL;

    // Adicionar um elemento que não deve ultrapassar a capacidade inicial
    void *element = (void *)2;
    enum cc_stat status = cc_array_add(&array, element);

    // Verificar se o tamanho foi incrementado corretamente
    bool size_correct = (array.size == 2);
    bool element_correct = (array.buffer[1] == element);

    return status == CC_OK && size_correct && element_correct;
}

bool test_cc_array_add_at_index_out_of_range() {
    // Setup array
    CC_Array array;
    array.size = 2;
    array.capacity = 2;
    void *buffer[2];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Inicializar elementos no buffer
    array.buffer[0] = (void *)1;
    array.buffer[1] = (void *)2;

    // Tentar adicionar um elemento fora do limite
    void *element = (void *)3;
    enum cc_stat status = cc_array_add_at(&array, element, 3); // Index 3 fora do limite

    return status == CC_ERR_OUT_OF_RANGE;
}

bool test_cc_array_add_at_max_index() {
    // Setup array
    CC_Array array;
    array.size = 2;
    array.capacity = 2;
    void *buffer[2];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Inicializar elementos no buffer
    array.buffer[0] = (void *)1;
    array.buffer[1] = (void *)2;

    // Adicionar um elemento na última posição válida
    void *element = (void *)3;
    enum cc_stat status = cc_array_add_at(&array, element, 2); // Index 2 é igual a ar->size

    bool size_correct = (array.size == 3);
    bool element_correct = (array.buffer[2] == element);

    return status == CC_OK && size_correct && element_correct;
}

bool test_cc_array_add_at_with_index_zero_in_empty_array() {
    // Setup array
    CC_Array array;
    array.size = 0;
    array.capacity = 2;
    void *buffer[2];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Tentar adicionar um elemento na posição 0 em uma array vazia
    void *element = (void *)3;
    enum cc_stat status = cc_array_add_at(&array, element, 0); // Index 0 deve ser válido

    return status == CC_OK && array.size == 1 && array.buffer[0] == element;
}

bool test_expand_capacity_called_when_size_greater_than_capacity() {
    // Setup array
    CC_Array array;
    array.size = 5;
    array.capacity = 4; // size > capacity
    void *buffer[4];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Tentar adicionar um elemento
    void *element = (void *)3;
    enum cc_stat status = cc_array_add_at(&array, element, 0);

    // Verificar se a capacidade foi expandida
    return status == CC_OK && array.capacity == 8; // Assumindo que a capacidade foi duplicada
}

bool test_expand_capacity_not_called_when_size_less_than_capacity() {
    // Setup array
    CC_Array array;
    array.size = 2;
    array.capacity = 4; // size < capacity
    void *buffer[4];
    array.buffer = buffer;
    array.mem_alloc = malloc;
    array.mem_calloc = calloc;
    array.mem_free = free;

    // Tentar adicionar um elemento
    void *element = (void *)3;
    enum cc_stat status = cc_array_add_at(&array, element, 0);

    // Verificar se a capacidade não foi expandida
    return status == CC_OK && array.capacity == 4;
}

enum cc_stat test_expand_capacity_positive(CC_Array *ar) {
    return CC_ERR_ALLOC;
}

enum cc_stat test_expand_capacity_negative(CC_Array *ar) {
    ar->capacity *= 2;
    void **new_buffer = realloc(ar->buffer, ar->capacity * sizeof(void *));
    if (!new_buffer)
        return CC_ERR_ALLOC;
    ar->buffer = new_buffer;
    return CC_OK;
}

bool test_cc_array_replace_at_out_of_range_index() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona um elemento ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));

    void *replacement = (void *)42;
    void *replaced_element;

    // Tenta substituir um elemento em um índice fora do alcance
    enum cc_stat status = cc_array_replace_at(a, replacement, 1, &replaced_element);

    // Verifica se o status é de erro por índice fora do alcance
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_swap_at_out_of_range_index1() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona dois elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));

    // Tenta trocar um elemento em um índice fora do alcance
    enum cc_stat status = cc_array_swap_at(a, 2, 1);

    // Verifica se o status é de erro por índice fora do alcance
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_swap_at_out_of_range_index2() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona dois elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));

    // Tenta trocar um elemento em um índice fora do alcance
    enum cc_stat status = cc_array_swap_at(a, 1, 2);

    // Verifica se o status é de erro por índice fora do alcance
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_not_found() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove um elemento que não está no array
    void *removed_element;
    enum cc_stat status = cc_array_remove(a, (void *)4, &removed_element);

    // Verifica se o status é de valor não encontrado
    bool result = (status == CC_ERR_VALUE_NOT_FOUND && removed_element == NULL);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_last_element1() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove o último elemento do array
    void *removed_element;
    enum cc_stat status = cc_array_remove(a, (void *)3, &removed_element);

    // Verifica se o status é de sucesso e se o último elemento foi removido
    bool result = (status == CC_OK && removed_element == (void *)3 && cc_array_size(a) == 2);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_at_assignment() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove o elemento no último índice do array
    void *removed_element;
    enum cc_stat status = cc_array_remove_at(a, 2, &removed_element);

    // Verifica se o status é de sucesso e se o elemento foi removido corretamente
    bool result = (status == CC_OK && removed_element == (void *)3 && cc_array_size(a) == 2);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_at_division_by_one() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove o elemento no último índice do array
    void *removed_element;
    enum cc_stat status = cc_array_remove_at(a, 2, &removed_element);

    // Verifica se o status é de sucesso e se o elemento foi removido corretamente
    bool result = (status == CC_OK && removed_element == (void *)3 && cc_array_size(a) == 2);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_at_subtract_zero() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove o elemento no último índice do array
    void *removed_element;
    enum cc_stat status = cc_array_remove_at(a, 2, &removed_element);

    // Verifica se o status é de sucesso e se o elemento foi removido corretamente
    bool result = (status == CC_OK && removed_element == (void *)3 && cc_array_size(a) == 2);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_remove_at_index_comparison() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Adiciona alguns elementos ao array
    ASSERT_CC_OK(cc_array_add(a, (void *)1));
    ASSERT_CC_OK(cc_array_add(a, (void *)2));
    ASSERT_CC_OK(cc_array_add(a, (void *)3));

    // Remove o elemento no último índice do array
    void *removed_element;
    enum cc_stat status = cc_array_remove_at(a, 2, &removed_element);

    // Verifica se o status é de sucesso e se o elemento foi removido corretamente
    bool result = (status == CC_OK && removed_element == (void *)3 && cc_array_size(a) == 2);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_get_last_empty_array() {
    CC_Array *a;
    ASSERT_CC_OK(cc_array_new(&a));

    // Configura o tamanho do array para ser negativo
    a->size = -1;

    void *last_element;
    enum cc_stat status = cc_array_get_last(a, &last_element);

    // Verifica se o status é CC_ERR_VALUE_NOT_FOUND
    bool result = (status == CC_ERR_VALUE_NOT_FOUND);

    cc_array_destroy(a);
    return result;
}

bool test_cc_array_subarray_mutant() {
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));

    size_t b = 0, e = 1;
    CC_Array *out;
    enum cc_stat status = cc_array_subarray(ar, b, e, &out);

    bool result = (status == CC_ERR_ALLOC);

    cc_array_destroy(ar);
    return result;
}

bool keep_all(const void *element) {
    return true;
}

bool test_cc_array_filter_mut_negative_size() {
    // Cria um array vazio
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));

    // Define o tamanho do array como -1
    ar->size = -1;

    // Define uma função de predicado para manter todos os elementos
    bool (*pred)(const void *) = &keep_all;

    // Chama a função cc_array_filter_mut com o array de tamanho negativo
    enum cc_stat status = cc_array_filter_mut(ar, pred);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_trim_capacity() {
    // Cria um array com tamanho 3 e capacidade 2
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 3));

    // Define o tamanho do array como 3 e a capacidade como 2
    ar->size = 3;
    ar->capacity = 2;

    // Chama a função cc_array_trim_capacity
    enum cc_stat status = cc_array_trim_capacity(ar);

    // Verifica se a capacidade foi reduzida para 3
    bool result = (status == CC_OK && ar->capacity == 3);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_add_at_invalid_index_negative() {
    // Cria um array vazio
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));

    // Tenta adicionar um elemento com index -1
    enum cc_stat status = cc_array_add_at(ar, (void*) 1, -1);

    // Verifica se o status é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_add_at_invalid_index_zero() {
    // Cria um array vazio
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));

    // Tenta adicionar um elemento com index 0
    enum cc_stat status = cc_array_add_at(ar, (void*) 1, 0);

    // Verifica se o status é CC_OK
    bool result = (status == CC_OK && cc_array_size(ar) == 1);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_add_at_invalid_index_equal_to_size() {
    // Cria um array com um elemento
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));

    // Tenta adicionar um elemento com index igual a size
    enum cc_stat status = cc_array_add_at(ar, (void*) 2, 1);

    // Verifica se o status é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE && cc_array_size(ar) == 1);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_add_at_invalid_index_larger_than_size() {
    // Cria um array com um elemento
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));

    // Tenta adicionar um elemento com index maior que size
    enum cc_stat status = cc_array_add_at(ar, (void*) 2, 3);

    // Verifica se o status é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE && cc_array_size(ar) == 1);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_add_at_invalid_index_in_middle() {
    // Cria um array com três elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 3));

    // Tenta adicionar um elemento com index no meio do array
    enum cc_stat status = cc_array_add_at(ar, (void*) 4, 1);

    // Verifica se o status é CC_OK e se o elemento foi adicionado corretamente
    void *added_element;
    cc_array_get_at(ar, 1, &added_element);
    bool result = (status == CC_OK && cc_array_size(ar) == 4 && added_element == (void*) 4);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_swap_at_invalid_index1() {
    // Cria um array com dois elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));

    // Tenta trocar os elementos em índices inválidos
    enum cc_stat status = cc_array_swap_at(ar, 1, 4);

    // Verifica se o status é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_swap_at_invalid_index2() {
    // Cria um array com dois elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));

    // Tenta trocar os elementos em índices inválidos
    enum cc_stat status = cc_array_swap_at(ar, 4, 1);

    // Verifica se o status é CC_ERR_OUT_OF_RANGE
    bool result = (status == CC_ERR_OUT_OF_RANGE);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_remove_index_1() {
    // Cria um array com dois elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));

    // Tenta remover um elemento em um índice válido
    void* removed_element;
    enum cc_stat status = cc_array_remove(ar, (void*) 2, &removed_element);

    // Verifica se o status é CC_OK e se o elemento removido é o esperado
    bool result = (status == CC_OK && (intptr_t)removed_element == 2 && cc_array_size(ar) == 1);


    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_remove_index_2() {
    // Cria um array com três elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 3));

    // Tenta remover um elemento em um índice válido
    void* removed_element;
    enum cc_stat status = cc_array_remove(ar, (void*) 3, &removed_element);

    // Verifica se o status é CC_OK e se o elemento removido é o esperado
    bool result = (status == CC_OK && (intptr_t)removed_element == 3 && cc_array_size(ar) == 2);


    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_remove_index_3() {
    // Cria um array com cinco elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 3));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 4));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 5));

    // Tenta remover um elemento em um índice válido
    void* removed_element;
    enum cc_stat status = cc_array_remove(ar, (void*) 5, &removed_element);

    // Verifica se o status é CC_OK e se o elemento removido é o esperado
    bool result = (status == CC_OK && (intptr_t)removed_element == 5 && cc_array_size(ar) == 4);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_cc_array_remove_index_4() {
    // Cria um array com três elementos
    CC_Array *ar;
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 1));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 2));
    ASSERT_CC_OK(cc_array_add(ar, (void*) 3));

    // Tenta remover um elemento em um índice inválido
    void* removed_element;
    enum cc_stat status = cc_array_remove(ar, (void*) 4, &removed_element);

    // Verifica se o status é CC_ERR_VALUE_NOT_FOUND
    bool result = (status == CC_ERR_VALUE_NOT_FOUND);

    // Limpa a memória
    cc_array_destroy(ar);

    return result;
}

bool test_remove_at_index_3_size_3() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));

    ASSERT_CC_OK(cc_array_remove_at(ar, 3, &removed_element));
    ASSERT_EQ(*(int *)removed_element, 3);
    cc_array_destroy(ar);
    return true;
}

bool test_remove_at_index_5_size_3() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));

    ASSERT_EQ(cc_array_remove_at(ar, 5, &removed_element), CC_ERR_OUT_OF_RANGE);
    cc_array_destroy(ar);
    return true;
}

bool test_remove_at_index_3_size_5() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3, element4 = 4, element5 = 5;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));
    ASSERT_CC_OK(cc_array_add(ar, &element4));
    ASSERT_CC_OK(cc_array_add(ar, &element5));

    ASSERT_CC_OK(cc_array_remove_at(ar, 3, &removed_element));
    ASSERT_EQ(*(int *)removed_element, 4);
    cc_array_destroy(ar);
    return true;
}

bool test_remove_at_index_4_size_3() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));

    ASSERT_EQ(cc_array_remove_at(ar, 4, &removed_element), CC_ERR_OUT_OF_RANGE);
    cc_array_destroy(ar);
    return true;
}

bool test_remove_at_index_7_size_6() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3, element4 = 4, element5 = 5, element6 = 6;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));
    ASSERT_CC_OK(cc_array_add(ar, &element4));
    ASSERT_CC_OK(cc_array_add(ar, &element5));
    ASSERT_CC_OK(cc_array_add(ar, &element6));

    ASSERT_EQ(cc_array_remove_at(ar, 7, &removed_element), CC_ERR_OUT_OF_RANGE);
    cc_array_destroy(ar);
    return true;
}

bool test_remove_at_index_5_size_6() {
    CC_Array *ar;
    int element1 = 1, element2 = 2, element3 = 3, element4 = 4, element5 = 5, element6 = 6;
    void *removed_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));
    ASSERT_CC_OK(cc_array_add(ar, &element2));
    ASSERT_CC_OK(cc_array_add(ar, &element3));
    ASSERT_CC_OK(cc_array_add(ar, &element4));
    ASSERT_CC_OK(cc_array_add(ar, &element5));
    ASSERT_CC_OK(cc_array_add(ar, &element6));

    ASSERT_CC_OK(cc_array_remove_at(ar, 5, &removed_element));
    ASSERT_EQ(*(int *)removed_element, 6);
    cc_array_destroy(ar);
    return true;
}

bool test_get_at_index_3_size_1() {
    CC_Array *ar;
    int element1 = 1;
    void *result;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &element1));

    ASSERT_EQ(cc_array_get_at(ar, 3, &result), CC_ERR_OUT_OF_RANGE);
    cc_array_destroy(ar);
    return true;
}

bool is_even(const void *element) {
    int num = *((int *)element);
    return num % 2 == 0;
}

bool test_get_last_size_minus_1() {
    CC_Array *ar;
    int data = 42; // Dados fictícios
    void *last_element;

    ASSERT_CC_OK(cc_array_new(&ar));
    ar->size = -1; // Definindo o tamanho como -1
    ASSERT_EQ(cc_array_get_last(ar, &last_element), CC_ERR_OUT_OF_RANGE); // Como o tamanho é -1, o array está vazio
    cc_array_destroy(ar);
    return true;
}

bool test_reverse_size_1() {
    CC_Array *ar;
    int data = 42; // Dados fictícios

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &data)); // Adicionando um elemento
    cc_array_reverse(ar); // Revertendo
    ASSERT_EQ(cc_array_size(ar), 1); // O tamanho deve permanecer o mesmo
    void *element;
    ASSERT_CC_OK(cc_array_get_at(ar, 0, &element)); // Obtendo o elemento
    ASSERT_EQ(*(int*)element, 42); // O elemento deve permanecer o mesmo
    cc_array_destroy(ar);
    return true;
}

bool test_reverse_size_0() {
    CC_Array *ar;

    ASSERT_CC_OK(cc_array_new(&ar));
    cc_array_reverse(ar); // Revertendo um array vazio
    ASSERT_EQ(cc_array_size(ar), 0); // O tamanho deve permanecer 0
    cc_array_destroy(ar);
    return true;
}

bool test_reverse_size_minus_1() {
    CC_Array *ar;

    ASSERT_CC_OK(cc_array_new(&ar));
    ar->size = -1; // Definindo o tamanho como -1
    cc_array_reverse(ar); // Revertendo
    ASSERT_EQ(cc_array_size(ar), -1); // O tamanho deve permanecer -1
    cc_array_destroy(ar);
    return true;
}

void sum_reduce_fn(void *a, void *b, void *result) {
    if (b != NULL) {
        int *x = (int *)a;
        int *y = (int *)b;
        int *res = (int *)result;
        *res = *x + *y;
    } else {
        int *x = (int *)a;
        int *res = (int *)result;
        *res = *x;
    }
}


bool test_reduce_size_2() {
    CC_Array *ar;
    int data[] = {2, 3}; // Dados fictícios
    int result = 0;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &data[0])); // Adicionando os dados ao array
    ASSERT_CC_OK(cc_array_add(ar, &data[1]));
    cc_array_reduce(ar, sum_reduce_fn, &result); // Reduzindo
    ASSERT_EQ(result, 5); // O resultado deve ser a soma dos elementos
    cc_array_destroy(ar);
    return true;
}

bool test_reduce_size_1() {
    CC_Array *ar;
    int data = 42; // Dados fictícios
    int result = 0;

    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, &data)); // Adicionando um elemento ao array
    cc_array_reduce(ar, sum_reduce_fn, &result); // Reduzindo
    ASSERT_EQ(result, 42); // O resultado deve ser o próprio elemento
    cc_array_destroy(ar);
    return true;
}

bool test_reduce_size_0() {
    CC_Array *ar;
    int result = 0;

    ASSERT_CC_OK(cc_array_new(&ar));
    cc_array_reduce(ar, sum_reduce_fn, &result); // Reduzindo um array vazio
    ASSERT_EQ(result, 0); // O resultado deve ser 0
    cc_array_destroy(ar);
    return true;
}

bool test_reduce_size_minus_1() {
    CC_Array *ar;
    int result = 0;

    ASSERT_CC_OK(cc_array_new(&ar));
    ar->size = -1; // Definindo o tamanho como -1
    cc_array_reduce(ar, sum_reduce_fn, &result); // Reduzindo
    ASSERT_EQ(result, 0); // O resultado deve ser 0
    cc_array_destroy(ar);
    return true;
}


bool new_test() {
    CC_Array* a;
    CC_ArrayConf conf;
    cc_array_conf_init(&conf);
    conf.capacity = 10;
    conf.exp_factor = 2;

    ASSERT_CC_OK(cc_array_new_conf(&conf, &a))
    ASSERT_EQ(10, cc_array_capacity(a))
    ASSERT_EQ(0, cc_array_size(a))

    ASSERT_CC_OK(cc_array_add_at(a, (void*) 2, 0))
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 3, 1))
    ASSERT_EQ(2, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(3, *(int*)get_result);

    ASSERT_CC_OK(cc_array_remove_at(a, 0, NULL))
    ASSERT_EQ(1, cc_array_size(a))

    void* replace_result;
    ASSERT_CC_OK(cc_array_replace_at(a, (void*) 4, 0, &replace_result))
    ASSERT_EQ(3, *(int*) replace_result)

    CC_Array* subarray;
    ASSERT_CC_OK(cc_array_subarray(a, 0, 0, &subarray))
    ASSERT_EQ(1, cc_array_size(subarray))

    CC_Array* shallow_copy;
    ASSERT_CC_OK(cc_array_copy_shallow(a, &shallow_copy))
    ASSERT_EQ(1, cc_array_size(shallow_copy))

    cc_array_destroy(a);
    cc_array_destroy(subarray);
    cc_array_destroy(shallow_copy);
    return true;
}

// Test to add elements at different positions
bool test_add_at() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add_at(a, (void*) 1, 0))
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 2, 1))
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 3, 1))  // Insert in the middle

    ASSERT_EQ(3, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(3, *(int*) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to remove elements and verify array structure
bool test_remove() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    ASSERT_EQ(3, cc_array_size(a))

    void* remove_result;
    ASSERT_CC_OK(cc_array_remove(a, (void*) 2, &remove_result))
    ASSERT_EQ(2, *(int*) remove_result)
    ASSERT_EQ(2, cc_array_size(a))

    ASSERT_CC_OK(cc_array_get_at(a, 1, &remove_result))
    ASSERT_EQ(3, *(int*) remove_result)

    cc_array_destroy(a);
    return true;
}

// Test to replace elements at different positions
bool test_replace_at() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    ASSERT_EQ(3, cc_array_size(a))

    void* replace_result;
    ASSERT_CC_OK(cc_array_replace_at(a, (void*) 4, 1, &replace_result))
    ASSERT_EQ(2, *(int*) replace_result)
    ASSERT_EQ(3, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(4, *(int*) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to trim the capacity of the array
bool test_trim_capacity() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 10; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_EQ(10, cc_array_size(a))
    ASSERT_CC_OK(cc_array_trim_capacity(a))
    ASSERT_EQ(10, cc_array_capacity(a))

    cc_array_destroy(a);
    return true;
}

// Test to copy the array deeply
bool test_copy_deep() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* deep_copy;
    ASSERT_CC_OK(cc_array_copy_deep(a, NULL, &deep_copy))
    ASSERT_EQ(5, cc_array_size(deep_copy))

    void* get_result;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_get_at(deep_copy, i, &get_result))
        ASSERT_EQ(i, (int) (intptr_t) get_result)
    }

    cc_array_destroy(a);
    cc_array_destroy(deep_copy);
    return true;
}

// Test to sort the array
int compare(const void* a, const void* b) {
    return (int) (intptr_t) a - (int) (intptr_t) b;
}

bool test_sort() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 3))
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))

    cc_array_sort(a, compare);

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to filter the array
bool is_even2(const void* value) {
    return ((int) (intptr_t) value) % 2 == 0;
}

bool test_filter() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* filtered;
    ASSERT_CC_OK(cc_array_filter(a, is_even2, &filtered))
    ASSERT_EQ(3, cc_array_size(filtered))  // 0, 2, 4

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(filtered, 0, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(filtered, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(filtered, 2, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    cc_array_destroy(filtered);
    return true;
}


// Test to reverse the array
bool test_reverse() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    cc_array_reverse(a);

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 4, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to remove and free all elements from the array
bool test_remove_all_free() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    cc_array_remove_all_free(a);

    ASSERT_EQ(0, cc_array_size(a))

    cc_array_destroy(a);
    return true;
}

// Test iterating and removing elements using iterator
bool test_iter_remove() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    void* get_result;
    while (cc_array_iter_next(&iter, &get_result) == CC_OK) {
        if ((int) (intptr_t) get_result % 2 == 0) {
            ASSERT_CC_OK(cc_array_iter_remove(&iter, NULL))
        }
    }

    ASSERT_EQ(2, cc_array_size(a))  // Only odd numbers should remain

    cc_array_destroy(a);
    return true;
}

// Test creating subarrays with different ranges
bool test_subarray() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* subarray;
    ASSERT_CC_OK(cc_array_subarray(a, 1, 3, &subarray))
    ASSERT_EQ(3, cc_array_size(subarray))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(subarray, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(subarray, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(subarray, 2, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    cc_array_destroy(subarray);
    return true;
}


// Test adding elements using iterator
bool test_iter_add() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    ASSERT_CC_OK(cc_array_iter_next(&iter, NULL))
    ASSERT_CC_OK(cc_array_iter_add(&iter, (void*) (intptr_t) 10))
    ASSERT_EQ(4, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(10, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test replacing elements using iterator
bool test_iter_replace() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    ASSERT_CC_OK(cc_array_iter_next(&iter, NULL))
    void* replace_result;
    ASSERT_CC_OK(cc_array_iter_replace(&iter, (void*) (intptr_t) 10, &replace_result))
    ASSERT_EQ(0, (int) (intptr_t) replace_result)

    ASSERT_CC_OK(cc_array_get_at(a, 0, &replace_result))
    ASSERT_EQ(10, (int) (intptr_t) replace_result)

    cc_array_destroy(a);
    return true;
}

// Test iterating two arrays in parallel
bool test_zip_iter_next() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(3, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(1, (int) (intptr_t) result1)
    ASSERT_EQ(4, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(2, (int) (intptr_t) result1)
    ASSERT_EQ(5, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test adding elements to two arrays in parallel using zip iterator
bool test_zip_iter_add() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 2; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 2)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL))
    ASSERT_CC_OK(cc_array_zip_iter_add(&iter, (void*) (intptr_t) 10, (void*) (intptr_t) 20))
    ASSERT_EQ(3, cc_array_size(a1))
    ASSERT_EQ(3, cc_array_size(a2))

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_get_at(a1, 1, &result1))
    ASSERT_EQ(10, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 1, &result2))
    ASSERT_EQ(20, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test removing elements from two arrays in parallel using zip iterator
bool test_zip_iter_remove() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(3, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_remove(&iter, NULL, NULL))
    ASSERT_EQ(2, cc_array_size(a1))
    ASSERT_EQ(2, cc_array_size(a2))

    ASSERT_CC_OK(cc_array_get_at(a1, 0, &result1))
    ASSERT_EQ(1, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 0, &result2))
    ASSERT_EQ(4, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to reduce the array to a single value
void sum(void* a, void* b, void* result) {
    *(int*)result = *(int*)a + *(int*)b;
}

bool test_reduce() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 1; i <= 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    int result = 0;
    cc_array_reduce(a, sum, &result);
    ASSERT_EQ(15, result)  // 1+2+3+4+5 = 15

    cc_array_destroy(a);
    return true;
}

// Test to map a function over the array
void increment(void* e) {
    int* num = (int*)e;
    (*num)++;
}

bool test_map() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    cc_array_map(a, increment);

    void* get_result;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_get_at(a, i, &get_result))
        ASSERT_EQ(i + 1, *(int*)get_result)
    }

    cc_array_destroy(a);
    return true;
}

// Test to check if the array contains a specific element
bool test_contains() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_EQ(3, cc_array_contains(a, (void*) 3))
    ASSERT_EQ(0, cc_array_contains(a, (void*) 5))

    cc_array_destroy(a);
    return true;
}

// Test to find the index of a specific element
bool test_index_of() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    size_t index;
    ASSERT_CC_OK(cc_array_index_of(a, (void*) 3, &index))
    ASSERT_EQ(3, index)

    enum cc_stat status = cc_array_index_of(a, (void*) 5, &index);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test to check the size and capacity of the array after various operations
bool test_size_capacity() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_EQ(0, cc_array_size(a))
    ASSERT_EQ(8, cc_array_capacity(a))

    for (int i = 0; i < 10; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_EQ(10, cc_array_size(a))
    ASSERT_TRUE(cc_array_capacity(a) > 10)

    cc_array_trim_capacity(a);
    ASSERT_EQ(10, cc_array_capacity(a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_remove_at(a, 0, NULL))
    }

    ASSERT_EQ(5, cc_array_size(a))

    cc_array_destroy(a);
    return true;
}

// Test to get the last element of the array
bool test_get_last() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    void* last_element;
    ASSERT_CC_OK(cc_array_get_last(a, &last_element))
    ASSERT_EQ(4, (int) (intptr_t) last_element)

    cc_array_destroy(a);
    return true;
}

// Test to iterate over all elements of the array
bool test_iterate() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    void* element;
    int count = 0;
    while (cc_array_iter_next(&iter, &element) == CC_OK) {
        ASSERT_EQ(count, (int) (intptr_t) element)
        count++;
    }

    ASSERT_EQ(5, count)

    cc_array_destroy(a);
    return true;
}

// Test to replace elements in different positions using iterator
bool test_iter_replace_positions() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    void* replaced_element;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_iter_next(&iter, NULL))
        ASSERT_CC_OK(cc_array_iter_replace(&iter, (void*) (intptr_t) (i + 10), &replaced_element))
        ASSERT_EQ(i, (int) (intptr_t) replaced_element)
    }

    for (int i = 0; i < 5; i++) {
        void* element;
        ASSERT_CC_OK(cc_array_get_at(a, i, &element))
        ASSERT_EQ(i + 10, (int) (intptr_t) element)
    }

    cc_array_destroy(a);
    return true;
}

// Test to create subarrays with different ranges
bool test_subarray_ranges() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 10; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* subarray;
    ASSERT_CC_OK(cc_array_subarray(a, 2, 5, &subarray))
    ASSERT_EQ(4, cc_array_size(subarray))  // 2, 3, 4, 5

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(subarray, 0, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(subarray, 1, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(subarray, 2, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(subarray, 3, &get_result))
    ASSERT_EQ(5, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    cc_array_destroy(subarray);
    return true;
}

// Test to remove the last element of the array
bool test_remove_last() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    void* last_element;
    ASSERT_CC_OK(cc_array_remove_last(a, &last_element))
    ASSERT_EQ(4, (int) (intptr_t) last_element)
    ASSERT_EQ(4, cc_array_size(a))

    cc_array_destroy(a);
    return true;
}

// Test to replace elements in parallel using zip iterator
bool test_zip_iter_replace() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(3, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_replace(&iter, (void*) 10, (void*) 20, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(3, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_get_at(a1, 0, &result1))
    ASSERT_EQ(10, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 0, &result2))
    ASSERT_EQ(20, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to get the index of the last iterated element in parallel using zip iterator
bool test_zip_iter_index() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    size_t index = 0;
    while (cc_array_zip_iter_next(&iter, &result1, &result2) == CC_OK) {
        index = cc_array_zip_iter_index(&iter);
    }

    ASSERT_EQ(2, index)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to filter the array mutably
bool test_filter_mut() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 6; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_CC_OK(cc_array_filter_mut(a, is_even))
    ASSERT_EQ(3, cc_array_size(a))  // 0, 2, 4

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Comparator function for test_contains_value
int compare_int(const void* a, const void* b) {
    return (int) (intptr_t) a - (int) (intptr_t) b;
}

// Test to check if the array contains a specific value using comparator
bool test_contains_value() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_EQ(1, cc_array_contains_value(a, (void*) 3, compare_int))
    ASSERT_EQ(0, cc_array_contains_value(a, (void*) 5, compare_int))

    cc_array_destroy(a);
    return true;
}

// Test to copy the array shallowly
bool test_copy_shallow() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* shallow_copy;
    ASSERT_CC_OK(cc_array_copy_shallow(a, &shallow_copy))
    ASSERT_EQ(5, cc_array_size(shallow_copy))

    void* get_result;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_get_at(shallow_copy, i, &get_result))
        ASSERT_EQ(i, (int) (intptr_t) get_result)
    }

    cc_array_destroy(a);
    cc_array_destroy(shallow_copy);
    return true;
}

// Test to swap elements at specific positions
bool test_swap_at() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    ASSERT_EQ(3, cc_array_size(a))

    ASSERT_CC_OK(cc_array_swap_at(a, 0, 2))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(3, *(int*) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(1, *(int*) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to remove all elements from the array
bool test_remove_all() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    cc_array_remove_all(a);
    ASSERT_EQ(0, cc_array_size(a))

    cc_array_destroy(a);
    return true;
}

// Test to get the index of the last iterated element using iterator
bool test_iter_index() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    void* element;
    size_t index = 0;
    while (cc_array_iter_next(&iter, &element) == CC_OK) {
        index = cc_array_iter_index(&iter);
    }

    ASSERT_EQ(4, index)

    cc_array_destroy(a);
    return true;
}

// Test to add elements to two arrays in parallel using zip iterator
bool test_zip_iter_add_parallel() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 2; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 2)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL))
    ASSERT_CC_OK(cc_array_zip_iter_add(&iter, (void*) (intptr_t) 10, (void*) (intptr_t) 20))
    ASSERT_EQ(3, cc_array_size(a1))
    ASSERT_EQ(3, cc_array_size(a2))

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_get_at(a1, 1, &result1))
    ASSERT_EQ(10, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 1, &result2))
    ASSERT_EQ(20, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Additional reduce test with multiplication
void multiply(void* a, void* b, void* result) {
    *(int*)result = (int)a * ((int)b ? (int)b : 1);
}

bool test_reduce_multiplication() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 1; i <= 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    int result = 1;
    cc_array_reduce(a, multiply, &result);
    ASSERT_EQ(120, result)  // 1*2*3*4*5 = 120

    cc_array_destroy(a);
    return true;
}

// Test to get elements at different positions, including edge cases
bool test_get_at() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 4, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    enum cc_stat status = cc_array_get_at(a, 5, &get_result);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test removing elements in parallel using zip iterator with additional checks
bool test_zip_iter_remove_extended() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(3, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_remove(&iter, NULL, NULL))
    ASSERT_EQ(2, cc_array_size(a1))
    ASSERT_EQ(2, cc_array_size(a2))

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(1, (int) (intptr_t) result1)
    ASSERT_EQ(4, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_remove(&iter, NULL, NULL))
    ASSERT_EQ(1, cc_array_size(a1))
    ASSERT_EQ(1, cc_array_size(a2))

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(2, (int) (intptr_t) result1)
    ASSERT_EQ(5, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to check if the array contains duplicated elements
bool test_contains_duplicates() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))  // Duplicate

    ASSERT_EQ(2, cc_array_contains(a, (void*) 1))
    ASSERT_EQ(1, cc_array_contains(a, (void*) 2))

    cc_array_destroy(a);
    return true;
}

// Additional sort test with floating point numbers
int compare_float(const void* a, const void* b) {
    return (*(float*)a > *(float*)b) - (*(float*)a < *(float*)b);
}

bool test_sort_float() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    float values[] = {3.1, 1.2, 2.3};
    for (int i = 0; i < 3; i++) {
        float* num = malloc(sizeof(float));
        *num = values[i];
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    cc_array_sort(a, compare_float);

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1.2, *(float*)get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2.3, *(float*)get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(3.1, *(float*)get_result)

    cc_array_destroy(a);
    return true;
}

// Additional map test with decrement function
void decrement(void* e) {
    int* num = (int*)e;
    (*num)--;
}

bool test_map_decrement() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    cc_array_map(a, decrement);

    void* get_result;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_get_at(a, i, &get_result))
        ASSERT_EQ(i - 1, *(int*)get_result)
    }

    cc_array_destroy(a);
    return true;
}

// Additional reduce test with subtraction
void subtract(void* a, void* b, void* result) {
    *(int*)result = *(int*)a - *(int*)b;
}

bool test_reduce_subtraction() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 1; i <= 5; i++) {
        int* num = malloc(sizeof(int));
        *num = i;
        ASSERT_CC_OK(cc_array_add(a, num))
    }

    int result = 0;
    cc_array_reduce(a, subtract, &result);
    ASSERT_EQ(-13, result)  // (((1-2)-3)-4)-5 = -13

    cc_array_destroy(a);
    return true;
}

// Test to add elements using iterator in different scenarios
bool test_iter_add_multiple() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    ASSERT_CC_OK(cc_array_iter_next(&iter, NULL))
    ASSERT_CC_OK(cc_array_iter_add(&iter, (void*) (intptr_t) 10))
    ASSERT_CC_OK(cc_array_iter_add(&iter, (void*) (intptr_t) 20))

    ASSERT_EQ(5, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(10, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(20, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test iterating two arrays of different sizes in parallel
bool test_zip_iter_next_different_sizes() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
    }

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 5)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(5, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(1, (int) (intptr_t) result1)
    ASSERT_EQ(6, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(2, (int) (intptr_t) result1)
    ASSERT_EQ(7, (int) (intptr_t) result2)

    enum cc_stat status = cc_array_zip_iter_next(&iter, &result1, &result2);
    ASSERT_EQ(CC_ITER_END, status)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test getting the last element of an empty array
bool test_get_last_empty() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    void* last_element;
    enum cc_stat status = cc_array_get_last(a, &last_element);
    ASSERT_EQ(CC_ERR_VALUE_NOT_FOUND, status)

    cc_array_destroy(a);
    return true;
}

// Test to remove alternating elements using iterator
bool test_iter_remove_alternate() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 10; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    void* element;
    int count = 0;
    while (cc_array_iter_next(&iter, &element) == CC_OK) {
        if (count % 2 == 0) {
            ASSERT_CC_OK(cc_array_iter_remove(&iter, NULL))
        }
        count++;
    }

    ASSERT_EQ(5, cc_array_size(a))

    cc_array_destroy(a);
    return true;
}

// Test to add multiple elements to two arrays in parallel using zip iterator
bool test_zip_iter_add_multiple() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 3)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL))
    ASSERT_CC_OK(cc_array_zip_iter_add(&iter, (void*) (intptr_t) 10, (void*) (intptr_t) 20))
    ASSERT_CC_OK(cc_array_zip_iter_add(&iter, (void*) (intptr_t) 30, (void*) (intptr_t) 40))
    ASSERT_EQ(5, cc_array_size(a1))
    ASSERT_EQ(5, cc_array_size(a2))

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_get_at(a1, 1, &result1))
    ASSERT_EQ(10, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 1, &result2))
    ASSERT_EQ(20, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_get_at(a1, 2, &result1))
    ASSERT_EQ(30, (int) (intptr_t) result1)

    ASSERT_CC_OK(cc_array_get_at(a2, 2, &result2))
    ASSERT_EQ(40, (int) (intptr_t) result2)

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to reverse arrays of different sizes
bool test_reverse_different_sizes() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    cc_array_reverse(a);
    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    for (int i = 3; i < 7; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    cc_array_reverse(a);
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(6, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(5, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 4, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 5, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 6, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to sort an array of strings
int compare_string(const void* a, const void* b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

bool test_sort_string() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    const char* values[] = {"banana", "apple", "cherry"};
    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*)values[i]))
    }

    cc_array_sort(a, compare_string);

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(0, strcmp("apple", (const char*)get_result))

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(0, strcmp("banana", (const char*)get_result))

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(0, strcmp("cherry", (const char*)get_result))

    cc_array_destroy(a);
    return true;
}

// Test to check if the array contains a specific structure
typedef struct {
    int id;
    char name[50];
} Person;

int compare_person(const void* a, const void* b) {
    return ((Person*)a)->id - ((Person*)b)->id;
}

bool test_contains_person() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    Person p1 = {1, "Alice"};
    Person p2 = {2, "Bob"};
    Person p3 = {3, "Charlie"};

    ASSERT_CC_OK(cc_array_add(a, &p1))
    ASSERT_CC_OK(cc_array_add(a, &p2))
    ASSERT_CC_OK(cc_array_add(a, &p3))

    Person p4 = {2, "Bob"};
    ASSERT_EQ(1, cc_array_contains_value(a, &p4, compare_person))

    Person p5 = {4, "David"};
    ASSERT_EQ(0, cc_array_contains_value(a, &p5, compare_person))

    cc_array_destroy(a);
    return true;
}

// Test removing elements from two arrays of different sizes in parallel using zip iterator
bool test_zip_iter_remove_different_sizes() {
    CC_Array* a1;
    CC_Array* a2;
    ASSERT_CC_OK(cc_array_new(&a1))
    ASSERT_CC_OK(cc_array_new(&a2))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a1, (void*) (intptr_t) i))
    }

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a2, (void*) (intptr_t) (i + 5)))
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, a1, a2);

    void* result1;
    void* result2;
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, &result1, &result2))
    ASSERT_EQ(0, (int) (intptr_t) result1)
    ASSERT_EQ(5, (int) (intptr_t) result2)

    ASSERT_CC_OK(cc_array_zip_iter_remove(&iter, NULL, NULL))
    ASSERT_EQ(4, cc_array_size(a1))
    ASSERT_EQ(2, cc_array_size(a2))

    cc_array_destroy(a1);
    cc_array_destroy(a2);
    return true;
}

// Test to check if the array contains repeated elements
bool test_contains_repeated() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))  // Duplicate
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))  // Duplicate

    ASSERT_EQ(2, cc_array_contains(a, (void*) 1))
    ASSERT_EQ(2, cc_array_contains(a, (void*) 2))
    ASSERT_EQ(1, cc_array_contains(a, (void*) 3))
    ASSERT_EQ(0, cc_array_contains(a, (void*) 4))

    cc_array_destroy(a);
    return true;
}

// Test creating subarrays with invalid indices
bool test_subarray_invalid_indices() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    CC_Array* subarray;
    enum cc_stat status = cc_array_subarray(a, 4, 6, &subarray);
    ASSERT_EQ(CC_ERR_INVALID_RANGE, status)

    status = cc_array_subarray(a, 6, 7, &subarray);
    ASSERT_EQ(CC_ERR_INVALID_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test trimming capacity after multiple additions and removals
bool test_trim_capacity_after_operations() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 20; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    for (int i = 0; i < 10; i++) {
        ASSERT_CC_OK(cc_array_remove_at(a, 0, NULL))
    }

    ASSERT_EQ(10, cc_array_size(a))
    ASSERT_TRUE(cc_array_capacity(a) > 10)

    cc_array_trim_capacity(a);
    ASSERT_EQ(10, cc_array_capacity(a))

    cc_array_destroy(a);
    return true;
}

// Custom memory allocation functions for testing
void* test_malloc(size_t size) {
    return malloc(size);
}

void* test_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

void test_free(void* ptr) {
    free(ptr);
}

// Floating-point comparison with epsilon
bool float_equal(float a, float b, float epsilon) {
    return fabs(a - b) < epsilon;
}

// Test configuration with expansion factor <= 1
bool test_exp_factor_leq_1() {
    CC_ArrayConf conf;
    CC_Array* array;

    // Set up the configuration
    conf.capacity = 10;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    // Test with exp_factor < 1
    conf.exp_factor = 0.5;
    ASSERT_EQ(CC_OK, cc_array_new_conf(&conf, &array))
    cc_array_destroy(array);

    // Test with exp_factor == 1
    conf.exp_factor = 1.0;
    ASSERT_EQ(CC_OK, cc_array_new_conf(&conf, &array))
    cc_array_destroy(array);

    return true;
}

// Test configuration with expansion factor > 1
bool test_exp_factor_gt_1() {
    CC_ArrayConf conf;
    CC_Array* array;

    // Set up the configuration
    conf.exp_factor = 2.0;
    conf.capacity = 10;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    // Test with exp_factor > 1
    ASSERT_EQ(CC_OK, cc_array_new_conf(&conf, &array))
    cc_array_destroy(array);

    return true;
}

// Test configuration with capacity == 0
bool test_invalid_capacity() {
    CC_ArrayConf conf;
    CC_Array* array;

    // Set up the configuration
    conf.exp_factor = 2.0;
    conf.capacity = 0;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    // Test with invalid capacity
    ASSERT_EQ(CC_ERR_INVALID_CAPACITY, cc_array_new_conf(&conf, &array))

    return true;
}

// Test configuration with valid capacity and expansion factor
bool test_valid_capacity_and_exp_factor() {
    CC_ArrayConf conf;
    CC_Array* array;

    // Set up the configuration
    conf.exp_factor = 2.0;
    conf.capacity = 10;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    // Test with valid capacity and expansion factor
    ASSERT_EQ(CC_OK, cc_array_new_conf(&conf, &array))
    cc_array_destroy(array);

    return true;
}

// Test configuration with expansion factor large enough to cause overflow
bool test_exp_factor_causes_overflow() {
    CC_ArrayConf conf;
    CC_Array* array;

    // Set up the configuration
    conf.exp_factor = (float)CC_MAX_ELEMENTS / 9;
    conf.capacity = 10;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    // Test with expansion factor causing overflow
    ASSERT_EQ(CC_ERR_INVALID_CAPACITY, cc_array_new_conf(&conf, &array))

    return true;
}

// Test to add elements at the beginning using iterator
bool test_iter_add_at_beginning() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    CC_ArrayIter iter;
    cc_array_iter_init(&iter, a);

    ASSERT_CC_OK(cc_array_iter_add(&iter, (void*) (intptr_t) 1))
    ASSERT_CC_OK(cc_array_iter_add(&iter, (void*) (intptr_t) 2))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test to get the underlying buffer of the array
bool test_get_buffer() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    const void* const* buffer = cc_array_get_buffer(a);
    for (int i = 0; i < 5; i++) {
        ASSERT_EQ(i, (int) (intptr_t) buffer[i])
    }

    cc_array_destroy(a);
    return true;
}

// Test swapping elements at indices out of bounds
bool test_swap_at_out_of_bounds() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 3; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    enum cc_stat status = cc_array_swap_at(a, 0, 3);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    status = cc_array_swap_at(a, 3, 0);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test reversing an array of odd size
bool test_reverse_odd_size() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    cc_array_reverse(a);

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    ASSERT_CC_OK(cc_array_get_at(a, 4, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Predicate function that returns true for all elements
bool true_predicate(const void* e) {
    return true;
}

// Test filtering the array mutably with a predicate that doesn't remove elements
bool test_filter_mut_no_removal() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_add(a, (void*) (intptr_t) i))
    }

    ASSERT_CC_OK(cc_array_filter_mut(a, true_predicate))
    ASSERT_EQ(5, cc_array_size(a))  // No elements should be removed

    void* get_result;
    for (int i = 0; i < 5; i++) {
        ASSERT_CC_OK(cc_array_get_at(a, i, &get_result))
        ASSERT_EQ(i, (int) (intptr_t) get_result)
    }

    cc_array_destroy(a);
    return true;
}

// Test adding element at index 0 in an empty array
bool test_add_at_empty_array() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    // Add element at index 0
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 1, 0))
    ASSERT_EQ(1, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding element at the last position
bool test_add_at_last_position() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    // Add elements to the array
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))

    // Add element at the last position
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 3, 2))
    ASSERT_EQ(3, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding element at a middle position
bool test_add_at_middle_position() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    // Add elements to the array
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    // Add element at the middle position
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 2, 1))
    ASSERT_EQ(3, cc_array_size(a))

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding element at an out of bounds position
bool test_add_at_out_of_bounds() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    // Add elements to the array
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))

    // Try to add element at an out of bounds position
    enum cc_stat status = cc_array_add_at(a, (void*) 2, 5);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test adding element when array is full and requires expansion
bool test_add_at_full_array() {
    CC_ArrayConf conf;
    CC_Array* a;

    // Set up the configuration
    conf.exp_factor = 2.0;
    conf.capacity = 2;
    conf.mem_alloc = test_malloc;
    conf.mem_calloc = test_calloc;
    conf.mem_free = test_free;

    ASSERT_EQ(CC_OK, cc_array_new_conf(&conf, &a))

    // Add elements to the array to fill it
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))

    // Add element at a position, triggering expansion
    ASSERT_CC_OK(cc_array_add_at(a, (void*) 3, 1))
    ASSERT_EQ(3, cc_array_size(a))
    ASSERT_EQ(4, cc_array_capacity(a)) // Check if capacity doubled

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding element at index > size - 1
bool test_add_at_index_greater_than_size_minus_one() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    // Add elements to the array
    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))

    // Try to add element at index greater than size - 1
    enum cc_stat status = cc_array_add_at(a, (void*) 3, 3);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

// Test adding an element at the beginning of the array
bool test_add_at_beginning() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    ASSERT_CC_OK(cc_array_add_at(a, (void*) 0, 0))  // Add at the beginning

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(0, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding an element in the middle of the array
bool test_add_at_middle() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 4))
    ASSERT_CC_OK(cc_array_add(a, (void*) 5))

    ASSERT_CC_OK(cc_array_add_at(a, (void*) 3, 2))  // Add in the middle

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 4, &get_result))
    ASSERT_EQ(5, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding an element at the end of the array
bool test_add_at_end() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    ASSERT_CC_OK(cc_array_add(a, (void*) 1))
    ASSERT_CC_OK(cc_array_add(a, (void*) 2))
    ASSERT_CC_OK(cc_array_add(a, (void*) 3))

    ASSERT_CC_OK(cc_array_add_at(a, (void*) 4, 3))  // Add at the end

    void* get_result;
    ASSERT_CC_OK(cc_array_get_at(a, 0, &get_result))
    ASSERT_EQ(1, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 1, &get_result))
    ASSERT_EQ(2, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 2, &get_result))
    ASSERT_EQ(3, (int) (intptr_t) get_result)
    ASSERT_CC_OK(cc_array_get_at(a, 3, &get_result))
    ASSERT_EQ(4, (int) (intptr_t) get_result)

    cc_array_destroy(a);
    return true;
}

// Test adding an element at an index greater than the size of the array
bool test_add_at_index_greater_than_size() {
    CC_Array* a;
    ASSERT_CC_OK(cc_array_new(&a))

    enum cc_stat status = cc_array_add_at(a, (void*) 1, 1);
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, status)

    cc_array_destroy(a);
    return true;
}

bool test_iter_remove_mutation() {
    CC_Array *ar;
    int data = 42; // Dados fictícios
    void *removed_element;

    // Inicializa o array e adiciona um elemento
    ASSERT_CC_OK(cc_array_new(&ar));
    ASSERT_CC_OK(cc_array_add(ar, (void *)&data));

    // Inicializa o iterador e avança uma posição
    CC_ArrayIter iter;
    cc_array_iter_init(&iter, ar);
    cc_array_iter_next(&iter, NULL);

    // Tenta remover o elemento com índice calculado incorretamente
    ASSERT_EQ(cc_array_iter_remove(&iter, &removed_element), CC_ERR_OUT_OF_RANGE);

    cc_array_destroy(ar);
    return true;
}

bool test_cc_array_zip_iter_next_mutant() {
    CC_Array *ar1, *ar2;
    CC_ArrayZipIter iter;
    int data1[] = {1, 2, 3};
    int data2[] = {4, 5};

    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    for (int i = 0; i < sizeof(data1) / sizeof(data1[0]); ++i) {
        ASSERT_CC_OK(cc_array_add(ar1, &data1[i]));
    }

    for (int i = 0; i < sizeof(data2) / sizeof(data2[0]); ++i) {
        ASSERT_CC_OK(cc_array_add(ar2, &data2[i]));
    }

    cc_array_zip_iter_init(&iter, ar1, ar2);

    void *out1, *out2;
    ASSERT_EQ(cc_array_zip_iter_next(&iter, &out1, &out2), CC_OK);
    ASSERT_EQ(*(int *)out1, 1);
    ASSERT_EQ(*(int *)out2, 4);

    ASSERT_EQ(cc_array_zip_iter_next(&iter, &out1, &out2), CC_OK);
    ASSERT_EQ(*(int *)out1, 2);
    ASSERT_EQ(*(int *)out2, 5);

    ASSERT_EQ(cc_array_zip_iter_next(&iter, &out1, &out2), CC_ITER_END);

    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

bool test_cc_array_zip_iter_remove_mutants() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona alguns elementos aos arrays
    int data1[] = {1, 2, 3};
    int data2[] = {4, 5, 6};
    for (size_t i = 0; i < sizeof(data1) / sizeof(data1[0]); i++) {
        ASSERT_CC_OK(cc_array_add(ar1, &data1[i]));
        ASSERT_CC_OK(cc_array_add(ar2, &data2[i]));
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Move o iterador para um índice adequado para testar os mutantes
    iter.index = 5; // O índice 5 é fora do intervalo de ar1 e ar2

    // Testa os mutantes
    void *out1, *out2;
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, cc_array_zip_iter_remove(&iter, &out1, &out2)); // Este teste deve matar o mutante (iter->index - 10) >= iter->ar1->size
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, cc_array_zip_iter_remove(&iter, &out1, &out2)); // Este teste deve matar o mutante (iter->index - 1) >= iter->ar1->size
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, cc_array_zip_iter_remove(&iter, &out1, &out2)); // Este teste deve matar o mutante (iter->index - 1) >= iter->ar2->size

    cc_array_destroy(ar1);
    cc_array_destroy(ar2);
    return true;
}

bool test_cc_array_zip_iter_remove_mutant() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona alguns elementos aos arrays
    int data1[] = {1, 2, 3};
    int data2[] = {4, 5, 6};
    for (size_t i = 0; i < sizeof(data1) / sizeof(data1[0]); i++) {
        ASSERT_CC_OK(cc_array_add(ar1, &data1[i]));
        ASSERT_CC_OK(cc_array_add(ar2, &data2[i]));
    }

    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Move o iterador para um índice onde a operação de módulo é diferente de zero
    iter.index = 4;

    // Testa o mutante iter->index % 1
    void *out1, *out2;
    ASSERT_EQ(CC_ERR_OUT_OF_RANGE, cc_array_zip_iter_remove(&iter, &out1, &out2)); // Este teste deve matar o mutante iter->index % 1

    cc_array_destroy(ar1);
    cc_array_destroy(ar2);
    return true;
}

// Teste para matar o mutante (iter->index + 1) >= iter->ar1->size || (iter->index - 1) >= iter->ar2->size
bool test_iter_remove_mutant_1() {
    // Configuração
    CC_ArrayZipIter iter;
    CC_Array ar1, ar2;
    CC_Array *ptr_ar1 = &ar1; // Ponteiro para o array 1
    CC_Array *ptr_ar2 = &ar2; // Ponteiro para o array 2
    ASSERT_CC_OK(cc_array_new(&ptr_ar1)) // Passa o endereço do ponteiro para o array 1
    ASSERT_CC_OK(cc_array_new(&ptr_ar2)) // Passa o endereço do ponteiro para o array 2
    cc_array_zip_iter_init(&iter, ptr_ar1, ptr_ar2); // Passa os ponteiros dos arrays

    // Mock do tamanho do array 1
    ar1.size = 5;
    // Mock do tamanho do array 2
    ar2.size = 4;
    // Mock do índice do iterador
    iter.index = 3;

    // Execução do teste
    void *out1, *out2;
    enum cc_stat result = cc_array_zip_iter_remove(&iter, &out1, &out2);

    // Verificação do resultado
    ASSERT_EQ(result, CC_OK);

    // Limpeza
    cc_array_destroy(ptr_ar1);
    cc_array_destroy(ptr_ar2);
    return true;
}

// Teste para matar o mutante (iter->index - 1) >= iter->ar1->size || (iter->index - 1) >= iter->ar2->size
bool test_iter_remove_mutant_2() {
    // Configuração
    CC_ArrayZipIter iter;
    CC_Array ar1, ar2;
    CC_Array *ptr_ar1 = &ar1; // Ponteiro para o array 1
    CC_Array *ptr_ar2 = &ar2; // Ponteiro para o array 2
    ASSERT_CC_OK(cc_array_new(&ptr_ar1)) // Passa o endereço do ponteiro para o array 1
    ASSERT_CC_OK(cc_array_new(&ptr_ar2)) // Passa o endereço do ponteiro para o array 2
    cc_array_zip_iter_init(&iter, ptr_ar1, ptr_ar2); // Passa os ponteiros dos arrays

    // Mock do tamanho do array 1
    ar1.size = 5;
    // Mock do tamanho do array 2
    ar2.size = 4;
    // Mock do índice do iterador
    iter.index = 0;

    // Execução do teste
    void *out1, *out2;
    enum cc_stat result = cc_array_zip_iter_remove(&iter, &out1, &out2);

    // Verificação do resultado
    ASSERT_EQ(result, CC_OK);

    // Limpeza
    cc_array_destroy(ptr_ar1);
    cc_array_destroy(ptr_ar2);
    return true;
}

bool test_mutant_1() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1));
    ASSERT_CC_OK(cc_array_add(ar2, (void *)1));

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

bool test_mutant_2() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1));
    ASSERT_CC_OK(cc_array_add(ar2, (void *)1));

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

bool test_mutant_3() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1));
    ASSERT_CC_OK(cc_array_add(ar2, (void *)1));

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

bool test_mutant_4() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1));
    ASSERT_CC_OK(cc_array_add(ar2, (void *)1));

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

bool test_mutant_5() {
    CC_Array *ar1, *ar2;
    ASSERT_CC_OK(cc_array_new(&ar1));
    ASSERT_CC_OK(cc_array_new(&ar2));

    // Adiciona elementos às arrays
    ASSERT_CC_OK(cc_array_add(ar1, (void *)1));
    ASSERT_CC_OK(cc_array_add(ar2, (void *)1));

    // Inicializa o iterador zip
    CC_ArrayZipIter iter;
    cc_array_zip_iter_init(&iter, ar1, ar2);

    // Avança o iterador zip
    ASSERT_CC_OK(cc_array_zip_iter_next(&iter, NULL, NULL));

    // Substitui o último par de elementos retornados
    void *replaced1, *replaced2;
    enum cc_stat status = cc_array_zip_iter_replace(&iter, (void *)10, (void *)20, &replaced1, &replaced2);

    // Verifica se o status retornado é CC_ERR_OUT_OF_RANGE
    ASSERT_EQ(status, CC_ERR_OUT_OF_RANGE);

    // Destrói as arrays
    cc_array_destroy(ar1);
    cc_array_destroy(ar2);

    return true;
}

test_t TESTS[] = {
    &test_cc_array_new_conf_valid_conf,
    &test_cc_array_new_conf_exp_factor_default,
    &test_cc_array_new_conf_invalid_capacity_zero,
    &test_cc_array_new_conf_invalid_capacity_large_exp_factor,
    &test_cc_array_new_conf_alloc_failure_array,
    &test_cc_array_new_conf_alloc_failure_buffer,
    &test_cc_array_add_with_sufficient_capacity,
    &test_cc_array_add_with_expansion,
    &test_cc_array_add_expand_failure,
    &test_cc_array_add_max_capacity,
    &test_cc_array_add_at_empty_array_start,
    &test_cc_array_add_at_empty_array_end,
    &test_cc_array_add_at_empty_array_middle,
    &test_cc_array_add_at_out_of_range_upper,
    &test_cc_array_add_at_out_of_range_lower,
    &test_cc_array_add_at_with_sufficient_capacity,
    &test_cc_array_add_at_expand_failure,
    &test_cc_array_remove_existing_element,
    &test_cc_array_remove_nonexistent_element,
    &test_cc_array_remove_from_empty_array,
    &test_cc_array_remove_from_array_with_multiple_elements,
    &test_cc_array_remove_last_element,
    &test_cc_array_remove_at_valid_index,
    &test_cc_array_remove_at_index_out_of_range,
    &test_cc_array_subarray_buffer_allocation_success,
    &test_cc_array_subarray_buffer_allocation_failure,
    &test_cc_array_filter_mut_non_empty_array,
    &test_cc_array_filter_mut_empty_array,
    &test_cc_array_filter_mut_single_element_array,
    &test_cc_array_filter_non_empty_array,
    &test_cc_array_filter_empty_array,
    &test_cc_array_filter_single_element,
    &test_cc_array_reverse_non_empty_array,
    &test_cc_array_reverse_empty_array,
    &test_cc_array_reverse_single_element,
    &test_cc_array_trim_capacity_capacity_greater_than_size,
    &test_cc_array_trim_capacity_capacity_equal_to_size,
    &test_cc_array_trim_capacity_empty_array,
    &test_cc_array_trim_capacity_size_greater_than_capacity,
    &test_cc_array_reduce_size_0,
    &test_cc_array_reduce_size_1,
    &test_cc_array_reduce_size_greater_than_1,
    &test_cc_array_reduce_size_equal_1,
    &test_cc_array_reduce_size_equal_2,
    &test_cc_array_reduce_size_greater_than_2,
    &test_cc_array_iter_replace_same_element,
    &test_cc_array_iter_replace_different_element,
    &test_cc_array_iter_replace_out_of_range,
    &test_cc_array_zip_iter_remove_success,
    &test_cc_array_zip_iter_remove_success_one_element,
    &test_cc_array_zip_iter_remove_success_empty_array,
    &test_cc_array_zip_iter_add_success,
    &test_cc_array_zip_iter_add_failure_alloc,
    &test_cc_array_zip_iter_replace_success,
    &test_cc_array_zip_iter_replace_failure_out_of_range,
    &test_cc_array_zip_iter_replace_failure_invalid_iterator,
    &test_cc_array_zip_iter_replace_success_check_replacement_elements,
    &test_cc_array_zip_iter_replace_success_check_elements,
    &test_cc_array_new_conf_exp_factor_equal_1,
    &test_cc_array_new_conf_exp_factor_equal_0,
    &test_cc_array_add_status_not_greater_than_CC_OK,
    &test_cc_array_add_pre_increment_size,
    &test_cc_array_add_at_index_out_of_range,
    &test_cc_array_add_at_max_index,
    &test_cc_array_add_at_with_index_zero_in_empty_array,
    &test_expand_capacity_called_when_size_greater_than_capacity,
    &test_expand_capacity_not_called_when_size_less_than_capacity,
    &test_cc_array_replace_at_out_of_range_index,
    &test_cc_array_swap_at_out_of_range_index1,
    &test_cc_array_swap_at_out_of_range_index2,
    &test_cc_array_remove_not_found,
    &test_cc_array_remove_last_element1,
    &test_cc_array_remove_at_assignment,
    &test_cc_array_remove_at_division_by_one,
    &test_cc_array_remove_at_subtract_zero,
    &test_cc_array_remove_at_index_comparison,
    &test_cc_array_get_last_empty_array,
    &test_cc_array_subarray_mutant,
    &test_cc_array_filter_mut_negative_size,
    &test_cc_array_trim_capacity,
    &test_cc_array_add_at_invalid_index_negative,
    &test_cc_array_add_at_invalid_index_zero,
    &test_cc_array_add_at_invalid_index_equal_to_size,
    &test_cc_array_add_at_invalid_index_larger_than_size,
    &test_cc_array_add_at_invalid_index_in_middle,
    &test_cc_array_swap_at_invalid_index1,
    &test_cc_array_swap_at_invalid_index2,
    &test_cc_array_remove_index_1,
    &test_cc_array_remove_index_2,
    &test_cc_array_remove_index_3,
    &test_cc_array_remove_index_4,
    &test_remove_at_index_3_size_3,
    &test_remove_at_index_5_size_3,
    &test_remove_at_index_3_size_5,
    &test_remove_at_index_3_size_3,
    &test_remove_at_index_5_size_3,
    &test_remove_at_index_3_size_5,
    &test_remove_at_index_4_size_3,
    &test_remove_at_index_7_size_6,
    &test_remove_at_index_5_size_6,
    &test_get_at_index_3_size_1,
    &test_get_last_size_minus_1,
    &test_reverse_size_1,
    &test_reverse_size_0,
    &test_reverse_size_minus_1,
    &test_reduce_size_2,
    &test_reduce_size_1,
    &test_reduce_size_0,
    &test_reduce_size_minus_1,
    &new_test,
    &test_add_at,
    &test_remove,
    &test_replace_at,
    &test_trim_capacity,
    &test_copy_deep,
    &test_sort,
    &test_filter,
    &test_reverse,
    &test_remove_all_free,
    &test_iter_remove,
    &test_subarray,
    &test_iter_add,
    &test_iter_replace,
    &test_zip_iter_next,
    &test_zip_iter_add,
    &test_zip_iter_remove,
    &test_reduce,
    &test_map,
    &test_contains,
    &test_index_of,
    &test_size_capacity,
    &test_get_last,
    &test_iterate,
    &test_iter_replace_positions,
    &test_subarray_ranges,
    &test_remove_last,
    &test_zip_iter_replace,
    &test_zip_iter_index,
    &test_filter_mut,
    &test_contains_value,
    &test_copy_shallow,
    &test_swap_at,
    &test_remove_all,
    &test_iter_index,
    &test_zip_iter_add_parallel,
    &test_reduce_multiplication,
    &test_get_at,
    &test_zip_iter_remove_extended,
    &test_contains_duplicates,
    &test_sort_float,
    &test_map_decrement,
    &test_reduce_subtraction,
    &test_iter_add_multiple,
    &test_zip_iter_next_different_sizes,
    &test_get_last_empty,
    &test_iter_remove_alternate,
    &test_zip_iter_add_multiple,
    &test_reverse_different_sizes,
    &test_sort_string,
    &test_contains_person,
    &test_zip_iter_remove_different_sizes,
    &test_contains_repeated,
    &test_subarray_invalid_indices,
    &test_trim_capacity_after_operations,
    &test_exp_factor_leq_1,
    &test_exp_factor_gt_1,
    &test_invalid_capacity,
    &test_valid_capacity_and_exp_factor,
    &test_exp_factor_causes_overflow,
    &test_iter_add_at_beginning,
    &test_get_buffer,
    &test_swap_at_out_of_bounds,
    &test_reverse_odd_size,
    &test_filter_mut_no_removal,
    &test_add_at_empty_array,
    &test_add_at_last_position,
    &test_add_at_middle_position,
    &test_add_at_out_of_bounds,
    &test_add_at_full_array,
    &test_add_at_index_greater_than_size_minus_one,
    &test_add_at_beginning,
    &test_add_at_middle,
    &test_add_at_end,
    &test_add_at_index_greater_than_size,
    test_iter_remove_mutation,
    test_cc_array_zip_iter_next_mutant,
    test_cc_array_zip_iter_remove_mutants,
    test_cc_array_zip_iter_remove_mutant,
    test_iter_remove_mutant_1,
    test_iter_remove_mutant_2,
    test_mutant_1,
    test_mutant_2,
    test_mutant_3,
    test_mutant_4,
    test_mutant_5,
    NULL
};