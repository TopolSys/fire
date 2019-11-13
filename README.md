# FIRE
Format Independent Routing Environment


# Compile
## Recommendation Setup
The developement of this project minimizes the dependency on third-party tools. The only known requirement is C++ version: 
+ g++ (GCC) 5.5.0 C++11

Compile the project with:
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
