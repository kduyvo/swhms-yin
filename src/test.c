#include "getHW.h"

int main(void) {
   struct alsa_list list = getMidiDevices();
   printList(&list);
   return 0;
}