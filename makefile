#CXXFLAGS=
#LDFLAGS=
#�l�ؿ���Makefile����Ū����l�ؿ��N��
SUBDIRS=$(shell ls -l | grep ^d | awk '{print $$9}')
#�H�U�P�ڥؿ��U��makefile���ۦP�N�X������
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