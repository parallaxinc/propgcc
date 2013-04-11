#ifndef _OMP_H
#define _OMP_H

#ifdef __cplusplus
extern "C" {
#endif

  int omp_get_num_threads(void);
  int omp_get_thread_num(void);
  void omp_set_num_threads(int max);
#ifdef __cplusplus
}
#endif

#endif
