CFLAGS=-Wall -O3

MAKEDEPEND=gcc -M $(CPPFLAGS) -o $*.d $<
COMPILE=gcc -c $(CPPFLAGS) $(CFLAGS) -o $@ $<
LINK=gcc $(LDFLAGS) -o $@

OBJS= capture.o logging.o debug.o

.PHONY: all
all: capture

capture: $(OBJS)
	$(LINK) $(OBJS)


%.o : %.c
	$(COMPILE) -MMD
	@cp $*.d $*.P;
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	@mv $*.P $*.d


-include $(OBJS:.o=.d) 
