CC      := gcc
CPPFLAGS:= -Iinclude
CFLAGS  := -O3 -march=native -fno-math-errno -fno-trapping-math -Wall -Wextra -MMD -MP

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

BINS := wrapper swhms frameRingBuffer frameProcessor yin envelope toMidi autotune live volumeModulation presetModulation debounce encoder uart debugScreen battery keypad lpf hpf alsa test

VPATH := $(SRC_DIR)

.PHONY: all clean convert
all: $(BIN_DIR) $(OBJ_DIR) $(addprefix $(BIN_DIR)/,$(BINS))

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# ----- object build -----
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

-include $(OBJ_DIR)/*.d

# ----- common objects -----
IO_OBJ   := $(OBJ_DIR)/helperIO.o
MIDI_FUN_OBJ := $(OBJ_DIR)/midiFun.o
INST_OBJ := $(OBJ_DIR)/instruments.o
COMMAND_HIST_OBJ := $(OBJ_DIR)/commandHistory.o
I2C_OBJ := $(OBJ_DIR)/i2c.o
GET_HW_OBJ := $(OBJ_DIR)/getHW.o
# ----- link each binary explicitly (simple + correct) -----
$(BIN_DIR)/swhms: $(OBJ_DIR)/swhms.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/uart: $(OBJ_DIR)/uart.o $(IO_OBJ) $(MIDI_FUN_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lgpiod

$(BIN_DIR)/frameRingBuffer: $(OBJ_DIR)/frameRingBuffer.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/frameProcessor: $(OBJ_DIR)/frameProcessor.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/yin: $(OBJ_DIR)/yin.o | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/envelope: $(OBJ_DIR)/envelope.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/toMidi: $(OBJ_DIR)/toMidi.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

# needs libm
$(BIN_DIR)/autotune: $(OBJ_DIR)/autotune.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lm

# needs libm
$(BIN_DIR)/live: $(OBJ_DIR)/live.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lm

$(BIN_DIR)/volumeModulation: $(OBJ_DIR)/volumeModulation.o $(IO_OBJ) $(INST_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lm

$(BIN_DIR)/presetModulation: $(OBJ_DIR)/presetModulation.o $(IO_OBJ) $(INST_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/debounce: $(OBJ_DIR)/debounce.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/encoder: $(OBJ_DIR)/encoder.o $(IO_OBJ) $(MIDI_FUN_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/debugScreen: $(OBJ_DIR)/debugScreen.o $(IO_OBJ) $(GET_HW_OBJ) $(COMMAND_HIST_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lncurses

$(BIN_DIR)/battery: $(OBJ_DIR)/battery.o $(IO_OBJ) $(I2C_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

# $(BIN_DIR)/midiFun: $(OBJ_DIR)/midiFun.o $(IO_OBJ) | $(BIN_DIR)
# 	$(CC) -o $@ $^

$(BIN_DIR)/wrapper: $(OBJ_DIR)/wrapper.o $(IO_OBJ) $(GET_HW_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lncurses

$(BIN_DIR)/keypad: $(OBJ_DIR)/keypad.o $(IO_OBJ) $(I2C_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/lpf: $(OBJ_DIR)/lpf.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^

$(BIN_DIR)/hpf: $(OBJ_DIR)/hpf.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lgpiod

$(BIN_DIR)/alsa: $(OBJ_DIR)/alsa.o $(IO_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lasound

$(BIN_DIR)/test: $(OBJ_DIR)/test.o $(GET_HW_OBJ) | $(BIN_DIR)
	$(CC) -o $@ $^ -lasound


clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.pcm

# Optional: convert WAV to raw PCM
ARG1 ?= example.wav
ARG2 ?= 48000
ARG3 ?= output.pcm
convert:
	sox $(ARG1) -r $(ARG2) -c 1 -b 16 -e signed-integer -L -t raw $(ARG3)

run:
	cd bin && ./wrapper
