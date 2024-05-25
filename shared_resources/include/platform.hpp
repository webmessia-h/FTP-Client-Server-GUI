// platfrom
#pragma once
#include <QObject>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include <vector>
struct file_info {
  std::ifstream file_stream;
  size_t file_size;
};

namespace Platform {
std::vector<std::string> list_directory(const std::string &path);

auto format_file_size(std::uintmax_t size);

std::string resolve_relative_path(const std::string &relative_path);

file_info read_file(const std::string &filename,
                    const std::string &base_directory);

bool write_file(const std::string filename, const char *buffer,
                size_t buffer_size, std::ofstream &file);

bool cleanup_handler(char *buffer);

}; // namespace Platform
