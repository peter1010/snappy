CFLAGS=-Wall -O3 -Wextra

CC=gcc -c
CCC=g++ -c

MAKEDEPEND=gcc -M $(CPPFLAGS)
LINK=gcc $(LDFLAGS)

OBJS= capture.o logging.o debug.o

.PHONY: all
all: capture

capture: $(OBJS)
	$(LINK) $(OBJS) -o $@ -lstdc++


%.o : %.c
	$(CC) $(CPPFLAGS) -MMD $(CFLAGS) -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	@mv $*.P $*.d

%.o : %.cpp
	$(CCC) $(CPPFLAGS) -MMD $(CFLAGS) -o $@ $<
	@cp $*.d $*.P
	@sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $*.P
	@rm -f $*.d
	@mv $*.P $*.d

-include $(OBJS:.o=.d)
