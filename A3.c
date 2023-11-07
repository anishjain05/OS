/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#define MAX_MAPPINGS 10000

struct AddressMapping {
    void* virtual_address;
    void* physical_address;
};

typedef struct SubChainNode {
    size_t size;
    struct SubChainNode* next;
    int segment_type; // Type of the segment: PROCESS or HOLE
} SubChainNode;

typedef struct FreeNode {
    struct SubChainNode* sub_chain_head;
    struct FreeNode* next;
} FreeNode;
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
#define PAGE_SIZE 4096
size_t total_mapped_pages = 0;
struct FreeNode* new_main_node;
void* base_register = NULL;
size_t c=0;
struct AddressMapping address_map[MAX_MAPPINGS];

/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_init(){
        new_main_node = (FreeNode*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        total_mapped_pages = 0;
        base_register=(void *)0;
        c++;
        struct SubChainNode* new_sc_node=(struct SubChainNode *)new_main_node;
        new_sc_node->size=PAGE_SIZE;
        new_sc_node->segment_type=1;
        new_sc_node->next=NULL;
        new_main_node->sub_chain_head=new_sc_node;
        new_main_node->next=NULL;
         for (int i = 0; i < MAX_MAPPINGS; i++) {
        address_map[i].virtual_address = NULL;
        address_map[i].physical_address = NULL;
    }
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish() {
    struct FreeNode* current_main = new_main_node;
    struct SubChainNode* sub_chain_node = NULL;

    while (current_main != NULL) {
        sub_chain_node = current_main->sub_chain_head;
        while (sub_chain_node != NULL) {
            struct SubChainNode* temp = sub_chain_node;
            sub_chain_node = sub_chain_node->next;
            if (temp->segment_type == 0) {
                // This is a segment we allocated with mmap, so unmap it.
                munmap(temp, temp->size);
            }
        }
        struct FreeNode* temp_main = current_main;
        current_main = current_main->next;
        // Unmap the main chain node.
        munmap(temp_main, PAGE_SIZE);
    }
    new_main_node = NULL;
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
void* mems_malloc(size_t size) {
    struct SubChainNode* ptr;
    while (new_main_node != NULL) {
        ptr = new_main_node->sub_chain_head;
        while (ptr != NULL) {
            if (ptr && ptr->size >= size && ptr->segment_type == 1) {
                ptr->segment_type = 0;
                if (ptr->size > size) {
                    struct SubChainNode* new = (struct SubChainNode*)((uintptr_t)ptr + size + sizeof(struct SubChainNode));
                    new->size = ptr->size - size - sizeof(struct SubChainNode);
                    new->segment_type = 1;
                    new->next = ptr->next;
                    ptr->size = size;
                    ptr->segment_type = 0;
                    ptr->next = new;
                }
                base_register = (void*)((uintptr_t)base_register + size);
                for (int i = 0; i < MAX_MAPPINGS; i++) {
                    if (address_map[i].virtual_address == NULL) {
                        address_map[i].virtual_address = base_register;
                        address_map[i].physical_address = (uintptr_t)ptr + size + sizeof(struct SubChainNode); // You need to modify this to get the actual physical address
                        break;
        }
    }
                total_mapped_pages++;
                return base_register;
            } else if (ptr->size > size + sizeof(struct SubChainNode)) {
                struct SubChainNode* new = (struct SubChainNode*)((uintptr_t)ptr + size + sizeof(struct SubChainNode));
                if (new == MAP_FAILED) {
                    perror("mmap failed");
                    return NULL;
                }
                new->size = ptr->size - size - sizeof(struct SubChainNode);
                new->segment_type = 1;
                new->next = ptr->next;
                ptr->size = size;
                ptr->segment_type = 0;
                ptr->next = new;
                base_register = (void*)((uintptr_t)base_register + size + sizeof(struct SubChainNode));
                for (int i = 0; i < MAX_MAPPINGS; i++) {
                    if (address_map[i].virtual_address == NULL) {
                        address_map[i].virtual_address = base_register;
                        address_map[i].physical_address = (uintptr_t)ptr + size + sizeof(struct SubChainNode); // You need to modify this to get the actual physical address
                        break;
        }
    }
                total_mapped_pages++;
                return base_register;
            } else {
                ptr = ptr->next;
            }
        }
        new_main_node = new_main_node->next;
    }

    struct FreeNode* new_main = (FreeNode*)mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (new_main == MAP_FAILED) {
        perror("mmap failed");
        return NULL;
    }
    struct SubChainNode* new_sc_node1 = (SubChainNode*)new_main;
    new_sc_node1->size = PAGE_SIZE;
    new_sc_node1->segment_type = 1;
    new_sc_node1->next = NULL;
    new_main->sub_chain_head = new_sc_node1;
    new_main->next = NULL;
    new_main_node->next = new_main;
    base_register = (void*)((uintptr_t)(c * PAGE_SIZE));
    for (int i = 0; i < MAX_MAPPINGS; i++) {
                    if (address_map[i].virtual_address == NULL) {
                        address_map[i].virtual_address = base_register;
                        address_map[i].physical_address = new_main; // You need to modify this to get the actual physical address
                        break;
        }
    }
    c++;
    return mems_malloc(size);
}

/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/
void mems_print_stats(){
    struct FreeNode* current_main = new_main_node;
    int main_chain_length = 0;
    int* sub_chain_lengths = NULL;
    int pages_used = 0;
    int space_unused = 0;

    while (current_main != NULL) {
        main_chain_length++;
        struct SubChainNode* sub_chain_node = current_main->sub_chain_head;
        while (sub_chain_node != NULL) {
            if (sub_chain_node->segment_type == 0) {
                pages_used += sub_chain_node->size / PAGE_SIZE;
            } else {
                space_unused += sub_chain_node->size;
            }
            sub_chain_node = sub_chain_node->next;
        }
        current_main = current_main->next;
    }

    printf("MeMS SYSTEM STATS\n");
    printf("MAIN Chain Length: %d\n", main_chain_length);
    printf("Pages used: %d\n", pages_used);
    printf("Space unused: %d\n", space_unused);
}


/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/
void *mems_get(void*v_ptr){
        for (int i = 0; i < MAX_MAPPINGS; i++) {
        if (address_map[i].virtual_address == v_ptr) {
            return address_map[i].physical_address;
        }
    }
    
    return NULL;
}


/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/
void mems_free(void* v_ptr) {
    uintptr_t virtual_address = (uintptr_t)v_ptr;

    FreeNode* main_current = free_list.head;

    while (main_current) {
        SubChainNode* sub_current = main_current->sub_chain_head;

        while (sub_current) {
            uintptr_t start_address = (uintptr_t)sub_current->start_address;
            uintptr_t end_address = start_address + sub_current->size;

            if (virtual_address >= start_address && virtual_address < end_address) {
                // Mark the segment as a HOLE
                sub_current->segment_type = 0;

                // Add this segment to the unused memory
                total_unused_memory += sub_current->size;

                // Merge consecutive HOLE segments (Next segment)
                SubChainNode* next_segment = sub_current->next;
                if (next_segment && next_segment->segment_type == 0) {
                    sub_current->size += next_segment->size;
                    sub_current->next = next_segment->next;
                }

                // Merge consecutive HOLE segments (Previous segment)
                SubChainNode* prev_segment = sub_current->prev;
                if (prev_segment && prev_segment->segment_type == 0) {
                    prev_segment->size += sub_current->size;
                    prev_segment->next = sub_current->next;
                }

                // You may want to implement additional logic to manage the free list here.

                return;
            }

            sub_current = sub_current->next;
        }

        main_current = main_current->next;
    }
}
