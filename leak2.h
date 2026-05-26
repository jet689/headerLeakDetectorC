#ifndef LEAK2H
#define LEAK2H
#define LEAK2H_hash_table_start_size 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// #define LEAK2H_unknown_free_error
// #define LEAK2H_show_warnings
#define LEAK2H_unknown_realloc_error
FILE* LEAK2_file;
FILE* LEAK2_leak_file;
FILE* LEAK2_warning_file;
struct{
    char *s;
    int count;
} LEAK2_last_warning;

char LEAK2_had_warning=0;



void LEAK2_show_last_warning(){
    LEAK2_had_warning=1;
    fprintf(LEAK2_warning_file,"(%dx) %s",LEAK2_last_warning.count,LEAK2_last_warning.s);
    fprintf(LEAK2_warning_file,"\n");
}

void LEAK2_warning(char *s,char* file, int line){
#ifdef LEAK2H_show_warnings
    printf(s);
    printf(" %s:%d\n",file,line);
#endif
    char warn[80];
    snprintf(warn,80,"%s %s:%d",s,file,line);
    if(strcmp(warn,LEAK2_last_warning.s)==0){
        LEAK2_last_warning.count+=1;
    }else if(LEAK2_last_warning.count==0){
        snprintf(LEAK2_last_warning.s,80,"%s",warn);
        LEAK2_last_warning.count=1;
    }
    else{
        LEAK2_show_last_warning();
        snprintf(LEAK2_last_warning.s,80,"%s",warn);
        LEAK2_last_warning.count=1;
    }
}

