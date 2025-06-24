#pragma once

#ifndef CMS_HTTP_SERVER_HPP
#define CMS_HTTP_SERVER_HPP

/***
###############################################################################
# Includes
###############################################################################
***/

#include <spdlog/spdlog.h>

#include "include/page.hpp"
#include "include/post.hpp"

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>
#include <thread>
#include <vector>

#include <boost/asio/coroutine.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/url.hpp>
#include <bsoncxx/stdx/string_view.hpp>

constexpr bsoncxx::stdx::string_view LOCALHOST_DB{"localhost"};
constexpr bsoncxx::stdx::string_view QUIZBIN_DB{"quizbin"};

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path) {
  using beast::iequals;
  auto const ext = [&path] {
    auto const pos = path.rfind(".");
    if (pos == beast::string_view::npos)
      return beast::string_view{};
    return path.substr(pos);
  }();
  if (iequals(ext, ".htm"))
    return "text/html";
  if (iequals(ext, ".html"))
    return "text/html";
  if (iequals(ext, ".php"))
    return "text/html";
  if (iequals(ext, ".css"))
    return "text/css";
  if (iequals(ext, ".txt"))
    return "text/plain";
  if (iequals(ext, ".js"))
    return "application/javascript";
  if (iequals(ext, ".json"))
    return "application/json";
  if (iequals(ext, ".xml"))
    return "application/xml";
  if (iequals(ext, ".swf"))
    return "application/x-shockwave-flash";
  if (iequals(ext, ".flv"))
    return "video/x-flv";
  if (iequals(ext, ".png"))
    return "image/png";
  if (iequals(ext, ".jpe"))
    return "image/jpeg";
  if (iequals(ext, ".jpeg"))
    return "image/jpeg";
  if (iequals(ext, ".jpg"))
    return "image/jpeg";
  if (iequals(ext, ".gif"))
    return "image/gif";
  if (iequals(ext, ".bmp"))
    return "image/bmp";
  if (iequals(ext, ".ico"))
    return "image/vnd.microsoft.icon";
  if (iequals(ext, ".tiff"))
    return "image/tiff";
  if (iequals(ext, ".tif"))
    return "image/tiff";
  if (iequals(ext, ".svg"))
    return "image/svg+xml";
  if (iequals(ext, ".svgz"))
    return "image/svg+xml";
  return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path) {
  if (base.empty())
    return std::string(path);
  std::string result(base);
#ifdef BOOST_MSVC
  char constexpr path_separator = '\\';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
  for (auto &c : result)
    if (c == '/')
      c = path_separator;
#else
  char constexpr path_separator = '/';
  if (result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());
#endif
  return result;
}

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
http::message_generator
handle_request(std::shared_ptr<blog::Post> post,
               std::shared_ptr<blog::Page> page, beast::string_view doc_root,
               http::request<Body, http::basic_fields<Allocator>> &&req) {
  // Returns a bad request response
  auto const bad_request = [&req](beast::string_view why) {
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = std::string(why);
    res.prepare_payload();
    return res;
  };

  // Returns a not found response
  auto const not_found = [&req](beast::string_view target) {
    http::response<http::string_body> res{http::status::not_found,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "The resource '" + std::string(target) + "' was not found.";
    res.prepare_payload();
    return res;
  };

  // Returns a server error response
  auto const server_error = [&req](beast::string_view what) {
    http::response<http::string_body> res{http::status::internal_server_error,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    res.body() = "An error occurred: '" + std::string(what) + "'";
    res.prepare_payload();
    return res;
  };

  // Returns html response
  auto const html_response = [&req](std::string &response) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, CONTENT_TYPE_HTML);
    res.keep_alive(req.keep_alive());
    res.body() = response;
    res.prepare_payload();
    return res;
  };

  // Make sure we can handle the method
  if (req.method() != http::verb::get && req.method() != http::verb::head) {
    return bad_request("Unknown HTTP-method");
  }

  // Request path must be absolute and not contain "..".
  if (req.target().empty() || req.target()[0] != '/' ||
      req.target().find("..") != beast::string_view::npos) {
    return bad_request("Illegal request-target");
  }

  // Get the host from the Host header
  std::string_view host = req[http::field::host];
  bsoncxx::stdx::string_view dbName = LOCALHOST_DB;
  if (host.find("quizbin.com") != beast::string_view::npos) {
    dbName = QUIZBIN_DB;
  }

  // Build the path to the requested file
  std::string path = path_cat(doc_root, req.target());

  /** DEBUG
  spdlog::debug("load page <{}> host <{}> db <{}>", req.target(), host, dbName);
  **/

  std::string urlRequest{"http://"};
  urlRequest.append(host);
  urlRequest.append(req.target());
  boost::system::result<boost::urls::url_view> urlResult =
      boost::urls::parse_uri(urlRequest);

  if (!urlResult.has_value()) {
    return server_error("URL error encountered");
  }
  if (urlResult.has_error()) {
    return server_error("URL error encountered");
  }

  boost::urls::url_view urlView = urlResult.value();
  auto segments = urlView.segments();

  if (req.method() == http::verb::get && segments.size() == 0) {
    // Handle the index route (/)
    std::string content = page->getPage(host, dbName, "index");
    return html_response(content);
  } else if (req.method() == http::verb::get && segments.size() == 1 &&
             req.target() == "/about") {
    std::string content = page->getPage(host, dbName, "about");
    return html_response(content);
  } else if (req.method() == http::verb::get &&
             req.target().find("/posts/") != beast::string_view::npos) {
    int postId = NONE_POST_ID;
    if (segments.size() >= 2) {
      int index = 0;

      for (const auto &segment : segments) {
        if (index == 1) {
          // Converts string segment to int with numeric validation
          auto [ptr, ec] = std::from_chars(
              segment.data(), segment.data() + segment.size(), postId);
          if (ec == std::errc() && ptr == segment.data() + segment.size()) {
            break;
          } else {
            postId = NONE_POST_ID;
          }
          break;
        }
        ++index;
      }

      if (postId == NONE_POST_ID) {
        return not_found(req.target());
      }

      std::string content = post->getPost(postId);

      if (postId > NONE_POST_ID && !content.empty()) {
        return html_response(content);
      }
    }
    return not_found(req.target());
  }

  // Attempt to open the file
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if (ec == beast::errc::no_such_file_or_directory) {
    return not_found(req.target());
  }

  // Handle an unknown error
  if (ec) {
    return server_error(ec.message());
  }

  // Cache the size since we need it after the move
  auto const size = body.size();

  // Respond to HEAD request
  if (req.method() == http::verb::head) {
    http::response<http::empty_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    return res;
  }

  // Respond to GET request
  http::response<http::file_body> res{
      std::piecewise_construct, std::make_tuple(std::move(body)),
      std::make_tuple(http::status::ok, req.version())};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, mime_type(path));
  res.content_length(size);
  res.keep_alive(req.keep_alive());
  return res;
}

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const *what) {
  if (ec.message().find("The socket was closed due to a timeout") !=
      beast::string_view::npos) {
    return;
  } else if (ec.message().find("Connection reset by peer") !=
             beast::string_view::npos) {
    return;
  }
  spdlog::error("{}: {}", what, ec.message());
}

