#pragma once

#ifndef CMS_CONTENT_HPP
#define CMS_CONTENT_HPP

/***
###############################################################################
# Includes
###############################################################################
***/
#include "keyValueCache.hpp"
#include "stringUtil.hpp"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/types.hpp>
#include <charconv>
#include <cmark.h>
#include <mongocxx/client.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/pool.hpp>
#include <spdlog/spdlog.h>
#include <string>

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
using std::string;

namespace pt = boost::posix_time;
namespace gr = boost::gregorian;

constexpr bsoncxx::stdx::string_view kIdField{"id"};
constexpr bsoncxx::stdx::string_view kFromField{"from"};
constexpr bsoncxx::stdx::string_view kLocalField{"localField"};
constexpr bsoncxx::stdx::string_view kForeignField{"foreignField"};
constexpr bsoncxx::stdx::string_view kModesCollection{"modes"};
constexpr bsoncxx::stdx::string_view kModeIdField{"modeId"};
constexpr bsoncxx::stdx::string_view kLayoutsCollection{"layouts"};
constexpr bsoncxx::stdx::string_view kLayoutIdField{"layoutId"};
constexpr bsoncxx::stdx::string_view kPostsCollection{"posts"};
constexpr bsoncxx::stdx::string_view kPagesCollection{"pages"};
constexpr bsoncxx::stdx::string_view kTitleField{"title"};
constexpr bsoncxx::stdx::string_view kContentField{"content"};
constexpr bsoncxx::stdx::string_view kHeaderField{"header"};
constexpr bsoncxx::stdx::string_view kFooterField{"footer"};
constexpr bsoncxx::stdx::string_view kLayoutField{"layout"};
constexpr bsoncxx::stdx::string_view kCreatedAtField{"createdAt"};

class Content {
public:
  explicit Content(std::shared_ptr<mongocxx::pool> dbPool,
                   services::KeyValueCache &cacheRef);

  // Default destructor
  ~Content() = default;

  string
  render(const std::string_view &host, const bsoncxx::stdx::string_view &dbName,
         const string &cachePrefix,
         const bsoncxx::stdx::string_view &collectionName,
         const ContentIdType &idType, const bsoncxx::stdx::string_view &idField,
         const string &idValue, const bsoncxx::stdx::string_view &titleField);

private:
  std::shared_ptr<mongocxx::pool> pool;
  services::KeyValueCache &cache;
};

Content::Content(std::shared_ptr<mongocxx::pool> dbPool,
                 services::KeyValueCache &cacheRef)
    : pool(dbPool), cache(cacheRef) {
  if (!pool) {
    throw std::invalid_argument("Invalid or null mongodb pool");
  }
}

string Content::render(
    const std::string_view &host, const bsoncxx::stdx::string_view &dbName,
    const string &cachePrefix, const bsoncxx::stdx::string_view &collectionName,
    const ContentIdType &idType, const bsoncxx::stdx::string_view &idField,
    const string &idValue, const bsoncxx::stdx::string_view &titleField) {
  string cacheKey{dbName};
  cacheKey.append("_");
  cacheKey.append(cachePrefix);
  cacheKey.append("_");
  cacheKey.append(idValue);

  // Check cache
  if (auto cacheValue = cache.get(cacheKey)) {
    return cacheValue.value();
  }

  string content;
  string titleTag;

  try {
    auto client = pool->acquire();
    auto db = client[dbName];
    auto collection = db[collectionName];

    mongocxx::pipeline byIdPipeline;
    byIdPipeline.lookup(make_document(
        kvp(kFromField, kModesCollection), kvp(kLocalField, kModeIdField),
        kvp(kForeignField, kIdField), kvp("as", "mode")));
    byIdPipeline.unwind("$mode");
    byIdPipeline.lookup(make_document(
        kvp(kFromField, kLayoutsCollection), kvp(kLocalField, kLayoutIdField),
        kvp(kForeignField, kIdField), kvp("as", "layout")));
    byIdPipeline.unwind("$layout");
    if (idType == ContentIdType::Integer) {
      int idNum = 0;
      auto [ptr, ec] = std::from_chars(idValue.data(),
                                       idValue.data() + idValue.size(), idNum);
      if (!(ec == std::errc() && ptr == idValue.data() + idValue.size())) {
        idNum = 0;
      }
      byIdPipeline.match(make_document(kvp(idField, idNum)));
    } else {
      byIdPipeline.match(make_document(kvp(idField, idValue)));
    }

    auto cursor = collection.aggregate(byIdPipeline);

    int modeId = MODE_HTML;
    bool found = false;
    for (const auto &doc : cursor) {
      found = true;
      if (doc[idField]) {
        modeId = doc[kModeIdField].type() == bsoncxx::type::k_int32
                     ? doc[kModeIdField].get_int32().value
                     : static_cast<int>(doc[kModeIdField].get_int64().value);
      }

      auto titleValue = doc[titleField].get_string().value;
      titleTag.append("<title>");
      titleTag.append(titleValue.data(), titleValue.size());
      titleTag.append("</title>");

      auto headerValue = doc[kLayoutField][kHeaderField].get_string().value;
      content.append(headerValue.data(), headerValue.size());

      auto contentValue = doc[kContentField].get_string().value;
      bsoncxx::types::b_date createdAt = doc[kCreatedAtField].get_date();

      if (modeId == MODE_MARKDOWN) {
        auto html = std::unique_ptr<char, void (*)(void *)>(
            cmark_markdown_to_html(contentValue.data(), contentValue.size(),
                                   CMARK_OPT_DEFAULT),
            std::free);
        content.append("<h1>");
        content.append(titleValue.data(), titleValue.size());
        content.append("</h1><h4>");
        content.append(string_util::timestamp(createdAt));
        content.append("</h4>");
        content.append(html.get());
      } else if (modeId == MODE_HTML) {
        content.append(contentValue.data(), contentValue.size());
      }

      auto footerValue = doc[kLayoutField][kFooterField].get_string().value;
      content.append(footerValue.data(), footerValue.size());

      /***
       * //DEBUG
       * std::cout << bsoncxx::to_json(doc) << std::endl;
       ***/
      break;
    }

    if (!found) {
      spdlog::warn("No result for Content::render => {}", idValue);
    }
  } catch (const std::exception &e) {
    spdlog::error("Content::render exception: {}", e.what());
  }

  // Replace title tag
  string_util::StringReplacer replacer("<title>%REPLACE_WITH_TITLE_ID%</title>",
                                       titleTag);
  string result = replacer.replace(content, 1);
  // Cache result
  int64_t ttl = 900; // 15 minutes
  if (cache.set(cacheKey, result, ttl)) {
    spdlog::info("get {} ({}) set cache ({})", cachePrefix, idValue, cacheKey);
  } else {
    spdlog::error("get {} ({}) failed to set cache ({})", cachePrefix, idValue,
                  cacheKey);
  }
  return result;
}

} // namespace cms

#endif // CMS_CONTENT_HPP
