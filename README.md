# FIRE
Fire (Format Independent Routing Environment) is a global router that designed in a way towards eliminating dependency of input format. Highlights of Fire:

+ deterministic parallelization
+ depends only on stable C++ version
+ easy to support any input format by filling out the database with customized interpreter. 

Currently it supports Cadence LEF/DEF format.

# Compile
This project minimizes the dependency on third-party tools. Standard installation of unix/linux-like system with following feature should be sufficient to Fire: 

+ GNU g++/gcc 5.5.0 C++11 (2015)
+ GNU Make 3.81 (2006)
+ GNU Bison 2.4.1 (2008)

Author developed Fire in CentOS 6.4 with compiler updated to g++ 5.5.0

Compile Fire with:
```bash
$ git clone https://github.com/TopolSys/fire.git 
$ cd fire
$ make -f main.mk
```
If success, the executable is called "lem" under the top level folder.

# Run 
It takes following parameters as input:
```bash
./lem -lef <LEF file> -def <DEF file> [-threads <NUM_THREADs>] -output <GUIDE_FILE_NAME>
```
For example: 
```bash
./lem -lef ispd18_test1.lef -def ispd18_test1.def -threads 8 -output ispd18_test1.guide
```
An alternative way to specify a design is:
```bash
./lem -efp <PATH_TO_LEF/DEF_FILES> [-threads <NUM_THREADs>] -output <GUIDE_FILE_NAME>
```
lem searchs lef/def files under the path specified with -efp option. 
For example: 
```bash
./lem -efp ispd18_test1/ -threads 8 -output ispd18_test1.guide
```
# Code Invocation
+ [FLUTE](http://home.eng.iastate.edu/~cnchu/flute.html)
  + a modified version of FLUTE package supports the steiner tree consturction of this project. 
+ [ISPD 2019: Organizer's Parser](http://www.ispd.cc/contests/19/tutorial.htm)
  + a modified parser based on ISPD 2019's opensource enables our support for Cadence LEF/DEF format. 
+ [ABC](https://people.eecs.berkeley.edu/~alanmi/abc/)
  + the makefile and file dependency analyzer are from ABC system. 
