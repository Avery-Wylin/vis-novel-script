#include <cstdio>
#include <pthread.h>
#include <enet/enet.h>
#include <bit>
#include <pthread.h>
#include "VNDebug.h"
#include "VNInterpreter.h"
#include "Window.h"

pthread_mutex_t window_init_lock;
pthread_cond_t window_init_cond;

int main( int, char ** ) {

    // Enforce that the processer is little endian. Otherwise assets will load incorrectly and inter-machine communications will fail.
    if( std::endian::native != std::endian::little ) {
        puts("ERROR: System must be little endian.");
        exit(EXIT_FAILURE);
    }

    VNOP::load_ops();
    VNDebug::enabled = true;
    Audio::init();
    VNI::window.init();
    VNI::window.run();
    puts("Succesfully Exited");
    fflush(stdout);
    return 0;
}
