OBJ=udp-send

include ../common.mk

.PHONY		: all clean


all		: $(OBJ)


udp-send	: udp-send.c
	$(CC) $(CPPOPT) $(CFLAGS) -o $@ $<


clean		:
	rm -f $(OBJ)
