########
# Utils
########

MKDIR=mkdir -p

########
# Paths
########

BUILD_RELEASE_DIR:=./build_release

BUILD_TYPE:=RELEASE
ARCH:=$(shell uname -m)

UNAME:=$(shell uname)
NPROC:=1
ifeq ($(UNAME),Darwin)
    NPROC:=$(shell sysctl -n hw.ncpu)
else
    NPROC:=$(shell nproc)
endif

########
# S2T-OBS
########
S2T_OBS_BUILD_CMAKE:=${BUILD_RELEASE_DIR}/CMakeLists.txt