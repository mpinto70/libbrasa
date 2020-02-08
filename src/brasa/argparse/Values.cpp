#include "Values.h"

void brasa::argparse::assert_can_digest(bool can_digest, const std::string& argument) {
    if (not can_digest) {
        throw InvalidArgument("Cannot digest " + argument);
    }
}
