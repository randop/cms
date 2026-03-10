#pragma once

#ifndef CMS_POST_HPP
#define CMS_POST_HPP

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
namespace pt = boost::posix_time;
namespace gr = boost::gregorian;

using string = std::string;

class Post {
public:
  // Constructor taking a shared_ptr to ConnectionPool
  explicit Post(std::shared_ptr<mongocxx::pool> dbPool,
                std::shared_ptr<Content> contentPtr);

  // Default destructor
  ~Post() = default;

  string getPost(const std::string_view &host,
                 const bsoncxx::stdx::string_view &dbName, int postId);

private:
  std::shared_ptr<mongocxx::pool> pool;
  std::shared_ptr<Content> content;
  ContentIdType idType;
};

Post::Post(std::shared_ptr<mongocxx::pool> dbPool,
           std::shared_ptr<Content> contentPtr)
    : pool(dbPool), content(contentPtr), idType(ContentIdType::Integer) {
  if (!pool) {
    throw std::invalid_argument("Invalid or null mongodb pool");
  }
}

string Post::getPost(const std::string_view &host,
                     const bsoncxx::stdx::string_view &dbName,
                     const int postId) {
  const string idValue = std::to_string(postId);
  return content->render(host, dbName, "post", kPostsCollection, idType,
                         kIdField, idValue, kTitleField);
}

} // namespace cms

#endif // CMS_POST_HPP
