#include <minix/endpoint.h>
#include <minix/config.h>
#include <stdio.h>

struct mutex{
    int mutex_id;
    endpoint_t owner;
    int idx;
};

struct queue_v{
    int prev;
    int next;
    endpoint_t whoIam;
};

struct cod_v{
    int cond_var_id;
    endpoint_t owner;
    int mutex_id;	
};

struct cod_v cod_arr[NR_PROCS];
struct mutex arr[1024];
struct queue_v queue[NR_PROCS];

void init(){
    for(int i=0;i<1024;i++){
        arr[i].mutex_id=-1;
        arr[i].owner=-1;
        arr[i].idx=-1;
    }
    for (int i = 0; i < NR_PROCS; ++i) {
        queue[i].prev=queue[i].next=queue[i].whoIam=-1;
	cod_arr[i].cond_var_id=cod_arr[i].owner=cod_arr[i].mutex_id=-1;
    }
}

int seek(int id){ // returns -1 if not found
    int to_return=-1;
    for(int i=0;i<1024;i++){
        if(arr[i].mutex_id==id && arr[i].owner!=-1)
            to_return=i;
    }
    return to_return;
}
