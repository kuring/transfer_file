INCLUDE = -I. 

FLAGS = -g -Wall $(INCLUDE)
FLAGS += -fPIC

LIBDIR = -L -lltime -L./lib -L/usr/lib64 -L/usr/local/lib -lz -lm -lcrypto -lpcap

LINK = $(LIBDIR) -lpthread

GCC = g++

# for C++ language
CODE.cpp = main.cpp \
			file_saver_unit.cpp \
			file_saver.cpp \
			file_sender_map.cpp \
			file_sender.cpp \
			inifile.cpp \
			netcard_reader.cpp \
			netcard_reader_raw.cpp \
			netcard_writer.cpp \
			public_func.cpp \
			public_var.cpp \
			session.cpp \

CPP.o = $(CODE.cpp:.cpp=.o)

OBJS.o = $(CPP.o)

# rule for C++ language
%.o : %.cpp	
	$(GCC) $(FLAGS) -o $@ -c $<	
	@echo $*.o build successfully!......

TARGET = transfer_file
	
$(TARGET) : $(OBJS.o) 
	$(GCC) $(OBJS.o) -o $(TARGET) $(LINK)
	@echo $(TARGET) BUILD OK!.........
	
all:	$(TARGET)

.PHONY:
clean:
	rm -rf $(OBJS.o) $(TARGET)
