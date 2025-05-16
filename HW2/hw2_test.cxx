#include "hw2_test.h"

long set_sec(int sword, int midnight, int clamp, int duty, int isolate) {
    long r = syscall(334, sword, midnight, clamp, duty, isolate);
    return r;
}

long get_sec(char clr) {
    long r = syscall(335, clr);
    return r;
}

long check_sec(pid_t pid, char clr) {
	long r = syscall(336, pid, clr);
    return r;
}

long flip_sec_branch(int height, char clr) {
	long r = syscall(337, height, clr);
    return r;
}
