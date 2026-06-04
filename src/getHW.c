#include "getHW.h"

int getAudioHW(int *destCard, int *destDevice) {
   FILE *fp = popen("arecord -l", "r");
   if (!fp) {
      perror("popen");
      exit(1);
   }

   char line[256];
   *destCard = -1;
   *destDevice = -1;
   int ret = -1;
   while (fgets(line, sizeof(line), fp)) {
      if (sscanf(line, "card %d: Device %*[^,], device %d", destCard,
                 destDevice) == 2) {
         ret = 0;
         break;
      }
   }
   pclose(fp);
   return ret;
}

struct alsa_list getMidiDevices(void) {
   FILE *fp = popen("aconnect -l", "r");
   if (!fp) {
      perror("popen");
      exit(1);
   }
   char line[256];
   struct alsa_list list = {0};
   list.size = 0;
   // uint8_t port = 0;
   while (fgets(line, sizeof(line), fp)) {
      if (list.size < 32 && strncmp(line, "client", 6) == 0) {
         struct alsa_device *current = &list.devices[list.size];
         int client;
         char name[128];

         if (sscanf(line, "client %d: '%127[^']'", &client, name) == 2) {
            current->client = client;
            strcpy(current->name, name);
            current->is_kernel = strstr(line, "type=kernel") != NULL;
            current->ports_size = 0;
            list.size++;
         }
      }
      // Detect port line
      else if ((line[0] == ' ' || line[0] == '\t') && list.size > 0) {
         struct alsa_device *current = &list.devices[list.size - 1];
         int port;
         char portname[128];

         if (sscanf(line, " %d '%127[^']'", &port, portname) == 2) {
            if (current->ports_size < 32) {
               current->ports[current->ports_size] = port;
               strcpy(current->ports_names[current->ports_size], portname);
               current->ports_size++;
            }
         }
      }
   }
   pclose(fp);
   return list;
}

void printList(struct alsa_list *list) {
   int list_size = list->size;
   for (int i = 0; i < list_size; i++) {
      int ports_size = list->devices[i].ports_size;
      for (int j = 0; j < ports_size; j++) {
         printf("%s %d:%d\n", list->devices[i].name, list->devices[i].client,
                list->devices[i].ports[j]);
      }
   }
}