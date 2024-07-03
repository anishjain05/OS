/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY

// add other headers as required


Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 

*/

/*

Assumptions :
The page size is a power of 2 ,
which is a justified assumption as we use either the 32 bit or 64 bit architecture so the granular units to encompass the virtual address space will be powers of 2 . 
The head node is first allocated memory when the user first asks for memory . Therefore in mem_init it is initialized to NULL .
*/

#include "s.h"
#define PAGE_SIZE 4096
#define MEMS_VA 1000


/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/

void mems_init(){
    head = NULL ;
    MeMS = MEMS_VA ;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish(){

    free_node * finish = head ;
    while(finish!=NULL)
    {
        munmap(finish->p_addr ,finish->size); //check E1
        finish = finish->next ;
    }   
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 


/*
    Everytime we mmap  ,  we will also have to mmap a free_node and a sub_node to contain the attributes of the memory region .
    This holds true for the sub-chain nodes also . We need to mmap them .
*/
void* mems_malloc(size_t size)
{
    unsigned long reqd_alloc ;
    if(size % PAGE_SIZE == 0){

        reqd_alloc = size ;  
    }
    else{
        reqd_alloc = ( size & ~(PAGE_SIZE - 1) ) + PAGE_SIZE ;
    }
    //printf("SIZE REQD : %lu\n",size) ;
    // If a particular node disintegrates I am submitting the info of the "PROCESS" to the node and allocating space for a  new node which will correspond to the "HOLE" .
    free_node * mems = head ;
    free_node * prev = NULL ;
    if(head == NULL){

        /* mmap a region to accomodate the user demands , store the reference addr in a  free_node ***** for which I also have  to mmap sizeof(free_node) *****     */
        //printf("In head == NULL: %d \n" , i++) ;    

        // error right now , handle exact multiple of PAGE_Size ;                                                                                                                                                              
        void  * p_addr = mmap(NULL, reqd_alloc , PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE| MAP_ANONYMOUS , -1, 0);                          // round up to the nearest multiple of PAGE_SIZE
        if (p_addr == MAP_FAILED){  printf("mmap failed\n") ; return NULL; }

        free_node * new = (free_node *)mmap(NULL, sizeof(free_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);                                   // allocate a free_node to store the attributes of the memory region .
        if (new == MAP_FAILED){ printf("mmap failed\n") ; return NULL;}

                                                                                                                                                                            // Instead of storing its own size , it will store the size of the main-chain node mmaped corresponding to it .
        new->size = reqd_alloc ;
        //printf("SIZE OF MMAP : %lu\n",new->size) ;
        new->v_addr = MeMS;
        //printf("v_Addr in head = null : %lu\n",MeMS) ;
        new->p_addr = p_addr ;
        new->flag = 0 ;
        new->next = NULL ;
        new->prev = NULL ;
        new->sub_list = NULL ;
        new->unused_size = new->size ;                                         

        head = new ;


        sub_node * new1 = (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0); 
        if (new == MAP_FAILED){ printf("mmap failed\n") ; return NULL; }

        //printf("equivalent physical addr : %p\n" , new->p_addr) ;                                            // new->p_addr is the virtual mem 
        new1->flag = 1 ;
        new1->parent = new; 
        new1->size = size ;
        //printf("size of sub_node1 : %lu\n",new1->size) ;
        new1->prev = NULL ;
        new1->v_addr = new->v_addr ;
        new1->next = NULL ;
        new->sub_list = new1 ;
        //printf("4\n") ;
        if(size % PAGE_SIZE != 0){
            sub_node * new2 = (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
            if (new == MAP_FAILED){ printf("mmap failed\n") ; return NULL; }
            // will new2_p_addr be needed ??
            new2->flag = 0 ;
            new2->parent = new;
            new2->size = new->size - size ;
            //printf("size of sub_node2 : %lu\n",new2->size) ;
            new2->prev = new1 ;
            new2->v_addr = new->v_addr + size ;
            new2->next = NULL ;
            new1->next = new2 ;
        }
        return (void*)(new1->v_addr) ;                                                                                                                       // return the virtual address of the user program .
    }
    while(1){                                                                                                                                                       // traverses the outer linked list
            if(mems == NULL){
                break ;
            }
            //printf("5\n") ;
            sub_node * in_mems = mems->sub_list ;
            while(in_mems != NULL){                                                                                                                                 // inner linked list ,  Assumming all the memory mmapped is allocated to either PROCESS or HOLE , by construction *
                //printf("6 here \n") ;
                //printf("flag : %d size : %lu\n", in_mems->flag , in_mems->size);
                if(in_mems->flag == 0 && in_mems->size >= size )
                {
                    //printf("7\n") ;
                    sub_node * new1 =  (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
                    if (new1 == MAP_FAILED){  printf("mmap failed\n") ; return NULL ;  }                                                                               // we know the physical address(v_a) of the main-node and this is offset , the v_a by mmap will be contiguous and it is already allocated . we just have to match it .
                    new1->flag = 1 ;
                    new1->parent = in_mems->parent ;
                    new1->size = size ;
                    new1->prev = in_mems->prev ;
                    in_mems->prev->next = new1 ;
                    new1->next = NULL ;
                    new1->v_addr = in_mems->v_addr ;
                    
                    if(in_mems->size > size){
                        sub_node  * new2 = (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
                        if (new2 == MAP_FAILED){ printf("mmap failed\n") ; return NULL; }

                        new2->flag = 0 ; 
                        new2->parent = in_mems->parent ;
                        new2->size = in_mems->size - size ;
                        //printf("new2 size : %lu\n" , new2->size) ;
                        new2->prev = new1 ;
                        new2->next = NULL ; 
                        new1->next = new2 ;
                        new2->v_addr = in_mems->v_addr + size ;
                        if(in_mems->next != NULL){
                            in_mems->next->prev = new2 ;
                            new2->next = in_mems->next ;
                        }
                    }
                    else{
                        if(in_mems->next != NULL)
                        {
                            in_mems->next->prev = new1 ;
                            new1->next = in_mems->next ;
                        }
                    }                    
                    munmap(in_mems , sizeof(sub_node)) ;                                                                                                // check .
                    return (void*)(new1->v_addr) ;
                }
            //check partitioning  of sub-chain node size , employ helper function to distribute in two nodes , 1. PROCESS  , 2 . HOLE . should be doubly linked .
            in_mems = in_mems->next ;
            //printf("After in sub_chain \n") ;
        }
        //printf("In  the outer loop ") ;
        prev = mems ;
        mems = mems->next ;

    }


    free_node * new = (free_node *)mmap(NULL, sizeof(free_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
    if (new == MAP_FAILED){ printf("mmap failed\n") ;  return NULL;}

    void * p_addr = mmap(NULL, reqd_alloc, PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
    if (p_addr == MAP_FAILED){ printf("mmap failed\n") ; return NULL; }


    new->size = reqd_alloc ;
    new->v_addr = prev->v_addr + prev->size ;
    //printf("MAIN NODE : %lu\n",new->v_addr) ;                                                                                                           // check if this is correct .
    new->p_addr = p_addr ;
    new->flag = 0 ;
    new->next = NULL ;
    new->prev = prev ;
    prev->next = new ;
    new->sub_list = NULL ;
    //new->unused_size = new->size ;                                                                                                               
    sub_node * new1 = (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
    if (new == MAP_FAILED){ printf("mmap failed\n") ; return NULL; }

    new1->flag = 1 ;
    new1->parent = new ;
    new1->size = size ;  
    new1->prev = NULL ;
    new1->v_addr = new->v_addr ;
    new1->next = NULL ;
    new->sub_list = new1 ;
    if(size % PAGE_SIZE != 0){
        sub_node * new2 = (sub_node *)mmap(NULL, sizeof(sub_node), PROT_READ|PROT_WRITE|PROT_EXEC , MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);
        if (new == MAP_FAILED){ printf("mmap failed\n") ;  return NULL; }

        new2->flag = 0 ;
        new2->parent = new ;
        new2->size = new->size - size ;
        new2->prev = new1 ;
        new2->v_addr = new->v_addr + size ; 
        new2->next = NULL ;
        new1->next = new2 ;
    }
    return (void*)(new->v_addr ) ;
}


/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/

void mems_print_stats()
{
    int i = 0 ; 
    int count = 0 ;
    unsigned long  unused = 0 ;
    int main_chain = 0 ; 
    int n = 20 ;
    int * arr = (int *)malloc(sizeof(int) * n  ) ;
    free_node * num_pages = head ;
    free_node * num_pages1 = head ;
    while(num_pages != NULL){                                                                                                         // check if this is correct .
        int free_count = num_pages->size / 4096 ;
        count += free_count ; 
        num_pages = num_pages->next ;
    }


    while(1)
    {
        if(num_pages1 == NULL){
            break ;
        }
        if(i == n){
            n = 2*n ;
            arr = realloc(arr , sizeof(int)*n) ;

        }
        printf("MAIN[%ld : %ld] ->\t",num_pages1->v_addr , num_pages1->v_addr + num_pages1->size -1 ) ;

        sub_node * sub = num_pages1->sub_list ;   // if free_node is not NULL , this can never be null by construction .
        int j = 0 ; 
        while(1)
        {
            if(sub == NULL){
                printf("NULL\n") ; 
                break ;
            }
            if(sub->flag == 0){
                unused += sub->size ;
                printf("H[%ld : %ld ] <-> " , sub->v_addr , sub->v_addr + sub->size -1);
            }
            else{
                printf("P[%ld : %ld] <-> ",sub->v_addr , sub->v_addr + sub->size -1 ) ;
            }
            j++ ; 
            sub = sub->next ;
        }
        arr[i] = j ; 
        i ++ ; 
        num_pages1 = num_pages1->next ;
    }
    printf("Number of pages allocated : %d\n", count) ;
    printf("Number of pages unused : %ld \n", unused ) ;
    printf("Main Chain Length : %d\n", i) ;
    printf("Sub Chain Length : [") ;
    for(int j = 0 ; j < i ; j++){
        if(j == i-1){
            printf("%d]\n" , arr[j]) ;
            break ;
        }
        printf("%d, " , arr[j]) ;
    }


}

// coalescing is not happening properly ! 



/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){

    // This will return any valid augmented MeMS physical address  (the virtual address returned by mmap) corresponding to the virtual address .
    free_node * get = head ;
    while(get != NULL){
        if(get->v_addr <= (unsigned long)(v_ptr) && (unsigned long)(v_ptr) < get->v_addr + get->size){  //check if correcet 
            return (void*)(get->p_addr + (unsigned long)(v_ptr) - get->v_addr);
        }
    }
    printf("Invalid address\n") ;
    return NULL ; 
    
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void *v_ptr){
    // after freeing , check coalescing of prev and next sub-chain nodes. 
    // assuming that v_ptr corresponds to the starting address of a  sub-chain node.
    // You should also check if 
    free_node * free = head ;
    while(free != NULL){
        if(free->v_addr <= (unsigned long)(v_ptr) && (unsigned long)(v_ptr) < free->v_addr + free->size){
            // This corresponds to the main-chain node that contains the sub-chain node that corresponds to v_ptr
            sub_node * sub = free->sub_list ;
            while(1){
                if(sub == NULL){
                    return ;
                }
                if(sub->v_addr == (unsigned long)(v_ptr)){
                        // This corresponds to the sub-chain node that corresponds to v_ptr
                        sub->flag = 0 ;
                        if(sub->prev != NULL && sub->prev->flag == 0){
                            sub->prev->size += sub->size ;
                            sub->prev->next = sub->next ;
                            if(sub->next != NULL){
                                sub->next->prev = sub->prev ;                                                   // back linking 
                            }
                            sub = sub->prev ;
                        }
                        if(sub->next != NULL && sub->next->flag == 0){
                            sub->size += sub->next->size ;
                            sub->next = sub->next->next ;
                            if(sub->next != NULL){
                                sub->next->prev = sub ;                                                         // back linking 
                            }
                        }

                        //check if the main-chain node is empty , if yes , then free->flag = 0 . check 
                        if(free->sub_list->next == NULL){
                            free->flag = 0 ;
                        }
                        return ;
                    }
                sub = sub->next ;
            }
            
        }
        free = free->next ;
    }
    
}