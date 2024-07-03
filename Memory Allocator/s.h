// include other header files as needed
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

// mems_malloc will be extremely complicated .
// realigning a partial node is also difficult -- to add two nodes .
    static unsigned long  MeMS  ; 


    typedef struct free_node{
    
    unsigned long size ;                                        // (size & (PAGE_SIZE - 1)) +  1 -> argument to mmap .
    unsigned long v_addr ;                                      //  I will have to check , I think v_addr will be typecasted to (void*) and simply returned because we are starting from 0 and it is augmented by us .
    void * p_addr ;                                             //  malloc will return memsz physical address(a virtual address) 
                                                                //  we will do something like curr->p_addr =  mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
    int flag ;                                                  //  0 means PROCESS , 1 means HOlES
    struct free_node * next ;
    struct free_node * prev ;
    struct sub_node * sub_list ;                                // corresponding to each node of the free list , there  is a sub_list 
    unsigned long unused_size  ;                                // Initially the nodes in the sub-chain will not be allocated simultaneously , so there might be a case where say one node is allocated 
                                                                // and free_node->size - 1st node->size is not 0 , so we will have to keep track of that .
                                                                // If this is greater than the size demanded by user program , we allocate memory here .

    } free_node ; 
    // all of this will be initialized by malloc accordingly .  v_addr will be like  (prev->v_addr + prev->size ) . 
    //** free_node->unused_size   will be initially = free_node->size .


    free_node * head ;    

    /*
    So ... mmap will be done always only when the user program demands a memory and that cannot be satisfied by the free_node .
    And , then it will always be the case that a particular size of that mmaped region will be allocated to the user program 

    Something like free_node.size > user_program , create_two_nodes() which returns the v_addr of the user_program allocation .
    I think will there will have to be distinction between first and the rest of the nodes in the sub_list .

    and the rest will be added to the free_node as "HOLES"
    So , HOlE node is automatically created as a constraint .

    */


    /* 
    malloc -
    If head == NULL , then new sublist->prev = NULL 
    new sublist->next = NULL 
    new sublist->size = size 
    new sublist->v_addr = 0 
    new sublist->p_addr = mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0)
    new sublist->flag = 0 , new sublist->parent = NULL 
    new sublist->unused_size = size , head = new sublist 
    return new sublist->v_addr ;


    enter free_node - enter its sub_list ; check size of sub_list HOLE node >= size demanded by user program , if yes....
    In the sub-list , continue until sub_list-> next == NULL , check parent->unused_size , if it is greater than the size demanded by user program ....
    If we reach the end of free_node , then   
    free_node * new ;
    new = malloc(sizeof(free_node));
    new->prev = curr ; 
    curr->next = new ;
    new->next = NULL ;
    new->size = (size & ~(PAGE_SIZE - 1) )+ 1 ;  check 
    new->v_addr = curr->v_addr + curr->size ;  
    // check the best way to do it .
    new->p_addr = mmap(NULL,(size & ~(PAGE_SIZE - 1)) + 1 ,  PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);

    Now , whenever user_demanded < free_node-> unused_size , there will be two nodes in the sub_list , one will be the node that will be returned to the user program  with flag =  1 (PROCESS)
    the other will be the node that will be added to the free_node with flag == 0 . ( HOLES ). 
    *****
    Therefore , I think it is a  very good idea to initialize the first node of the sub_list with the size of the free_node->size . 
    . This removes the need for the unused_size variable .
    As the unused_size will now be represented by HOLE next to the last PROCESS node in the sub_list .
    I need to be very cautious will implementing the create_sub_list function .
    -- Check whether split in two . make two nodes , Initialize  them properly . set double pointers properly .

    ******
    Then we will have to distribute this in two nodes , one will be the node that will be returned to the user program  with flag =  1 (PROCESS)
     the other will be the node that will be added to the free_node with flag == 0 . ( HOLES )
    -- prev should be initialized first by malloc 
    -- then size , maybe depending on the size  demanded by the user program .
    -- 

    */


typedef struct sub_node
{

    unsigned long size ;
    unsigned long v_addr ;
    int flag  ;
    struct sub_node * next ;
    struct sub_node * prev ;
    struct free_node * parent ;                                 //  this will point to the parent node of the free list .


} sub_node ; 