#CXXFLAGS=
#LDFLAGS=
#子目錄的Makefile直接讀取其子目錄就行
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
#以下同根目錄下的makefile的相同代碼的解釋
CUR_SOURCE=${wildcard *.cpp}
CUR_OBJS=${patsubst %.cpp, %.o, $(CUR_SOURCE)}
all:$(SUBDIRS) $(CUR_OBJS)
$(SUBDIRS):ECHO
	$(MAKE) -C $@
$(CUR_OBJS):%.o:%.cpp
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ $(LIBS) $(LTHREAD)
ECHO:
	@echo $(SUBDIRS)

clean:
	rm -f  ./*.o *~