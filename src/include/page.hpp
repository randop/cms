#pragma once

#ifndef CMS_PAGE_HPP
#define CMS_PAGE_HPP

/***
###############################################################################
# Includes
###############################################################################
***/

#include "content.hpp"
#include "keyValueCache.hpp"
#include "stringUtil.hpp"
#include <array>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <chrono>
#include <cmark.h>
#include <iomanip>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

/***
###############################################################################
# Constants
###############################################################################
***/
#include "include/constants.h"

namespace cms {

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;
using string = std::string;

class Page {
public:
  explicit Page(std::shared_ptr<mongocxx::pool> dbPool,
                std::shared_ptr<Content> contentPtr);

  // Default destructor
  ~Page() = default;

  string getPage(const std::string_view &host,
                 const bsoncxx::stdx::string_view &dbName,
                 const string &pageId);

private:
  std::shared_ptr<mongocxx::pool> pool;
  std::shared_ptr<Content> content;
  ContentIdType idType;
};

Page::Page(std::shared_ptr<mongocxx::pool> dbPool,
           std::shared_ptr<Content> contentPtr)
    : pool(dbPool), content(contentPtr), idType(ContentIdType::String) {
  if (!pool) {
    throw std::invalid_argument("Invalid or null mongodb pool");
  }
}

string Page::getPage(const std::string_view &host,
                     const bsoncxx::stdx::string_view &dbName,
                     const string &pageId) {
  return content->render(host, dbName, "page", kPagesCollection, idType,
                         kIdField, pageId, kTitleField);
}

} // namespace cms

#endif // CMS_PAGE_HPP
