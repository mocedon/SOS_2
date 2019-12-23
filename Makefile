#
# Bank implementation
# using C++11
#
CPPFLAGS = -g -pthread -std=c++11
LDFLAGS = -lpthread
OBJS = Bank.o Account.o
RM = rm -f
Bank: $(OBJS)
	  $(CXX) -o Bank $(OBJS) $(LDFLAGS)
Account.o: Account.cpp Account.h
Bank.o: Bank.cpp Bank.h Account.h
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*