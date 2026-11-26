#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "kv_store.h"
#include <string>

std::string handle_command(KVStore &store, const std::string &command);

#endif