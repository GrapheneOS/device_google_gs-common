#pragma once

#include <ostream>
#include <string_view>

namespace pixel_modem::logging {

/**
 * @brief Data object for information about dumpings logs.
 *
 * @param src_dir is a const char* containing the path to the directory to be
 copied.
 * @param dest_dir is a const char* containing the path to the directory that
 the contents of the source directory should be copied to.
 * @param limit is an int of the maximum number of files to copy.
 * @param prefix is a const char* containing a prefix that all files to be
 copied should have.
*/
struct LogDumpInfo {
  const std::string_view src_dir;
  const std::string_view dest_dir;
  int limit;
  const std::string_view prefix;

  friend bool operator==(const LogDumpInfo& lhs, const LogDumpInfo& rhs) {
    return (lhs.limit == rhs.limit) && (lhs.src_dir == rhs.src_dir) &&
           (lhs.dest_dir == rhs.dest_dir) && (lhs.prefix == rhs.prefix);
  }

  // Do I have to use .data() here?
  friend std::ostream& operator<<(std::ostream& os, const LogDumpInfo& obj) {
    os << "src_dir: " << obj.src_dir << ", dest_dir: " << obj.dest_dir
       << ", limit: " << obj.limit << ", prefix: " << obj.prefix;
    return os;
  }
};

/**
 * @brief Data object for information about dumpings logs.
 *
 * @param src_dir is a const char* containing the path to a file to be copied.
 * @param dest_dir is a const char* containing the destination path for the file
 * to be copied to.
 */
struct FileCopyInfo {
  const std::string_view src_dir;
  const std::string_view dest_dir;

  friend bool operator==(const FileCopyInfo& lhs, const FileCopyInfo& rhs) {
    return (lhs.src_dir == rhs.src_dir) && (lhs.dest_dir == rhs.dest_dir);
  }

  // Do I have to add .data() here?
  friend std::ostream& operator<<(std::ostream& os, const FileCopyInfo& obj) {
    os << "src_dir: " << obj.src_dir << ", dest_dir: " << obj.dest_dir;
    return os;
  }
};

/**
 * @brief Interface for dumping modem logs and files.
 */
class Dumper {
 public:
  virtual ~Dumper() = default;
  virtual void DumpLogs(const LogDumpInfo& log_dump_info);
  virtual void CopyFile(const FileCopyInfo& file_copy_info);
};

}  // namespace pixel_modem::logging
