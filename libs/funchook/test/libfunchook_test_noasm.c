/*
/^-----^\   data: 2026-04-30
V  o o  V  plik: libs/funchook/test/libfunchook_test_noasm.c
 |  Y  |   autor: pupnoodle
  \ Q /
  / - \
  |    \
  |     \     )
  || (___\====
*/

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern int get_val_in_dll(void);

DLLEXPORT
int call_get_val_in_dll(void)
{
  return get_val_in_dll();
}

DLLEXPORT
int jump_get_val_in_dll(void)
{
  return get_val_in_dll();
}
