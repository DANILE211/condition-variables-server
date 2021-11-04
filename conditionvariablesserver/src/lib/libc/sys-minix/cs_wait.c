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

int cs_wait(int cond_var_id,int mutex_id){
    endpoint_t mucod_pt;
    if(minix_rs_lookup("mucod", &mucod_pt)!=OK){
        errno = ENOSYS;
        return -1;
    }
    message m;
    m.m1_i1 = cond_var_id;
    m.m1_i2 = mutex_id;
    int res = _syscall(mucod_pt, 2, &m);
    if (res == -1){
	errno = EINVAL * (-1);
	return res;
    }
    return res;
}

