#pragma once

#ifndef CMS_POST_HPP
#define CMS_POST_HPP

/***
###############################################################################
# Includes
###############################################################################
***/

#include "keyValueCache.hpp"
#include "properties.hpp"
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
                services::KeyValueCache &cacheRef);

  // Default destructor
  ~Post() = default;

  string getPost(const std::string_view &host,
                 const bsoncxx::stdx::string_view &dbName, int postId);

private:
  std::shared_ptr<mongocxx::pool> pool;
  services::KeyValueCache &cache;
};

Post::Post(std::shared_ptr<mongocxx::pool> dbPool,
           services::KeyValueCache &cacheRef)
    : pool(dbPool), cache(cacheRef) {
  if (!pool) {
    throw std::invalid_argument("Invalid or null mongodb pool");
  }
}

string Post::getPost(const std::string_view &host,
                     const bsoncxx::stdx::string_view &dbName,
                     const int postId) {
  std::string cacheKey{dbName};
  cacheKey.append("_post_");
  cacheKey += std::to_string(postId);

  if (auto cacheValue = cache.get(cacheKey)) {
    return cacheValue.value();
  }

  string post;
  string titleTag;

  try {
    auto client = pool->acquire();
    auto db = client[dbName];

    auto collection = db[kPostsCollection];

    mongocxx::pipeline postByIdPipeline;
    postByIdPipeline.lookup(make_document(
        kvp(kFromField, kModesCollection), kvp(kLocalField, kModeIdField),
        kvp(kForeignField, kIdField), kvp("as", "mode")));
    postByIdPipeline.unwind("$mode");
    postByIdPipeline.lookup(make_document(
        kvp(kFromField, kLayoutsCollection), kvp(kLocalField, kLayoutIdField),
        kvp(kForeignField, kIdField), kvp("as", "layout")));
    postByIdPipeline.unwind("$layout");
    postByIdPipeline.match(make_document(kvp(kIdField, postId)));
    auto cursor = collection.aggregate(postByIdPipeline);

    int modeId = MODE_HTML;
    bool found = false;
    for (const auto &doc : cursor) {
      found = true;
      if (doc[kModeIdField]) {
        modeId = doc[kModeIdField].type() == bsoncxx::type::k_int32
                     ? doc[kModeIdField].get_int32().value
                     : static_cast<int>(doc[kModeIdField].get_int64().value);
      }

      auto titleValue = doc[kTitleField].get_string().value;
      titleTag.append("<title>");
      titleTag.append(titleValue.data(), titleValue.size());
      titleTag.append("</title>");

      auto headerValue = doc[kLayoutField][kHeaderField].get_string().value;
      post.append(headerValue.data(), headerValue.size());

      auto contentValue = doc[kContentField].get_string().value;
      bsoncxx::types::b_date createdAt = doc[kCreatedAtField].get_date();

      if (modeId == MODE_MARKDOWN) {
        auto html = std::unique_ptr<char, void (*)(void *)>(
            cmark_markdown_to_html(contentValue.data(), contentValue.size(),
                                   CMARK_OPT_DEFAULT),
            std::free);
        post.append("<h1>");
        post.append(titleValue.data(), titleValue.size());
        post.append("</h1><h4>");
        post.append(string_util::timestamp(createdAt));
        post.append("</h4>");
        post.append(html.get());
      } else if (modeId == MODE_HTML) {
        post.append(contentValue.data(), contentValue.size());
      }

      auto footerValue = doc[kLayoutField][kFooterField].get_string().value;
      post.append(footerValue.data(), footerValue.size());

      /***
       * //DEBUG
       * std::cout << bsoncxx::to_json(doc) << std::endl;
       ***/
      break;
    }

    if (!found) {
      spdlog::warn("No result for Post::getPost => {}", postId);
    }
  } catch (const std::exception &e) {
    spdlog::error("Post::getPost exception: {}", e.what());
  }

  string_util::StringReplacer replacer("<title>%REPLACE_WITH_TITLE_ID%</title>",
                                       titleTag);
  string resultPost = replacer.replace(post, 1);

  int64_t ttl = 900; // 15 minutes TTL.
  if (cache.set(cacheKey, resultPost, ttl)) {
    spdlog::info("get post ({}) set cache ({})", postId, cacheKey);
  } else {
    spdlog::error("get post ({}) failed to set cache ({})", postId, cacheKey);
  }
  return resultPost;
}

} // namespace cms

#endif // CMS_POST_HPP
