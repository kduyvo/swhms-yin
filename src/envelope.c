#include "swhms.h"
#include "envelope.h"

int main(void){
    int16_t buffer[BUFFER_SIZE];
    uint32_t e = 0;
    while(read_full(STDIN_FILENO, buffer, sizeof(buffer)) > 0){
        uint32_t alpha;
        uint16_t amplitude;
        for (int i = 0; i < BUFFER_SIZE; i++){
            buffer[i] = (buffer[i] < 0) ? -buffer[i] : buffer[i];
            alpha = ALPHA_Z;

            e = (INT16_MAX+1-alpha)*buffer[i] + alpha*e;
            e >>= 15;
            amplitude = e;
        }
        if (write_full(STDOUT_FILENO, &amplitude, sizeof(uint16_t)) < 0){
            perror("write");
            exit(1);
        }
    }
}