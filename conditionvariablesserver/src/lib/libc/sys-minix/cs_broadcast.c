#define _SYSTEM 1
#define _MINIX 1
#include <sys/cdefs.h>
#include <lib.h>
#include "namespace.h"

#include <minix/rs.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <errno.h>

int cs_broadcast(int cond_var_id){
    endpoint_t mucod_pt;
    if(minix_rs_lookup("mucod", &mucod_pt)!=OK){
        errno = ENOSYS;
        return -1;
    }
    message m;
    m.m1_i1 = cond_var_id;
    int res = _syscall(mucod_pt, 3, &m);
    return res;
}

