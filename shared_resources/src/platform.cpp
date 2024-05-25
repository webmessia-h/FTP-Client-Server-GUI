#include "../include/platform.hpp"
#include <cstddef>
#include <iostream>

auto Platform::format_file_size(std::uintmax_t size) {
  const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
  int suffixIndex = 0;

  while (size >= 1024 && suffixIndex < 4) {
    size /= 1024;
    suffixIndex++;
  }

  std::ostringstream out;
  out << std::fixed << std::setprecision(2) << static_cast<double>(size);
  std::string result = out.str();

  // Truncate the decimals to two values
  auto pos = result.find('.');
  if (pos != std::string::npos && result.length() > pos + 3) {
    result.erase(pos + 3, std::string::npos);
  }

  return (result + suffixes[suffixIndex]);
}

std::vector<std::string> Platform::list_directory(const std::string &path) {
  std::vector<std::string> items;
  try {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      //
      if (std::filesystem::is_regular_file(entry)) {
        std::string fileName = entry.path().filename().string();
        size_t file_size = std::filesystem::file_size(entry);
        items.push_back(fileName + " - " + format_file_size(file_size));
      }
    }
  } catch (const std::exception &ex) {
    std::cerr << "Error listing directory: " << ex.what() << std::endl;
  }
  return items;
}

// Was used in CLI-version may be needed if fileDialog provides path input field
/*std::string Platform::resolve_relative_path(const std::string &relative_path)
{
    if (relative_path.empty()) {
        throw std::invalid_argument("Empty path provided");
    }

    // Check if the path starts with a tilde (~)
    if (relative_path[0] == '~') {
        // Get the home directory of the current user
        const char *home_dir = getenv("HOME");
        if (!home_dir) {
            struct passwd *pw = getpwuid(getuid());
            if (pw && pw->pw_dir) {
                home_dir = pw->pw_dir;
            } else {
                throw std::runtime_error("Failed to determine home directory");
            }
        }
        // Construct the absolute path
        std::string absolute_path = std::string(home_dir) +
relative_path.substr(1);
        // Return the absolute path
        return absolute_path;
    }

    // Otherwise, the path is already absolute
    return relative_path;
}*/

file_info Platform::read_file(const std::string &filename,
                              const std::string &base_directory) {
  std::filesystem::path file_path =
      std::filesystem::path(base_directory) / filename;
  file_info FILE = {std::ifstream(), 0};

  try {
    if (!std::filesystem::exists(file_path) ||
        !std::filesystem::is_regular_file(file_path)) {
      std::cerr << "Error: File does not exist or is not a regular file: "
                << file_path << std::endl;
      return FILE;
    }

    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
      std::cerr << "Error: Failed to open file: " << strerror(errno) << '\t'
                << "'" << file_path << "'" << std::endl;
      return FILE;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    FILE.file_stream = std::move(file);
    FILE.file_size = file_size;

  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Filesystem error: " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return FILE; // must close file by itself according to RAII, hovewer
               // explicitly closed in send_file() :)
}

bool Platform::write_file(const std::string filename, const char *buffer,
                          size_t buffer_size, std::ofstream &file) {
  // std::string absolute_path = resolve_relative_path(filename);
  file.write(buffer, buffer_size);
  if (!file.good()) {
    std::cerr << "Error: Failed to write to file. " << strerror(errno)
              << std::endl;
    return false;
  }
  return true;
}

bool Platform::cleanup_handler(char *buffer) {
  if (buffer != nullptr) {
    delete[] buffer;
    buffer = nullptr;
    return true;
  }
  return false;
}