// Handles an HTTP server connection
class session : public boost::asio::coroutine,
                public std::enable_shared_from_this<session> {
  beast::tcp_stream stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;
  bool keep_alive_ = true;
  std::shared_ptr<blog::Post> post;
  std::shared_ptr<blog::Page> page;

public:
  // Take ownership of the socket
  explicit session(tcp::socket &&socket,
                   std::shared_ptr<std::string const> const &doc_root,
                   std::shared_ptr<blog::Post> blogPost,
                   std::shared_ptr<blog::Page> blogPage)
      : stream_(std::move(socket)), doc_root_(doc_root), post(blogPost),
        page(blogPage) {}

  // Start the asynchronous operation
  void run() {
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session. Although not strictly necessary
    // for single-threaded contexts, this example code is written to be
    // thread-safe by default.
    net::dispatch(stream_.get_executor(),
                  beast::bind_front_handler(&session::loop, shared_from_this(),
                                            beast::error_code{}, 0));
  }

#include <boost/asio/yield.hpp>

  void loop(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);
    reenter(*this) {
      for (;;) {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        stream_.expires_after(std::chrono::seconds(30));

        // Read a request
        yield http::async_read(
            stream_, buffer_, req_,
            beast::bind_front_handler(&session::loop, shared_from_this()));

        if (ec == http::error::end_of_stream) {
          // The remote host closed the connection
          break;
        }
        if (ec)
          return fail(ec, "read");

        yield {
          // Handle request
          http::message_generator msg =
              handle_request(post, page, *doc_root_, std::move(req_));

          // Determine if we should close the connection
          keep_alive_ = msg.keep_alive();

          // Send the response
          beast::async_write(
              stream_, std::move(msg),
              beast::bind_front_handler(&session::loop, shared_from_this()));
        }

        if (ec)
          return fail(ec, "write");
        if (!keep_alive_) {
          // This means we should close the connection, usually because
          // the response indicated the "Connection: close" semantic.
          break;
        }
      }

      // Send a TCP shutdown
      stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

      // At this point the connection is closed gracefully
    }
  }

#include <boost/asio/unyield.hpp>
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public boost::asio::coroutine,
                 public std::enable_shared_from_this<listener> {
  net::io_context &ioc_;
  tcp::acceptor acceptor_;
  tcp::socket socket_;
  std::shared_ptr<std::string const> doc_root_;
  std::shared_ptr<blog::Post> post;
  std::shared_ptr<blog::Page> page;

public:
  listener(net::io_context &ioc, tcp::endpoint endpoint,
           std::shared_ptr<std::string const> const &doc_root,
           std::shared_ptr<blog::Post> blogPost,
           std::shared_ptr<blog::Page> blogPage)
      : ioc_(ioc), acceptor_(net::make_strand(ioc)),
        socket_(net::make_strand(ioc)), doc_root_(doc_root), post(blogPost),
        page(blogPage) {
    beast::error_code ec;

    // Open the acceptor
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
      fail(ec, "open");
      return;
    }

    // Allow address reuse
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
      fail(ec, "set_option");
      return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
      fail(ec, "bind");
      return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
      fail(ec, "listen");
      return;
    }
  }

  // Start accepting incoming connections
  void run() { loop(); }

private:
#include <boost/asio/yield.hpp>

  void loop(beast::error_code ec = {}) {
    reenter(*this) {
      for (;;) {
        yield acceptor_.async_accept(
            socket_,
            beast::bind_front_handler(&listener::loop, shared_from_this()));
        if (ec) {
          fail(ec, "accept");
        } else {
          // Create the session and run it
          std::make_shared<session>(std::move(socket_), doc_root_, post, page)
              ->run();
        }

        // Make sure each session gets its own strand
        socket_ = tcp::socket(net::make_strand(ioc_));
      }
    }
  }

#include <boost/asio/unyield.hpp>
};

#endif // CMS_HTTP_SERVER_HPP
