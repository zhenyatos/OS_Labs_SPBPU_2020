#include "Error.h"
#include "Daemon.h"
#include <iostream>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "You should specify configuration file\n";
        return EXIT_FAILURE;
    }

    Daemon* daemon = nullptr;

    try {
        daemon = Daemon::get();
        daemon->run(argv[1]);
    }
    catch (Error error) {
        delete daemon;
        if (error == Error::OK)
            return EXIT_SUCCESS;
        else
            return EXIT_FAILURE;
    }

    delete daemon;
    return EXIT_SUCCESS;
}
