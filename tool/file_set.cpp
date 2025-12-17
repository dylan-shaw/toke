#include "file_set.h"

#include "file_visitor.h"

#include <toke/cxx_api.hpp>

#include <fstream>
#include <vector>

namespace {

using Path = std::filesystem::path;

class FileSetImpl final : public FileSet
{
public:
  FileSetImpl(const Path& root, const bool cache, const std::string_view& filterConfig)
    : m_filter(toke::Filter::create())
  {
    m_filter->parseConfig(filterConfig);

    buildFileList(root, m_paths);

    if (cache) {
      buildCache(m_buffers, m_paths, *m_filter);
    }
  }

  void accept(FileVisitor& visitor) const override
  {
    visitor.prepare(m_paths.size());

#pragma omp parallel for

    for (int i = 0; i < static_cast<int>(m_paths.size()); i++) {

      std::string buffer;

      std::string_view data;

      if (m_buffers.empty()) {
        buffer = openFile(m_paths[i], *m_filter);
        data = std::string_view(buffer);
      } else {
        data = m_buffers[i];
      }

      visitor.visitFile(data, i);
    }

    visitor.finalize();
  }

protected:
  [[nodiscard]] static auto openFile(const std::filesystem::path& path, const toke::Filter& filter) -> std::string
  {
    std::ifstream file(path, std::ios::binary | std::ios::in);
    if (!file.good()) {
      return {};
    }

    file.seekg(0, std::ios::end);

    const auto fileSize = file.tellg();
    if (fileSize < 0) {
      return {};
    }

    file.seekg(0, std::ios::beg);

    std::string buffer;

    buffer.resize(static_cast<size_t>(fileSize));

    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

    buffer.resize(file.gcount());

    return filter.filter(buffer);
  }

  static void buildFileList(const std::filesystem::path& root, std::vector<Path>& paths)
  {
    for (const auto& entry : std::filesystem::directory_iterator(root)) {
      if (entry.is_directory()) {
        buildFileList(entry.path(), paths);
      } else if (entry.is_regular_file()) {
        paths.emplace_back(entry.path());
      }
    }
  }

  static void buildCache(std::vector<std::string>& buffers, const std::vector<Path>& paths, const toke::Filter& filter)
  {
    buffers.resize(paths.size());

    for (size_t i = 0; i < paths.size(); i++) {
      buffers[i] = openFile(paths[i], filter);
    }
  }

private:
  std::unique_ptr<toke::Filter> m_filter;

  std::vector<Path> m_paths;

  std::vector<std::string> m_buffers;
};

} // namespace

auto
FileSet::create(const std::filesystem::path& root, const bool cache, const std::string_view& filterConfig)
  -> std::unique_ptr<FileSet>
{
  return std::make_unique<FileSetImpl>(root, cache, filterConfig);
}
