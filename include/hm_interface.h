#ifndef  __HM_INTERFACE_H__
#define  __HM_INTERFACE_H__

#include <stdint.h>

#define   MAX_HASH_INDEX           8

#define  HASH_TABLE_ARRY_SIZE      23  /*I have 1<<HASH_TABLE_ARRY_SIZE hash_array*/

#define   LITTLE_GAP               16

#define HASH_NODE_POOL
#define __CHECK__                  0

#ifdef HASH_NODE_POOL
#define HASH_NODE_POOL_STEP   1024
#endif

enum
{
    index_type_unique,
    index_type_multi,
};

#pragma pack (8)
typedef struct __value_list
{
    struct __value_list *next;
}value_list_t;
#pragma pack ()

typedef struct __hash_list_node
{
    void *p_value;
    struct __hash_list_node *next;
}hash_list_node_t;
typedef struct 
{
    uint32_t index_type;
    int k_offset;
    int k_len;
}index_type_key_t;

typedef struct
{
    index_type_key_t key;
    hash_list_node_t *hash_arry[];
}index_hash_head_t;

#ifdef HASH_NODE_POOL
typedef struct
{
    hash_list_node_t sys_malloc_mem;
    hash_list_node_t *pool_list;
}hash_list_node_pool_t;
#endif

#pragma pack (8)
typedef struct
{
    int thread_id;
    int table_type;
    value_list_t *free_value_list;

    index_hash_head_t *hash_array_ptr[MAX_HASH_INDEX];
#ifdef HASH_NODE_POOL
    hash_list_node_pool_t hash_list_node_pool;
#endif
    uint32_t v_item_size;
    uint32_t v_item_size_real;
    uint32_t v_num;
    uint8_t *p_value;
}hm_table_handle_t;
#pragma pack ()

#define hm_malloc(sz)       malloc(sz)
#define hm_calloc(sz,num)   calloc(sz,num)
#define hm_free(ptr)        free(ptr)

#ifndef HASH_NODE_POOL
#define hs_lh_malloc(h,sz)       malloc(sz)
#define hs_lh_calloc(h,sz,num)   calloc(sz,num)
#define hs_lh_free(h,ptr)        free(ptr)
#else
#define hs_lh_malloc(h,sz)       hs_lh_pool_get(h,sz)
#define hs_lh_calloc(h,sz,num)   hs_lh_pool_getzero(h,sz,num)
#define hs_lh_free(h,ptr)        hs_lh_pool_ret(h,ptr)
#endif


/*If base is NULL we will malloc for this table.
    if base is not NULL, and len is enough,we will format it and return base.
    max_items is the max number items you store in this table
    item_size is the size of each item.

    for example:

    hm_table_handle_t *handle = alloc_table(NULL,0,1,512*512,sizeof(struct data));

    or:

    uint32_t len = calc_table_size(512*512,sizeof(struct data));
    void *base = malloc(len);
    if(NULL != base)
    {
        hm_table_handle_t *handle = alloc_table(base,len,1,512*512,sizeof(struct data));
    }
*/
extern hm_table_handle_t *alloc_table(void *base,uint32_t len,uint32_t thread_id,uint32_t table_type,uint32_t max_items,uint32_t item_size);
/*after alloc_table then you may use update_index,the first index should unique*/
extern int update_index(hm_table_handle_t *handle,int index_type,int key_offset,int key_len);

/*if you have a value want to add to this table,you should use add_value*/
extern int add_value(hm_table_handle_t *handle,void *p_item);

/*get a free item node, we can store value on this directly*/
extern void *get_afree_item(hm_table_handle_t *handle);
/*return a free item node, we donot want to store value on it*/
extern int ret_afree_item(hm_table_handle_t *handle,void *p_item);
/*after fill the free item and add to table use add_value_from_afree*/
extern int add_value_from_afree(hm_table_handle_t *handle,void *p_item);

extern int find_from_key(hm_table_handle_t *handle,uint32_t index_ra,void *key,int *rtcount,hash_list_node_t **rtlist);
extern int delete_value(hm_table_handle_t *handle,void *p_value);
extern int free_rt_list(hm_table_handle_t *handle,hash_list_node_t *rtlist);
extern int show_rt_list(hash_list_node_t *rtlist);
extern int calc_table_size(uint32_t max_items,uint32_t item_size);

#define hm_printf   printf

#endif
