#ifndef _CV_H_
#define _CV_H_
__BEGIN_DECLS
int cs_lock(int mutex_id);
int cs_unlock(int mutex_id);
int cs_wait(int cond_var_id, int mutex_id);
int cs_broadcast(int cond_var_id);
__END_DECLS
#endif /* !_CV_H_ */


