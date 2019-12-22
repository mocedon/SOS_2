#
# makefile for HW2 in OS
#
CPPFLAGS = -g -pthread
LDFLAGS = -lpthread
OBJS = Bank.o Account.o
RM = rm -f
Bank: $(OBJS)
	  $(CXX) -o Bank $(OBJS) $(LDFLAGS)
Account.o: Account.cpp Account.h
Bank.o: Bank.cpp Bank.h Account.h
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*


