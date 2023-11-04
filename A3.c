#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

#define PAGE_SIZE 4096

typedef struct SubChainNode {
    void* start_address;
    size_t size;
    struct SubChainNode* prev;
    struct SubChainNode* next;
    int segment_type; // Type of the segment: PROCESS or HOLE
} SubChainNode;

typedef struct FreeNode {
    struct SubChainNode* sub_chain_head;
    struct SubChainNode* sub_chain_tail;
    struct FreeNode* prev;
    struct FreeNode* next;
} FreeNode;

typedef struct FreeList {
    FreeNode* head;
    FreeNode* tail;
} FreeList;

FreeList free_list;
void* mems_start_address = NULL;
size_t total_mapped_pages = 0;
size_t total_unused_memory = 0;
void* base_register = NULL;

void mems_init() {
    free_list.head = NULL;
    free_list.tail = NULL;
    mems_start_address = (void*)1000;
    base_register = mems_start_address;
    total_mapped_pages = 0;
    total_unused_memory = 0;
}

void mems_finish() {
    FreeNode* main_current = free_list.head;

    while (main_current) {
        FreeNode* main_next = main_current->next;

        SubChainNode* sub_current = main_current->sub_chain_head;

        while (sub_current) {
            SubChainNode* sub_next = sub_current->next;
            munmap(sub_current->start_address, sub_current->size);
            sub_current = sub_next;
        }

        main_current = main_next;
    }

    free_list.head = NULL;
    free_list.tail = NULL;
    total_mapped_pages = 0;
    total_unused_memory = 0;
    base_register = NULL;
}

void* mems_malloc(size_t size) {
    if (base_register == NULL) {
        return NULL;  // MeMS not initialized
    }

    FreeNode* main_current = free_list.head;

    while (main_current) {
        SubChainNode* sub_current = main_current->sub_chain_head;

        while (sub_current) {
            if (total_unused_memory >= size && sub_current->segment_type == 0) {
                void* allocated_address = base_register;
                size_t remaining_size = sub_current->size - size;

                if (remaining_size >= PAGE_SIZE) {
                    // Update the current sub-chain node
                    sub_current->start_address = base_register;
                    sub_current->size = remaining_size - PAGE_SIZE;
                } else {
                    // Remove the current sub-chain node
                    if (sub_current == main_current->sub_chain_head) {
                        main_current->sub_chain_head = sub_current->next;
                        if (sub_current == main_current->sub_chain_tail) {
                            main_current->sub_chain_tail = NULL;
                        }
                    } else if (sub_current == main_current->sub_chain_tail) {
                        main_current->sub_chain_tail = sub_current->prev;
                        main_current->sub_chain_tail->next = NULL;
                    } else {
                        sub_current->prev->next = sub_current->next;
                        sub_current->next->prev = sub_current->prev;
                    }
                }

                total_unused_memory -= size;
                sub_current->segment_type = 1;  // Mark as PROCESS
                base_register+=size;
                return base_register;
            }

            sub_current = sub_current->next;
        }

        main_current = main_current->next;
    }

    size_t allocation_size = size + PAGE_SIZE - (size % PAGE_SIZE);

    void* allocated_address = mmap(NULL, allocation_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (allocated_address == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    FreeNode* new_main_node = (FreeNode*)allocated_address;
    SubChainNode* new_sub_node = (SubChainNode*)(allocated_address + sizeof(FreeNode));

    new_sub_node->start_address = allocated_address;
    new_sub_node->size = allocation_size;
    new_sub_node->segment_type = 0; // Initially a HOLE
    new_sub_node->prev = NULL;
    new_sub_node->next = NULL;

    new_main_node->sub_chain_head = new_sub_node;
    new_main_node->sub_chain_tail = new_sub_node;
    new_main_node->prev = free_list.tail;
    new_main_node->next = NULL;

    if (free_list.tail) {
        free_list.tail->next = new_main_node;
    } else {
        free_list.head = new_main_node;
    }

    free_list.tail = new_main_node;

    // Update total_mapped_pages
    total_mapped_pages += allocation_size / PAGE_SIZE;

    // Update the remaining free memory
    base_register = (total_mapped_pages-1)*PAGE_SIZE+size;
    total_unused_memory+=PAGE_SIZE;
    return base_register;
}


void mems_print_stats() {
    printf("Total Mapped Pages: %zu\n", total_mapped_pages);
    printf("Total Unused Memory: %zu bytes\n", total_unused_memory);

    FreeNode* main_current = free_list.head;
    int main_segmentNumber = 0;

    while (main_current != NULL) {
        printf("Main Chain Segment %d:\n", main_segmentNumber);

        SubChainNode* sub_current = main_current->sub_chain_head;
        int sub_segmentNumber = 0;

        while (sub_current != NULL) {
            printf("Sub-Chain Segment %d: Address: %p, Size: %zu bytes, Type: %s\n", sub_segmentNumber,
                   sub_current->start_address, sub_current->size, (sub_current->segment_type == 1) ? "PROCESS" : "HOLE");

            sub_current = sub_current->next;
            sub_segmentNumber++;
        }

        main_current = main_current->next;
        main_segmentNumber++;
    }
}

void* mems_get(void* v_ptr) {
    if (!v_ptr) {
        return NULL;
    }

    uintptr_t virtual_address = (uintptr_t)v_ptr;
    uintptr_t start_address = (uintptr_t)mems_start_address;

    if (virtual_address >= start_address && virtual_address < (start_address + total_mapped_pages * PAGE_SIZE)) {
        uintptr_t offset = virtual_address - start_address;
        uintptr_t physical_address = (uintptr_t)sbrk(0) + offset;
        return (void *)physical_address;
    }

    return NULL;
}

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

                // Merge consecutive HOLE segments
                SubChainNode* next_segment = sub_current->next;
                if (next_segment && next_segment->segment_type == 0) {
                    sub_current->size += next_segment->size;
                    sub_current->next = next_segment->next;
                }

                SubChainNode* prev_segment = sub_current->prev;
                if (prev_segment && prev_segment->segment_type == 0) {
                    prev_segment->size += sub_current->size;
                    prev_segment->next = sub_current->next;
                }
                return;
            }

            sub_current = sub_current->next;
        }

        main_current = main_current->next;
    }
}
