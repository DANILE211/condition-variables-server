#include "inc.h"
#include "que.c"
#include <minix/config.h>
#include <stdio.h>

int AddToQueue(endpoint_t who, int mutex_id, int f) {
    int where = seek(mutex_id);
    if (where == -1) { // mutex is free
        int empty = -1;
        for (int i = 0; i < 1024; i++) {
            if (arr[i].owner == -1) {
                empty = i;
                break;
            }
        }
        // what if empty still -1
        arr[empty].mutex_id = mutex_id;
        arr[empty].owner = who;
        arr[empty].idx = -1;
        if(f){
            message res;
            res.m_type = 0;
            sendnb(who, &res);
        }
        return 0;
    }
    if (where != -1) {
        if (arr[where].idx == -1) {
            int empty = -1;
            for (int i = 0; i < NR_PROCS; i++) {
                if (queue[i].whoIam == -1) {
                    empty = i;
                    break;
                }
            }
            arr[where].idx = empty;
            queue[empty].whoIam = who;
            queue[empty].next = -1;
            queue[empty].prev = -1; // he is the first;
        } else if (arr[where].idx != -1) {
            int empty = -1;
            for (int i = 0; i < NR_PROCS; i++) {
                if (queue[i].whoIam == -1) {
                    empty = i;
                    break;
                }
            }
            int last = arr[where].idx;
            while (queue[last].next != -1) {
                last = queue[last].next;
            }
            queue[last].next = empty;
            queue[empty].prev = last;
            queue[empty].next = -1;
            queue[empty].whoIam = who;

        }
        return 1;
    }
}

int DeleteFromQueue(endpoint_t who, int mutex_id) {
    int where = seek(mutex_id);
    if (where == -1) return -1;
    if (where != -1 && arr[where].owner != who) return -1;
    if (where != -1 && arr[where].owner == who) {
        if (arr[where].idx == -1) {
            arr[where].mutex_id = -1;
            arr[where].owner = -1;
            arr[where].idx = -1;
            return 0;
        } else if (arr[where].idx != -1) {
            int toClear = arr[where].idx;
            arr[where].owner = queue[arr[where].idx].whoIam;

            message res;
            res.m_type = 0;
            sendnb(arr[where].owner, &res);

            int idx2 = queue[arr[where].idx].next;
            if (idx2 == -1) {
                arr[where].idx = -1;
            }
            if (idx2 != -1) {
                queue[idx2].prev = -1;
                arr[where].idx = idx2;
            }
            queue[toClear].prev = -1;
            queue[toClear].next = -1;
            queue[toClear].whoIam = -1;
            return 0;
        }
    }
}
int do_lock(message *m) {
    int mutex_id = m->m1_i1;
    endpoint_t who = m->m_source;
    int res = AddToQueue(who, mutex_id,0);
    return res;
}

int do_unlock(message *m) {
    int mutex_id = m->m1_i1;
    endpoint_t who = m->m_source;
    int res = DeleteFromQueue(who, mutex_id);
    return res;
}

int do_wait(message *m) {
    int cond_var_id = m->m1_i1;
    int mutex_id = m->m1_i2;
    endpoint_t who = m->m_source;
    int whatToDo = DeleteFromQueue(who, mutex_id);
    if (whatToDo == -1) { // we don't have mutex
        return -1;
    }
    int empty = -1;
    for (int i = 0; i < NR_PROCS; i++) {
        if (cod_arr[i].owner == -1) {
            empty = i;
            break;
        }
    }
    cod_arr[empty].owner = who;
    cod_arr[empty].cond_var_id = cond_var_id;
    cod_arr[empty].mutex_id = mutex_id;
    return 0;
}

int do_broadcast(message *m) {
    int cond_var_id = m->m1_i1;
    for (int i = 0; i < NR_PROCS; i++) {
        if (cod_arr[i].cond_var_id == cond_var_id) {
            AddToQueue(cod_arr[i].owner, cod_arr[i].mutex_id,1);
            cod_arr[i].cond_var_id = cod_arr[i].owner = cod_arr[i].mutex_id = -1;
        }
    }
    return 0;
}

