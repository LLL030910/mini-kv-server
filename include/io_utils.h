#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <string>

void write_all(int fd, const void *data, std::size_t len);

std::string read_to_end(int fd);

void set_nonblocking(int fd);

void send_all(int fd, const void *data, std::size_t size);

#endif