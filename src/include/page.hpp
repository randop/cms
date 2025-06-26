#pragma once

#ifndef CMS_PAGE_HPP
#define CMS_PAGE_HPP

/***
###############################################################################
# Includes
###############################################################################
***/

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <cmark.h>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/pipeline.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "include/stringUtil.hpp"
#include "keyValueCache.hpp"

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

constexpr bsoncxx::stdx::string_view kIdField{"id"};
constexpr bsoncxx::stdx::string_view kFromField{"from"};
constexpr bsoncxx::stdx::string_view kLocalField{"localField"};
constexpr bsoncxx::stdx::string_view kForeignField{"foreignField"};
constexpr bsoncxx::stdx::string_view kModesCollection{"modes"};
constexpr bsoncxx::stdx::string_view kModeIdField{"modeId"};
constexpr bsoncxx::stdx::string_view kLayoutsCollection{"layouts"};
constexpr bsoncxx::stdx::string_view kLayoutIdField{"layoutId"};
constexpr bsoncxx::stdx::string_view kPagesCollection{"pages"};
constexpr bsoncxx::stdx::string_view kTitleField{"title"};
constexpr bsoncxx::stdx::string_view kContentField{"content"};
constexpr bsoncxx::stdx::string_view kHeaderField{"header"};
constexpr bsoncxx::stdx::string_view kFooterField{"footer"};
constexpr bsoncxx::stdx::string_view kLayoutField{"layout"};
constexpr bsoncxx::stdx::string_view kCreatedAtField{"createdAt"};

using string = std::string;

class Page {
public:
  // Constructor taking a shared_ptr to mongodb uri
  explicit Page(std::shared_ptr<mongocxx::pool> dbPool,
                services::KeyValueCache &cacheRef);

  // Default destructor
  ~Page() = default;

  string getPage(const std::string_view &host,
                 const bsoncxx::stdx::string_view &dbName,
                 const string &pageId);

private:
  std::shared_ptr<mongocxx::pool> pool;
  services::KeyValueCache &cache;
};

Page::Page(std::shared_ptr<mongocxx::pool> dbPool,
           services::KeyValueCache &cacheRef)
    : pool(dbPool), cache(cacheRef) {
  if (!pool) {
    throw std::invalid_argument("Invalid or null mongodb pool");
  }
}

string Page::getPage(const std::string_view &host,
                     const bsoncxx::stdx::string_view &dbName,
                     const string &pageId) {
  string cacheKey{dbName};
  cacheKey.append("_page_");
  cacheKey.append(pageId);

  if (auto cacheValue = cache.get(cacheKey)) {
    return cacheValue.value();
  }

  string page;
  string titleTag;

  try {
    auto client = pool->acquire();
    auto db = client[dbName];

    auto collection = db[kPagesCollection];

    mongocxx::pipeline pageByIdPipeline;
    pageByIdPipeline.lookup(make_document(
        kvp(kFromField, kModesCollection), kvp(kLocalField, kModeIdField),
        kvp(kForeignField, kIdField), kvp("as", "mode")));
    pageByIdPipeline.unwind("$mode");

    pageByIdPipeline.lookup(make_document(
        kvp(kFromField, kLayoutsCollection), kvp(kLocalField, kLayoutIdField),
        kvp(kForeignField, kIdField), kvp("as", "layout")));
    pageByIdPipeline.unwind("$layout");

    pageByIdPipeline.match(make_document(kvp(kIdField, pageId)));
    auto cursor = collection.aggregate(pageByIdPipeline);

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
      page.append(headerValue.data(), headerValue.size());

      auto contentValue = doc[kContentField].get_string().value;
      bsoncxx::types::b_date createdAt = doc[kCreatedAtField].get_date();

      if (modeId == MODE_MARKDOWN) {
        auto html = std::unique_ptr<char, void (*)(void *)>(
            cmark_markdown_to_html(contentValue.data(), contentValue.size(),
                                   CMARK_OPT_DEFAULT),
            std::free);
        page.append("<h1>");
        page.append(titleValue.data(), titleValue.size());
        page.append("</h1><h4>");
        page.append(string_util::timestamp(createdAt));
        page.append("</h4>");
        page.append(html.get());
      } else if (modeId == MODE_HTML) {
        page.append(contentValue.data(), contentValue.size());
      }

      auto footerValue = doc[kLayoutField][kFooterField].get_string().value;
      page.append(footerValue.data(), footerValue.size());

      /***
       * //DEBUG
       * std::cout << bsoncxx::to_json(doc) << std::endl;
       ***/
      break;
    }

    if (!found) {
      spdlog::warn("No result for Page::getPage => {}", pageId);
    }
  } catch (const std::exception &e) {
    spdlog::error("get page failure: {}", e.what());
  }

  string_util::StringReplacer replacer("<title>%REPLACE_WITH_TITLE_ID%</title>",
                                       titleTag);
  string resultPage = replacer.replace(page, 1);

  int64_t ttl = 900; // 15 minutes TTL.
  if (cache.set(cacheKey, resultPage, ttl)) {
    spdlog::info("get page ({}) set cache ({})", pageId, cacheKey);
  } else {
    spdlog::error("get page ({}) failed to set cache ({})", pageId, cacheKey);
  }
  return resultPage;
}

} // namespace cms

#endif // CMS_PAGE_HPP
