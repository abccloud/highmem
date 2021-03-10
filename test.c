#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <assert.h>
#include "hm_interface.h"

#define NAME_L     24

struct student_t
{
    int id;
    char name[NAME_L];
    int age;
};
#define  MEM_SIZE(type, member)   sizeof(((type*)0)->member)

#define V_NUM_TST    (1024*1024)
#define ITEM_SIZE_TST  (sizeof(struct student_t))

char *jims_none = "jims_none";
char *jims_many = "jims_what";
int Jims_num;

struct student_t *new_stu(int id,char *name,int age)
{
    struct student_t *nstu = calloc(sizeof(struct student_t),1);
    nstu->id = id;
    nstu->age = age;
    strncpy(nstu->name,name,sizeof(nstu->name));
    return nstu;
}
void free_stu(struct student_t *pstu)
{
    free(pstu);
}
int test_find_uni(hm_table_handle_t * handle)
{
    int index_ra = 0;
    int rc = 0;
    int ii = 0;
    int rtcount = 0;
    int key = 0;
    hash_list_node_t *listfind=NULL;
    hash_list_node_t *listfind_tmp=NULL;
    struct student_t *pstu = NULL;
    for(ii=0;ii<V_NUM_TST+1;ii++)
    {
        key = ii+1;
        rc = find_from_key(handle,index_ra,&key,&rtcount,&listfind);
        assert(0==rc);
        if(ii < V_NUM_TST)
        {
            assert(rtcount == 1);
            assert(NULL != listfind);
        }
        else
        {
            assert(rtcount == 0);
            assert(NULL == listfind);
        }
        listfind_tmp = listfind;
        while(NULL != listfind_tmp)
        {
            pstu = listfind_tmp->p_value;
            listfind_tmp = listfind_tmp->next;
        }
        free_rt_list(listfind);
    }
    return 0;
}
int test_find_multi(hm_table_handle_t * handle,int index_ra)
{
    int rc = 0;
    int rtcount = 0;
    char key[NAME_L]={0};
    hash_list_node_t *listfind=NULL;
    hash_list_node_t *listfind_tmp=NULL;
    struct student_t *pstu = NULL;
    memset(key,0,NAME_L);
    strcpy(key,jims_many);

    rc = find_from_key(handle,index_ra,key,&rtcount,&listfind);
    assert(0 == rc);
    hm_printf("%d=%d\n",Jims_num,rtcount);
    assert(Jims_num == rtcount);
    assert(NULL != listfind);
    listfind_tmp = listfind;
    while(NULL != listfind_tmp)
    {
        pstu = listfind_tmp->p_value;
        hm_printf("list:(%s=%d %s %d),rt=%d,rc=%d\n",key,pstu->id,pstu->name,pstu->age,rtcount,rc);
        listfind_tmp = listfind_tmp->next;
    }
    free_rt_list(listfind);

    memset(key,0,NAME_L);
    strcpy(key,jims_none);

    listfind = NULL;
    rtcount = 0;
    rc = find_from_key(handle,index_ra,key,&rtcount,&listfind);
    assert(0 == rc);
    assert(0 == rtcount);
    assert(NULL == listfind);
    return 0;
}
int test_have_this_node(hm_table_handle_t * handle,int index_ra,struct student_t * pstu,int flag)
{
    int rc = 0;
    int rtcount = 0;
    hash_list_node_t *listfind=NULL;
    hash_list_node_t *listfind_tmp=NULL;
    if(index_type_unique == index_ra)
    {
        rc = find_from_key(handle,index_ra,&pstu->id,&rtcount,&listfind);
        if(0!=rc)
        {
            return -1;
        }
        if(0==rtcount)
        {
            assert(NULL == listfind);
            return 0;
        }
        assert(1==rtcount);
        if(flag)
        {
            if(memcmp(listfind->p_value,pstu,handle->v_item_size)!=0)
            {
                return -3;
            }
        }
        else
        {
            if((uint64_t)listfind->p_value != (uint64_t)pstu)
            {
                return -3;
            }
        }
        if(NULL != listfind->next)
        {
            return -4;
        }
        free_rt_list(listfind);
        return 1;
    }
    else if(index_type_multi== index_ra)
    {
        rc = find_from_key(handle,index_ra,&pstu->name,&rtcount,&listfind);
        if(0!=rc)
        {
            return -1;
        }
        if(1!=rtcount)
        {
            return -2;
        }

        listfind_tmp = listfind;
        while(NULL != listfind_tmp)
        {
            if(flag)
            {
                if(memcmp(listfind_tmp->p_value,pstu,handle->v_item_size)==0)
                {
                    break;
                }
            }
            else
            {
                if((uint64_t)listfind->p_value == (uint64_t)pstu)
                {
                    break;
                }
            }
            listfind_tmp = listfind_tmp->next;
        }
        free_rt_list(listfind);
        if(NULL == listfind_tmp)
        {
            return 0;
        }
        return 1;
    }
    return -1;
}

