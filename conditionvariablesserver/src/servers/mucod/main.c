#include "func.c"

int identifier = 0x1234;
endpoint_t who_e;
int call_nr;
endpoint_t SELF_E;
static int verbose = 0;

/* SEF functions and variables. */
static void sef_local_startup(void);
static int sef_cb_init_fresh(int type, sef_init_info_t *info);
static void sef_cb_signal_handler(int signo);


int main(int argc, char *argv[]) {
    init();
    message m;
    /* SEF local startup. */
    env_setargs(argc, argv);
    sef_local_startup();

    while (TRUE) {
        int r;
        if ((r = sef_receive(ANY, &m)) != OK)
            printf("ef_receive failed %d.\n", r);
        who_e = m.m_source;
        call_nr = m.m_type;
        if (verbose)
            printf("MUCO: get %d from %d\n", call_nr, who_e);
        int result;
        if (who_e == PM_PROC_NR) {
            int id = m.PM_PROC;
            if (m.m_type == PM_UNPAUSE) {
                int flag = 0;
                for (int i = 0; i < NR_PROCS; i++) {
                    if (queue[i].whoIam == id) {
                        flag = 1;
                        if (queue[i].prev == -1 && queue[i].next == -1) {
                            queue[i].whoIam = -1;
                            for (int j = 0; j < 1024; j++) {
                                if (arr[j].idx == i) {
                                    arr[j].idx = -1;
                                    break;
                                }
                            }
                        } else if (queue[i].prev == -1) {
                            queue[i].whoIam = -1;
                            int tmp = queue[i].next;
                            queue[tmp].prev = -1;
                            for (int j = 0; j < 1024; j++) {
                                if (arr[j].idx == i) {
                                    arr[j].idx = tmp;
                                    break;
                                }
                            }
                            queue[i].next = -1;
                        } else if (queue[i].next == -1) {
                            queue[i].whoIam = -1;
                            int tmp = queue[i].prev;
                            queue[tmp].next = -1;
                            queue[i].prev = -1;
                        } else {
                            queue[i].whoIam = -1;
                            int tmp1 = queue[i].next;
                            int tmp2 = queue[i].prev;
                            queue[tmp2].next = tmp1;
                            queue[tmp1].prev = tmp2;
                            queue[i].prev = -1;
                            queue[i].next = -1;
                        }
                    }
                }
                if (flag) {
                    message toSend;
                    toSend.m_type = EINTR;
                    sendnb(id, &toSend);
                }
                flag = 0;
                int found_mutex;
                int i;
                for (i = 0; i < NR_PROCS; i++) {
                    if (cod_arr[i].owner == id) {
                        flag = 1;
                        found_mutex = cod_arr[i].mutex_id;
                        break;
                    }
                }
                if (flag) {
                    cod_arr[i].cond_var_id = cod_arr[i].owner = cod_arr[i].mutex_id = -1;
                    int flag2 = 0;
                    int i;
                    for (i = 0; i < 1024; i++) {
                        if (arr[i].mutex_id == found_mutex) {
                            flag2 = 1;
                            break;
                        }
                    }
                    if (flag2) {
                        AddToQueue(id, found_mutex, 0);
                    } else if (flag2 == 0) {
                        int empty = -1;
                        for (int i = 0; i < 1024; i++) {
                            if (arr[i].owner == -1) {
                                empty = i;
                                break;
                            }
                        }
                        arr[empty].mutex_id = found_mutex;
                        arr[empty].owner = id;
                        arr[empty].idx = -1;
                        message temp;
                        temp.m_type = 0;
                        sendnb(id, &temp);
                    }
                }
            } else if (m.m_type == PM_EXIT || m.m_type == PM_DUMPCORE) {
                for (int i = 0; i < 1024; i++) {
                    if (arr[i].owner == id) {
                        if (arr[i].idx == -1) {
                            arr[i].mutex_id = -1;
                            arr[i].owner = -1;
                            arr[i].idx = -1;
                        } else if (arr[i].idx != -1) {
                            int toClear = arr[i].idx;
                            arr[i].owner = queue[toClear].whoIam;

                            int idx2 = queue[toClear].next;
                            if (idx2 == -1) {
                                arr[i].idx = -1;
                            }
                            if (idx2 != -1) {
                                queue[idx2].prev = -1;
                                arr[i].idx = idx2;
                            }
                            queue[toClear].prev = -1;
                            queue[toClear].next = -1;
                            queue[toClear].whoIam = -1;

                            message res;
                            res.m_type = 0;
                            sendnb(arr[i].owner, &res);
                        }
                    }
                }
                for (int i = 0; i < NR_PROCS; i++) {
                    if (queue[i].whoIam == id) {
                        if (queue[i].prev == -1 && queue[i].next == -1) {
                            queue[i].whoIam = -1;
                            for (int j = 0; j < 1024; j++) {
                                if (arr[j].idx == i) {
                                    arr[j].idx = -1;
                                }
                            }
                        } else if (queue[i].prev == -1) {
                            queue[i].whoIam = -1;
                            int tmp = queue[i].next;
                            queue[tmp].prev = -1;
                            for (int j = 0; j < 1024; j++) {
                                if (arr[j].idx == i) {
                                    arr[j].idx = tmp;
                                }
                            }
                            queue[i].next = -1;
                        } else if (queue[i].next == -1) {
                            queue[i].whoIam = -1;
                            int tmp = queue[i].prev;
                            queue[tmp].next = -1;
                            queue[i].prev = -1;
                        } else {
                            queue[i].whoIam = -1;
                            int tmp1 = queue[i].next;
                            int tmp2 = queue[i].prev;
                            queue[tmp2].next = tmp1;
                            queue[tmp1].prev = tmp2;
                            queue[i].prev = -1;
                            queue[i].next = -1;

                        }
                    }
                }
                for (int i = 0; i < NR_PROCS; i++) {
                    if (cod_arr[i].owner == id) {
                        cod_arr[i].cond_var_id = cod_arr[i].owner = cod_arr[i].mutex_id = -1;
                    }
                }
            }
            continue;
        }
        switch (call_nr) {
            case 0:
                result = do_lock(&m);
                if (result == 0) {
                    m.m_type = result;
                    sendnb(who_e, &m);
                }
                break;
            case 1:
                result = do_unlock(&m);
                if (result == -1) {
                    m.m_type = result;
                    sendnb(who_e, &m);
                }
                if (result == 0) {
                    m.m_type = result;
                    sendnb(who_e, &m);
                }
                break;
            case 2:
                result = do_wait(&m);
                if (result == -1) {
                    m.m_type = result;
                    sendnb(who_e, &m);
                }
                break;
            case 3:
                do_broadcast(&m);
                m.m_type = 0;
                sendnb(who_e, &m);
                break;
        }
    }
    /* no way to get here */
    return -1;
}
/*===========================================================================*
 *                             sef_local_startup                             *
 *===========================================================================*/
static void sef_local_startup() {
    /* Register init callbacks. */
    sef_setcb_init_fresh(sef_cb_init_fresh);
    sef_setcb_init_restart(sef_cb_init_fresh);

/* No live update support for now. */

/* Register signal callbacks. */
    sef_setcb_signal_handler(sef_cb_signal_handler);

/* Let SEF perform startup. */
    sef_startup();
}

/*===========================================================================*
 *                          sef_cb_init_fresh                                *
 *===========================================================================*/
static int sef_cb_init_fresh(int UNUSED(type), sef_init_info_t *UNUSED(info)){
/* Initialize the sws server. */
SELF_E = getprocnr();
if(verbose)
printf("MUCOD: self: %d\n", SELF_E);
return(OK);
}

/*===========================================================================*
 *                          sef_cb_signal_handler                            *
 *===========================================================================*/
static void sef_cb_signal_handler(int signo) {
    /* Only check for termination signal, ignore anything else. */
    if (signo != SIGTERM) return;
}


