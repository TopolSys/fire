#ABCROOT=/home/VMCentOS-00/abc
ROOT = $(shell pwd)

#DEF_DIR=${ROOT}/DEF-Parser
#LEF_DIR=${ROOT}/LEF
#IFLAG=-I${ABCROOT}/src/
#LFLAG=-L${ABCROOT}
#override IFLAG += -I${LEF_DIR}/include -I${DEF_DIR}/include
#override LFLAG += -L${LEF_DIR}/lib -L${DEF_DIR}/lib

EFPARSER=${ROOT}/lib/efparser/ispd2019_opensource/src
EFPARSER_H=${ROOT}/lib/efparser/
FLUTE=${ROOT}/lib/flute/

$(info $$ROOT ${ROOT})
$(info $$EFPARSER ${EFPARSER})
$(info $$FLUTE ${FLUTE})

override IFLAG += -I${EFPARSER_H}/ -I${FLUTE}/ -I./
override LFLAG += -L${EFPARSER} -L${FLUTE}/
CFLAG=-std=c++11 -O3 
LIB  = -static-libstdc++ -lef -lflute -lm -ldl -lrt -rdynamic -lreadline -lpthread

SRC += $(wildcard *.cpp)
MODULES := db alg/rsmt alg/rmst utils/sys utils/itp core/tex 
include $(patsubst %, %/module.make, $(MODULES))

OBJFILES := \
		$(patsubst %.cc, %.o, $(filter %.cc, $(SRC))) \
		$(patsubst %.cpp, %.o, $(filter %.cpp, $(SRC))) \
		$(patsubst %.c, %.o,  $(filter %.c, $(SRC)))  \
		$(patsubst %.y, %.o,  $(filter %.y, $(SRC)))

DEPFILES	:= $(OBJFILES:.o=.d)
GPP  =g++

.PHONY: all default clean

EXE			:= lem


#$(info $$SRCFILES : [${SRC}] )
#$(info $$OBJFILES : [${OBJFILES}] )
#$(info $$DEPFILES : [${DEPFILES}] )

all: $(EXE)
default: $(EXE)


%.o: %.cpp
	@echo "Compiling $@"
	@${GPP} ${CFLAG} ${IFLAG} -c $< -o $@

%.d: %.cpp
	@echo "Generating dependency $@"
	@./depends.sh $(CXX) `dirname $*.cpp` ${CFLAG} ${IFLAG} $*.cpp > $@

-include $(DEPFILES)

depend: $(DEPFILES)


$(EXE): $(OBJFILES)
	cd ${EFPARSER}/lefdef/lef; make;\
	cd ${EFPARSER}/lefdef/def; make;\
	cd ${EFPARSER}/; make libef.a;
	cd ${FLUTE}; make; make libflute.a;\
	cd ${ROOT}
	@echo "Linking"
	@${GPP} ${LFLAG} -o $(EXE) $(OBJFILES) $(LIB)

clean:
	rm -f $(EXE) $(OBJFILES) $(DEPFILES)
