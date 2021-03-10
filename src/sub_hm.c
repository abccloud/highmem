
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hm_interface.h"
#include "sub_hm.h"

int add_list_add_boom_size(uint32_t item_size,int align)
{
    item_size = sizeof(value_list_t)+item_size;
    int tmp = item_size%align;
    if(0 != tmp)
    {
        item_size += align-tmp;
    }
    return item_size;
}
int calc_table_size(uint32_t max_items,uint32_t item_size)
{
    int head_len = sizeof(hm_table_handle_t);
    uint32_t item_size_real = add_list_add_boom_size(item_size,8);
    int value_erea_len = max_items*item_size_real;
    return head_len+value_erea_len;
}
hm_table_handle_t *alloc_table(void *base,uint32_t len,uint32_t thread_id,uint32_t table_type,uint32_t max_items,uint32_t item_size)
{
    uint8_t *p_tmp;
    value_list_t *p_value_list_node;
    int need_len = calc_table_size(max_items,item_size);
    if(NULL != base)
    {
        if(len < need_len)
        {
            return NULL;
        }
    }
    else
    {
        len = need_len;
        base = hm_malloc(len);
        if(NULL == base)
        {
            return NULL;
        }
    }
    hm_table_handle_t *handle = base;
    handle->thread_id = thread_id;
    handle->table_type = table_type;
    handle->v_item_size = item_size;
    handle->v_item_size_real = add_list_add_boom_size(item_size,8);
    handle->free_value_list = NULL;
    handle->v_num = max_items;
    handle->p_value = ((uint8_t *)handle)+sizeof(hm_table_handle_t);


    int ii = 0;
    for(ii=max_items-1;ii>=0;ii--)
    {
        p_value_list_node = (value_list_t *)(handle->p_value + ii*handle->v_item_size_real);
        p_value_list_node->next = handle->free_value_list;
        handle->free_value_list = p_value_list_node;
    }
    for(ii=0;ii<MAX_HASH_INDEX;ii++)
    {
        handle->hash_array_ptr[ii] = NULL;
    }
    return handle;
}
int update_index(hm_table_handle_t *handle,int index_type,int key_offset,int key_len)
{
    uint8_t *p_tmp;
    int index_ra0 = 0;
    if(NULL == handle)
    {
        return -1;
    }
    if((index_type<index_type_unique)||(index_type>index_type_multi))
    {
        return -2;
    }
    if((key_offset < 0)||(key_len <= 0)||(key_offset+key_len>handle->v_item_size))
    {
        return -3;
    }
    int ii = 0;
    /*find the hash index place*/
    for(ii=0;ii<MAX_HASH_INDEX;ii++)
    {
        /*hash_head is NULL  so this index is empty*/
        if(NULL == handle->hash_array_ptr[ii])
        {
            break;
        }
    }
    if(ii>=MAX_HASH_INDEX)
    {
        return -4;
    }
    index_ra0 = ii;
    /*the first index lxr should be unique index,others can use index_type_multi type index*/
    if((0 == index_ra0)&&(index_type_unique != index_type))
    {
        return -5;
    }
    handle->hash_array_ptr[index_ra0] = hs_lh_calloc(sizeof(index_hash_head_t)+hashsize(HASH_TABLE_ARRY_SIZE)*sizeof(hash_list_node_t *),1);
    if(NULL == handle->hash_array_ptr[index_ra0])
    {
        return -6;
    }
    
    handle->hash_array_ptr[index_ra0]->key.index_type = index_type;
    handle->hash_array_ptr[index_ra0]->key.k_offset = key_offset;
    handle->hash_array_ptr[index_ra0]->key.k_len = key_len;
    return index_ra0;
}
int delete_index_for_item_line(hm_table_handle_t *handle,void *p_item,int index_ra)
{
    index_hash_head_t * hash_head = handle->hash_array_ptr[index_ra];
    if(NULL == hash_head)
    {
        return 0;
    }
    uint8_t *p_tmp = NULL;
    uint8_t *p_key = p_item+hash_head->key.k_offset;
    uint32_t hash_key = hashlittle(p_key,hash_head->key.k_len,0)&hashmask(HASH_TABLE_ARRY_SIZE);
    hash_list_node_t *node_list = hash_head->hash_arry[hash_key];
    hash_list_node_t *node_tmp = NULL;
    hash_list_node_t *node_pre = NULL;
    while(NULL != node_list)
    {
        p_tmp = node_list->p_value;
        if((uint64_t)p_item == (uint64_t)p_tmp)
        {
            node_tmp = node_list->next;
            if(hash_head->hash_arry[hash_key] == node_list)/*if you are the head*/
            {
                hash_head->hash_arry[hash_key] = node_tmp;
            }
            else
            {
                node_pre->next = node_tmp;
            }
            hs_lh_free(node_list);
            break;
        }
        node_pre = node_list;
        node_list = node_list->next;
    }
    return 0;
}

