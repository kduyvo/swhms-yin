#include "instruments.h"

struct instrument preset[128];

void presetInit(void){
    FILE *fp = fopen("../presets", "r");
    char buffer[256];
    if (fp == NULL) {
        perror("Error opening file"); // Prints the system error message
        exit(1);
    }
    while(fgets(buffer, sizeof buffer, fp) != NULL){
        char* name = strtok(buffer, " \t\n");
        uint8_t midiNum, velocity, cc11;
        if (name == NULL || name[0] == '#')
            continue;
        char* midiNumStr = strtok(NULL, " \t\r\n");
        char* velocityStr = strtok(NULL, " \t\r\n");
        char* cc11Str = strtok(NULL, " \t\r\n");
        if (!midiNumStr || !velocityStr || !cc11Str)
            continue;
        midiNum = parseStrToU8(midiNumStr);
        velocity = parseStrToU8(velocityStr);
        cc11 = parseStrToU8(cc11Str);
        addPreset(name, midiNum, velocity, cc11);
    }
    
    fclose(fp);
}

uint8_t parseStrToU8(char* s) {
    char* endptr;
    uint8_t value = (uint8_t) strtol(s, &endptr, 10);
    if (*endptr != '\0'){
        printf("Invalid parse\n");
        exit(1);
    }
    return value;
}

void addPreset(char name[32], uint8_t midiNum, uint8_t velocity, uint8_t cc11) {
    static int currentIndex = 0;
    strncpy(preset[currentIndex].name, name, 32);
    preset[currentIndex].midiNum = midiNum;
    preset[currentIndex].velocity = velocity;
    preset[currentIndex].cc11 = cc11;
    currentIndex++;
}

char* getName(uint8_t index){
    return preset[index].name;
}

uint8_t getMidiNum(uint8_t index){
    return preset[index].midiNum;
}

uint8_t getVelocity(uint8_t index){
    return preset[index].velocity;
}

uint8_t getCC11(uint8_t index){
    return preset[index].cc11;
}

