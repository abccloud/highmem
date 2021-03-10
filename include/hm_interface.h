#ifndef  __HM_INTERFACE_H__
#define  __HM_INTERFACE_H__

#include <stdint.h>
/*if you want redefine uthash_malloc,uthash_free, you should define before ut[xxx].h*/

#define   INDEX_T   uint32_t

#define   MAX_HASH_INDEX           8

#define  HASH_TABLE_ARRY_SIZE      24  /*I have 1<<HASH_TABLE_ARRY_SIZE hash_array*/

#define   LITTLE_GAP               16

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

#pragma pack (8)
typedef struct
{
    int thread_id;
    int table_type;
    value_list_t *free_value_list;

    index_hash_head_t *hash_array_ptr[MAX_HASH_INDEX];
    
    uint32_t v_item_size;
    uint32_t v_item_size_real;
    uint32_t v_num;
    uint8_t *p_value;
}hm_table_handle_t;
#pragma pack ()

#define hm_malloc(sz)       malloc(sz)
#define hm_calloc(sz,num)   calloc(sz,num)
#define hm_free(ptr)        free(ptr)

#define hs_lh_malloc(sz)       malloc(sz)
#define hs_lh_calloc(sz,num)   calloc(sz,num)
#define hs_lh_free(ptr)        free(ptr)


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
extern int update_index(hm_table_handle_t *handle,int index_type,int key_offset,int key_len);
extern int add_value(hm_table_handle_t *handle,void *p_item);
extern int find_from_key(hm_table_handle_t *handle,uint32_t index_ra,void *key,int *rtcount,hash_list_node_t **rtlist);
extern int delete_value(hm_table_handle_t *handle,void *p_value);
extern int free_rt_list(hash_list_node_t *rtlist);
extern int show_rt_list(hash_list_node_t *rtlist);
extern int calc_table_size(uint32_t max_items,uint32_t item_size);

#define hm_printf   printf

#endif