int build_index_for_item_line(hm_table_handle_t *handle,void *p_item,int index_ra)
{
    index_hash_head_t * hash_head = handle->hash_array_ptr[index_ra];
    if(NULL == hash_head)
    {
        return 0;
    }
    uint8_t *p_key = p_item+hash_head->key.k_offset;
    uint32_t hash_key = hashlittle(p_key,hash_head->key.k_len,0)&hashmask(HASH_TABLE_ARRY_SIZE);
    hash_list_node_t *new_node = hs_lh_malloc(sizeof(hash_list_node_t));
    if(NULL == new_node)
    {
        return -1;
    }
    new_node->p_value = p_item;
    /*insert to head*/
    new_node->next = hash_head->hash_arry[hash_key];
    hash_head->hash_arry[hash_key] = new_node;
    return 0;
}
int build_index_for_item(hm_table_handle_t *handle,void *p_item)
{
    int rc = 0;
    int ii = 0;
    for(ii=0;ii<MAX_HASH_INDEX;ii++)
    {
        rc = build_index_for_item_line(handle,p_item,ii);
        if(0 != rc)
        {
            return 0-ii;
        }
    }
    return 0;
}
int add_value(hm_table_handle_t *handle,void *p_item)
{
#if __CHECK__
    if(NULL == handle)
    {
        return -1;
    }
    if(NULL == p_item)
    {
        return -2;
    }
#endif
    /*get a empty value rom*/
    void *p_e_value = get_afree_item(handle);
    if(NULL == p_e_value)
    {
        return -3;
    }
    memcpy(p_e_value,p_item,handle->v_item_size);
    return add_value_from_afree(handle,p_e_value);
}
void *get_afree_item(hm_table_handle_t *handle)
{
#if __CHECK__
    if(NULL == handle)
    {
        return NULL;
    }
#endif
    /*get a empty value rom*/
    if(NULL == handle->free_value_list)
    {
        return NULL;/*no free item value*/
    }
    value_list_t *p_empty_v = handle->free_value_list;
    handle->free_value_list = handle->free_value_list->next;
    p_empty_v->next = NULL;
    return ((void *)p_empty_v)+sizeof(value_list_t);
}
/*return a free item node, we donot want to store value on it*/
int ret_afree_item(hm_table_handle_t *handle,void *p_item)
{
#if __CHECK__
    if((NULL == handle)||(NULL == p_item))
    {
        return NULL;
    }
#endif

    value_list_t *p_empty_v = p_item-sizeof(value_list_t);
    p_empty_v->next = handle->free_value_list;
    handle->free_value_list = p_empty_v;
    
    return 0;
}
/*after fill the free item and add to table use add_value_from_afree*/
int add_value_from_afree(hm_table_handle_t *handle,void *p_item)
{
    return build_index_for_item(handle,p_item);
}
int find_from_key(hm_table_handle_t *handle,uint32_t index_ra,void *key,int *rtcount,hash_list_node_t **rtlist)
{
#if __CHECK__
    if(NULL == handle)
    {
        return -1;
    }
    if(NULL == key)
    {
        return -2;
    }
#endif
    int rt = 0;
    uint8_t *p_tmp=NULL;

    *rtcount = 0;
    *rtlist = NULL;

    index_hash_head_t * hash_head = handle->hash_array_ptr[index_ra];
    if(NULL == hash_head)
    {
        return 0;
    }
    hash_list_node_t *new_node = NULL;
    uint32_t hash_key = hashlittle(key,hash_head->key.k_len,0)&hashmask(HASH_TABLE_ARRY_SIZE);
    hash_list_node_t *rt_head = NULL;
    hash_list_node_t *this_head = hash_head->hash_arry[hash_key];
    while(NULL != this_head)
    {
        p_tmp = this_head->p_value;
        if(memcmp(key,p_tmp+hash_head->key.k_offset,hash_head->key.k_len)==0)
        {

            if(index_type_unique == hash_head->key.index_type)
            {
                new_node = hs_lh_malloc(sizeof(hash_list_node_t));
                if(NULL != new_node)
                {
                    new_node->next = NULL;
                    new_node->p_value = p_tmp;
                    *rtlist = new_node;
                    *rtcount = 1;
                    rt = 0;
                    break;
                }
                else
                {
                    rt = -3;
                    break;
                }
            }
            else
            {
                new_node = hs_lh_malloc(sizeof(hash_list_node_t));
                if(NULL != new_node)
                {
                    new_node->next = *rtlist;
                    new_node->p_value = p_tmp;
                    *rtlist = new_node;
                    *rtcount += 1;
                }
                else
                {
                    rt = -3;
                    break;
                }
            }
        }
        this_head = this_head->next;
    }
    return rt;
}
int delete_value(hm_table_handle_t *handle,void *p_value)
{
    int rc = 0;
    int ii = 0;
    for(ii=0;ii<MAX_HASH_INDEX;ii++)
    {
        rc = delete_index_for_item_line(handle,p_value,ii);
        if(0 != rc)
        {
            return 0-ii;
        }
    }
    value_list_t *p_v = p_value-sizeof(value_list_t);
    p_v->next = handle->free_value_list;
    handle->free_value_list = p_v;
    return 0;
}
int free_rt_list(hash_list_node_t *rtlist)
{
    hash_list_node_t * pnode = NULL;
    while(NULL != rtlist)
    {
        pnode = rtlist->next;
        hs_lh_free(rtlist);
        rtlist = pnode;
    }
    return 0;
}
int show_rt_list(hash_list_node_t *rtlist)
{
    hm_printf("\n###################\n");
    while(NULL != rtlist)
    {
        hm_printf("p_v(0x%x) ",rtlist->p_value);
        rtlist = rtlist->next;
    }
    hm_printf("\n*******************\n");
    return 0;
}


