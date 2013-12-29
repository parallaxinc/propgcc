TARGET=chimera.elf

BOARD?=c3
MODEL?=xmm-split

$(TARGET):	chimaera.c
	propeller-elf-gcc -Os -m$(MODEL) -o $@ $< -lm

run:	$(TARGET)
	propeller-load -b $(BOARD) $(TARGET) -r -t

clean:
	rm -rf $(TARGET)
