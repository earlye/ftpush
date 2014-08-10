export
UNAME?=$(shell uname)
include config.$(UNAME).mk
WORKSPACE?=$(shell pwd)
CPP_FILES=$(shell find $(WORKSPACE) -name "*.cpp" | grep -v ecpp/test)
CXXFLAGS+=-I$(WORKSPACE)/lib/ecpp -pthread
ifneq ($(DEBUG),)
CXXFLAGS+=-g
endif
LINKFLAGS+=-lboost_program_options-mt -lboost_regex-mt -lboost_filesystem-mt -lboost_system-mt -lcurl
O_FILES=$(patsubst %.cpp,%.o,$(CPP_FILES))
TARGET=$(WORKSPACE)/ftpush$(EXE_EXTENSION)

.PHONY : all
all : $(TARGET)

$(TARGET) : $(O_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $+ $(LINKFLAGS) 

.PHONY : test
test : $(TARGET)
	$(TARGET) --help
	$(TARGET) ~/Sites/katso.me/development.ftpush

.PHONY : clean
clean : bump
	rm -f $(O_FILES)
	rm -f $(TARGET)

.PHONY : bump
bump: 
	rm -f build_environment.h
	rm -f main.o

.PHONY : install
install : all
	cp $(TARGET) /opt/local/bin

.PHONY : emacs
emacs :
	emacs .

.PHONY : build_environment.h
build_environment.h : Makefile 
	echo "#ifndef h26395c1ec663488086eed1967515245e" > $@
	echo "#define h26395c1ec663488086eed1967515245e" >> $@
	export | grep -v "export _=" | sed "s/^export \([^=]*\)[=]*/#define \1 /g;" >> $@
	if [ "$(BUILD_NUMBER)" == "" ]; then echo "#define BUILD_NUMBER \"0\""  >> $@ ; fi
	if [ "$(BUILD_ID)" == "" ]; then echo "#define BUILD_ID \"Developer Build, `date`\""  >> $@ ; fi
	echo "#endif" >> $@

$(WORKSPACE)/main.o : main.cpp build_environment.h Makefile


main.precompiled : main.cpp
	$(CXX) $(CXXFLAGS) -dM -E $< > $@


.PHONY : diff
diff : diff.txt

.PHONY : diff.txt
diff.txt : 
	@svn diff > $@
	@svn status | egrep ^X | sed "s/^X//g" | xargs svn diff >> $@
	@echo "86E93DC8-89E5-45B6-B2D1-BF46989F143F - Geneerated diff file - Please edit and try again" >> $@
	@echo "$@:1:"
	@cat $@


.PHONY : commit
commit : 
	@grep "86E93DC8-89E5-45B6-B2D1-BF46989F143F" diff.txt | sed "s/86E93DC8-89E5-45B6-B2D1-BF46989F143F//g"
	@! grep -q "86E93DC8-89E5-45B6-B2D1-BF46989F143F" diff.txt
	@svn status | egrep ^X | sed "s/^X//g" | xargs -n 1 svn ci -F diff.txt
	@svn ci -F diff.txt
	@echo "86E93DC8-89E5-45B6-B2D1-BF46989F143F - You used this diff to commit already" >> diff.txt
