#include <FreeRTOS.h>
#include <task.h>

int * __errno(void) {
  int *errnoptr = pvTaskGetThreadLocalStoragePointer( NULL, 0 );
  if(errnoptr == NULL) {
      errnoptr = pvPortMalloc( sizeof(int) );
      vTaskSetThreadLocalStoragePointer( NULL, 0, errnoptr );
  }
  return errnoptr;
}

int get_errno(void) {
  return *(int *)pvTaskGetThreadLocalStoragePointer( NULL, 0 );
}

void set_errno(int errno) {
  *(int *)pvTaskGetThreadLocalStoragePointer( NULL, 0 ) = errno;
}
