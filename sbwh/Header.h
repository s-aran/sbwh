/*
Copyright 2020 Sumiishi Aran (s-aran)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef __SBWH_HEADER_H__
#define __SBWH_HEADER_H__

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <iterator>
#include <fstream>

#define _WIN32_WINNT 0x0601

#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>

#include "../toml11/toml.hpp"
#include "../json/single_include/nlohmann/json.hpp"
#include <mstch/mstch.hpp>

#if defined ( _WIN32)
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32")
#elif defined (__linux__)

#endif /* _WINNT */

#ifdef _DEBUG
#pragma comment(lib, "mstchd")
#pragma comment(lib, "libcrypto64MTd")
#pragma comment(lib, "libssl64MTd")
#else
#pragma comment(lib, "mstch")
#pragma comment(lib, "libcrypto64MT")
#pragma comment(lib, "libssl64MT")
#endif /* _DEBUG */



class Logger
{
public:
  enum class LogLevel: int
  {
    Trace = 0,
    Info,
    Warn,
    Warning = Warn,
    Error,
    Fatal,
  };

private:
  static LogLevel level_;

  static std::string getLevelStringFromLogLevel(LogLevel level);
  static void writeLog(LogLevel level, const std::string& message);
  static void writeLog(LogLevel level, const boost::format& format);

public:
  static void setLevel(LogLevel value);
  static LogLevel getLevel();

  static void trace(const std::string& message) { Logger::writeLog(LogLevel::Trace, message); }
  static void trace(const boost::format& format) { Logger::writeLog(LogLevel::Trace, format); }
  static void info(const std::string& message)  { Logger::writeLog(LogLevel::Info, message); }
  static void info(const boost::format& format)  { Logger::writeLog(LogLevel::Info, format); }
  static void warn(const std::string& message)  { Logger::writeLog(LogLevel::Warn, message); }
  static void warn(const boost::format& format)  { Logger::writeLog(LogLevel::Warn, format); }
  static void error(const std::string& message) { Logger::writeLog(LogLevel::Error, message); }
  static void error(const boost::format& format) { Logger::writeLog(LogLevel::Error, format); }
  static void fatal(const std::string& message) { Logger::writeLog(LogLevel::Fatal, message); }
  static void fatal(const boost::format& format) { Logger::writeLog(LogLevel::Fatal, format); }
};

struct Version
{
  static constexpr int Major = 0;
  static constexpr int Minor = 991;
  static constexpr char const* Status = "Beta 3";

  static const std::string getVersion() { return (boost::format("%d.%02d %s") % Major % Minor % Status).str(); }
};

struct Utilities
{
  struct Destination
  {
    std::string protocol = "";
    std::string host = "";
    int port = 0;
    std::string target = "";
  };

  static const Destination getDestinationFromUrl(const std::string& url);

  static const std::map<const std::string_view, const std::string_view> getLibraries();
};


// toml table
class Section
{
public:
  using PayloadMap = std::map<const std::string, const std::string>;
  using MustacheMap = std::map<const std::string, const std::string>;

private:
  std::string name_ = "";
  std::string url_ = "";
  int port_ = 0;
  PayloadMap payload_ = {};
  MustacheMap mustache_ = {};

  static const std::map<const std::string, const std::string> table2map(const toml::value& table);

public:
  Section() = default;
  Section(const std::string& name, const std::string& url, const int port, const PayloadMap& payload, const MustacheMap& mustache);

  static PayloadMap table2payloadMap(const toml::value& table) { return Section::table2map(table); }
  static MustacheMap table2mustacheMap(const toml::value& table) { return Section::table2map(table); }

  const std::string getName() const { return this->name_; }
  const std::string getUrl() const { return this->url_; }
  const int getPort() const { return this->port_; }
  const PayloadMap getPayload() const { return this->payload_; }
  const MustacheMap getMustache() const { return this->mustache_; }
};


// toml
class Configure
{
private:
  toml::value table_;

public:
  Configure() = default;
  explicit Configure(const std::string& filepath);

  Section getSection(const std::string& name) const;
};


// payload to send by POST
class Payload
{
private:
  nlohmann::json json_;

protected:
  nlohmann::json& getJson();

public:
  Payload(const Section::PayloadMap payload, const Section::MustacheMap mustache);

  const std::string str() const;

};


class SendByWebhook
{
private:
  Utilities::Destination dest_;

  void load_root_certificates(boost::asio::ssl::context& context);

public:
  SendByWebhook(const std::string& url);

  bool send(const Payload& payload);

};

#endif /* __SBWH_HEADER_H__ */