int test_add(hm_table_handle_t * handle)
{
    int ii = 0;
    char name[NAME_L] = {0,};
    struct student_t * pstu;
    srand((unsigned)time(NULL));
    int rc = 0;
    int rand_ii=0;
    int rand_name_len = strlen(jims_many)-1;/*we shorter than this so we can not equal with this*/
    Jims_num = 0;
    for(ii=0;ii<V_NUM_TST;ii++)
    {
        if(rand()%100 == 0)
        {
            Jims_num++;
            strcpy(name,jims_many);
        }
        else
        {
            for(rand_ii=0;rand_ii<rand_name_len;rand_ii++)
            {
                name[rand_ii]= (rand_ii==0?'A':'a') + (rand()%26);
            }
            name[rand_ii] = '\0';
        }
        pstu = new_stu(ii+1,name,(ii+20)%100);
        assert(NULL != pstu);
        rc = add_value(handle,pstu);
        if(rc < 0)
        {
            hm_printf("add_fail:%d %s %d,rc=%d\n",pstu->id,pstu->name,pstu->age,rc);
        }
        rc = test_have_this_node(handle,index_type_unique,pstu,1);
        assert(1 == rc);
        free_stu(pstu);
    }
    return ii;
}
int test_del(hm_table_handle_t * handle)
{
    int index_ra = 0;
    int rc = 0;
    int ii = 0;
    int rtcount = 0;
    int key = 0;
    hash_list_node_t *listfind=NULL;
    hash_list_node_t *listfind_tmp=NULL;
    struct student_t *pstu = NULL;

    for(ii=0;ii<V_NUM_TST+1;ii++)
    {
        key = ii+1;
        rc = find_from_key(handle,index_ra,&key,&rtcount,&listfind);
        listfind_tmp = listfind;
        while(NULL != listfind_tmp)
        {
            pstu = listfind_tmp->p_value;
            delete_value(handle,pstu);
            assert(0 == test_have_this_node(handle,index_type_unique,pstu,0));
            listfind_tmp = listfind_tmp->next;
        }
        free_rt_list(listfind);
    }
    return ii;
}

int main(int arc,char *argv[])
{
    int len = calc_table_size(V_NUM_TST,ITEM_SIZE_TST);
    void *base = malloc(len);
    if(NULL == base)
    {
        hm_printf("error malloc(%d)\n",len);
        return -1;
    }
    hm_table_handle_t * handle = alloc_table(base,len,1,0,V_NUM_TST,ITEM_SIZE_TST);
    if(NULL != handle)
    {
        hm_printf("alloc_table OK\n");
    }
    int index0= update_index(handle,index_type_unique,offsetof(struct student_t,id),MEM_SIZE(struct student_t,id)); 
    int index1= update_index(handle,index_type_multi,offsetof(struct student_t,name),MEM_SIZE(struct student_t,name));
    assert((0==index0)&&(1==index1));

    /*you should first update_index and then use add del and find, after you use add, you cannot use update_index*/
    
    test_add(handle);
    test_del(handle);
    test_add(handle);
    test_find_uni(handle);
    test_find_multi(handle,index_type_multi);
    free(base);
    return 0;
}

