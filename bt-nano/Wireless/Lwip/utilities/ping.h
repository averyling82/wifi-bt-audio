//----------------------------------------------------------------------------//
#ifndef __PING_H
#define __PING_H

#ifdef __cplusplus
  extern "C" {
#endif

/* Exported test functions ------------------------------------------------------- */
void do_ping_test(char *ip, int size, int count, int interval);
void do_ping_call(char *ip, int size, int count, int interval);
rk_err_t Ping_Shell(HDC dev, uint8 * pstr);

#ifdef __cplusplus
  }
#endif

#endif // __MAIN_TEST_H

//----------------------------------------------------------------------------//