void *LEAK2_catchAlloc(void *ptr){
    if(ptr==NULL){
        printf("LEAK2-INTERNAL-ERROR::ALLOCATION FAILURE");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

//one element of the seperate chaining hash tables linked list
typedef struct{
    void* next;
    char* file;
    int line;
    void *ptr;
    size_t size;
    size_t reallocSize;
    void *chronological_next;
    void *chronological_previous;
} LEAK2_one_element;

LEAK2_one_element *LEAK2_last_element=NULL;
LEAK2_one_element *LEAK2_first_element=NULL;

//the hash table
struct LEAK2_table_template {
    LEAK2_one_element **vals;
    int size_of_vals;
    int frees; //total frees
    int allocs; //total allocations
    int reallocs; //total reallocations
    int freeSize; //total freed
    int allocSize; //total allocated
} LEAK2_table;

int LEAK2_initialized=0;

int LEAK2_exited=0;

void LEAK2_show_leak(LEAK2_one_element* l,int count, long long total_size, long long total_reallocated_size){
    fprintf(LEAK2_leak_file,"%d leak(s) at %s:%d totalling %lld bytes with %lld realloc bytes\n",count,l->file,l->line,total_size+total_reallocated_size,total_reallocated_size);
}
void LEAK2_show(){
    printf("========================\n");
    printf("number of allocations: %d\n",LEAK2_table.allocs);
    printf("number of frees: %d\n",LEAK2_table.frees);
    printf("number of reallocs: %d\n",LEAK2_table.reallocs);
    printf("number of leaks: %d\n",LEAK2_table.allocs-LEAK2_table.frees);
    printf("allocated amount: %d bytes\n",LEAK2_table.allocSize);
    printf("freed amount: %d bytes\n",LEAK2_table.freeSize);
    printf("leaked amount: %d bytes\n",LEAK2_table.allocSize-LEAK2_table.freeSize);
    printf("memory allocation table scaled to %d buckets\n", LEAK2_table.size_of_vals);
    if(LEAK2_first_element!=NULL){
        printf("========================\n");
        printf("leak(s) detected check leak2leaks.txt\n");
        LEAK2_one_element *cur=LEAK2_first_element;
        LEAK2_one_element *pre=NULL;
        long count;
        long long total_size;
        long long total_reallocated_size;
        while(cur!=NULL){
            if(pre==NULL){
                total_size=0;
                total_reallocated_size=0;
                count=0;
            }
            pre=cur;
            cur=cur->chronological_next;
            total_size+=pre->size;
            total_reallocated_size+=pre->reallocSize;
            count+=1;
            if(cur!=NULL){
                // LEAK2_show_leak(pre,count,total_size);
                if(strcmp(pre->file,cur->file)!=0 || pre->line!=cur->line){
                    LEAK2_show_leak(pre,count,total_size,total_reallocated_size);
                    pre=NULL;
                }
            }
        }
        if(pre!=NULL){
            LEAK2_show_leak(pre,count,total_size,total_reallocated_size);
        }

    }
    if(LEAK2_had_warning){
        printf("========================\n");
        printf("warning(s) detected check leak2warnings.txt\n");
    }
}

void LEAK2_exit(){
    if(!LEAK2_exited){
        LEAK2_exited=1;
        if(LEAK2_last_warning.count>0){
            LEAK2_show_last_warning();
        }
        LEAK2_show();
    }
}
//potentially init the function
void LEAK2_pot_init(){
    if(!LEAK2_initialized){
        LEAK2_initialized=1;
        LEAK2_table.vals=LEAK2_catchAlloc(calloc(LEAK2H_hash_table_start_size, sizeof(void*)));
        LEAK2_table.size_of_vals=LEAK2H_hash_table_start_size;
        LEAK2_table.allocs=0;
        LEAK2_table.allocSize=0;
        LEAK2_table.frees=0;
        LEAK2_table.reallocs=0;
        LEAK2_last_warning.count=0;
        LEAK2_last_warning.s=LEAK2_catchAlloc(malloc(sizeof(char)*80));
        LEAK2_last_warning.s[0]='\0';
        LEAK2_warning_file=fopen("leak2warnings.txt","w");
        LEAK2_leak_file=fopen("leak2leaks.txt","w");
        atexit(LEAK2_exit);
    }
}

LEAK2_one_element *LEAK2_get_hash_bucket(struct LEAK2_table_template table, void *ptr){
    return LEAK2_table.vals[(uintptr_t)ptr%table.size_of_vals];
}

LEAK2_one_element **LEAK2_get_pointer_to_hash_bucket(struct LEAK2_table_template table, void *ptr){
    return &table.vals[(uintptr_t)ptr%table.size_of_vals];
}

void LEAK2_pot_rescale(){
    if((LEAK2_table.allocs-LEAK2_table.frees+1)/LEAK2_table.size_of_vals>1.5){
        struct LEAK2_table_template temp_table;
        int nonPrimeRescale=LEAK2_table.size_of_vals*3;
        temp_table.size_of_vals=nonPrimeRescale+(int)log(nonPrimeRescale);
        temp_table.vals=LEAK2_catchAlloc(calloc(temp_table.size_of_vals, sizeof(void*)));
        // printf("rescaled from %d to %d\n",LEAK2_table.size_of_vals,temp_table.size_of_vals);
        LEAK2_one_element *pre=LEAK2_first_element;
        int i=0;
        while(pre!=NULL){
            
            int j=0;
            LEAK2_one_element **cur=LEAK2_get_pointer_to_hash_bucket(temp_table,pre->ptr);
            while(*cur!=NULL){
                j++;
                cur=(LEAK2_one_element **)&(*cur)->next;
            }
            *cur=pre;
            pre->next=NULL;
            pre=pre->chronological_next;
            i++;
        }
        free(LEAK2_table.vals);
        LEAK2_table.vals=temp_table.vals;
        LEAK2_table.size_of_vals=temp_table.size_of_vals;
    }
}

void LEAK2_append(void *ptr, size_t size, char* file, int line){
    LEAK2_pot_rescale();
    // printf("pointer %p \n",ptr);
    LEAK2_one_element **cur=LEAK2_get_pointer_to_hash_bucket(LEAK2_table,ptr);
    while(*cur!=NULL){
        cur=(LEAK2_one_element**)&((LEAK2_one_element*)*cur)->next;
    }
    *cur=(LEAK2_one_element*)LEAK2_catchAlloc(calloc(1,sizeof(LEAK2_one_element)));
    LEAK2_one_element *val=*cur;
    val->chronological_previous=LEAK2_last_element;
    
    
    if(LEAK2_last_element==NULL){
        LEAK2_first_element=val;
        
    }else{
        ((LEAK2_one_element*)LEAK2_last_element)->chronological_next=val;
    }
    LEAK2_last_element=val;
    val->ptr=ptr;
    val->line=line;
    val->size=size;
    val->file=LEAK2_catchAlloc(malloc(sizeof(char)*(strlen(file)+1)));
    strcpy(val->file,file);
    LEAK2_table.allocs+=1;
    LEAK2_table.allocSize+=size;
}


void *LEAK2_malloc(size_t size, char* file,int line){
    LEAK2_pot_init();
    void *out=malloc(size);
    if(out!=NULL){
    LEAK2_append(out,size,file,line);
    }
    return out;

}

void *LEAK2_calloc(int num_of_elements,size_t size, char* file,int line){
    LEAK2_pot_init();
    void *out=calloc(num_of_elements,size);
    if(out!=NULL){
    LEAK2_append(out,size*num_of_elements,file,line);
    }
    return out;

}

void LEAK2_free(void *ptr, char* file,int line){
    // printf("pointer %p \n",ptr);
    LEAK2_one_element **pre=LEAK2_get_pointer_to_hash_bucket(LEAK2_table,ptr);
    LEAK2_one_element *cur=LEAK2_get_hash_bucket(LEAK2_table,ptr);
    while(cur!=NULL){
        if(cur->ptr!=ptr){
            pre=(LEAK2_one_element **)&cur->next;
            cur=cur->next;
            // printf("shifted");
        }else{
            if(cur->chronological_previous!=NULL){
                ((LEAK2_one_element *)cur->chronological_previous)->chronological_next
                =cur->chronological_next;
            }
            else{
                LEAK2_first_element=NULL;
                
            }
            if(cur->chronological_next!=NULL) {
                ((LEAK2_one_element *)cur->chronological_next)->chronological_previous
                =cur->chronological_previous;
            }
            else{
                LEAK2_last_element=cur->chronological_previous;
            }
            LEAK2_table.freeSize+=cur->size;
            LEAK2_table.frees+=1;
            free(cur->file);
            *pre=cur->next;
            free(cur);
            return;
        }
    }

    LEAK2_warning("LEAK-DETECTOR-WARNING::UNKNOWN FREE",file,line);
#ifdef LEAK2H_unknown_free_error
    exit(EXIT_FAILURE);
#endif
}

void *LEAK2_realloc(void *ptr, size_t new_size,char* file, int line){
    LEAK2_one_element **pre=LEAK2_get_pointer_to_hash_bucket(LEAK2_table,ptr);
    LEAK2_one_element *cur=LEAK2_get_hash_bucket(LEAK2_table,ptr);
    while(cur!=NULL){
        if(cur->ptr!=ptr){
            pre=(LEAK2_one_element **)&cur->next;
            cur=cur->next;
        }else{
            break;
        }
    }
    if(cur==NULL){
        LEAK2_warning("LEAK-DETECTOR-WARNING::UNKNOWN REALLOC",file,line);
#ifdef LEAK2H_unknown_realloc_error
        exit(EXIT_FAILURE);
#endif
    }
    void *out=realloc(ptr,new_size);
    if(out==NULL){
        return out;
    }
    LEAK2_table.allocSize+=(new_size-cur->size)-cur->reallocSize;
    LEAK2_table.reallocs+=1;
    cur->reallocSize=new_size-cur->size;

    *pre=cur->next;

    cur->ptr=out;
    cur->next=NULL;
    LEAK2_one_element **cur2=LEAK2_get_pointer_to_hash_bucket(LEAK2_table,out);

    while((*cur2)!=NULL){
        cur2=(LEAK2_one_element **)&(*cur2)->next;
    }
    *cur2=cur;
    
    return out;
    // return
}
#define realloc(ptr,newsize) LEAK2_realloc(ptr,newsize,__FILE__,__LINE__)
#define malloc(size) LEAK2_malloc(size,__FILE__,__LINE__)
#define calloc(num_of_elements,size) LEAK2_calloc(num_of_elements,size,__FILE__,__LINE__)
#define free(ptr) LEAK2_free(ptr,__FILE__,__LINE__)

#endif