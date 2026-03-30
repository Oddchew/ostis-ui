/*
 * This source file is part of an OSTIS project. For the latest info, see
 * http://ostis.net Distributed under the MIT License (See accompanying file
 * COPYING.MIT or copy at http://opensource.org/licenses/MIT)
 */

#include "utils.hpp"

void loadKB(ScAgentContext &context, ScsLoader &loader,
            std::filesystem::path const &path) {
  if (std::filesystem::is_regular_file(path) && path.extension() == ".scs") {
    loader.loadScsFile(context, path);
  } else if (std::filesystem::is_directory(path)) {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      loadKB(context, loader, entry.path());
    }
  }
}
