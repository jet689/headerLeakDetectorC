#include "leak2.h"
//for testing purposes
int main(){
    // test one value being created and freed (once through an error)
    void *z=malloc(sizeof(int));
    free(z);
    
    //try mallocing a few ints and losing their pointers
    void *a;
    for(int i=0;i<10;i++){
        a=malloc(sizeof(int));
    }
    void *b;
    //try mallocing a few longs and losing their pointers
    for(int i=0;i<10;i++){
        b=malloc(sizeof(long));
    }
    //test free
    free(a);
    //throw warnings
    for(int i=0;i<10;i++){
    free(a);}
    //more free
    free(b);

    //test realloc
    int *c=malloc(sizeof(int));
    *c=10;
    c=realloc(c,sizeof(int)*2);
    printf("%d\n",*c);
    c=malloc(sizeof(int));
    c=realloc(c,sizeof(int)*4);
    // printf("%p",c);
    free(c);
    c=malloc(sizeof(int));
    
    //test calloc
    int *d=calloc(2,sizeof(int));
    free(d);
    d=calloc(2,sizeof(int));
    printf("%d",d[0]);
    // c=realloc(c,sizeof(int)*4);
    // LEAK2_append(a,sizeof(void*),__LINE__);
    // LEAK2_append(a,sizeof(void*),__LINE__);
    // LEAK2_show();

    return 0;
}
