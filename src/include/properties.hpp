#pragma once

#ifndef CMS_PROPERTIES_HPP
#define CMS_PROPERTIES_HPP

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>

namespace cms {

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

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
} // namespace cms

#endif // CMS_PROPERTIES_HPP
