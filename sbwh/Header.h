#ifndef __SBWH_HEADER_H__
#define __SBWH_HEADER_H__

#include <vector>
#include <map>
#include <memory>

#define _WIN32_WINNT 0x0601

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include "../toml11/toml.hpp"
#include "../json/single_include/nlohmann/json.hpp"
#include <mstch/mstch.hpp>

#pragma comment(lib, "mstch")


// payload to send by POST
#include "../toml11/toml.hpp"
#include "../json/single_include/nlohmann/json.hpp"


class Payload
{
private:
  nlohmann::json json_;

protected:
  nlohmann::json& getJson();

public:
  static Payload createFromString(const std::string& value);

  const std::string get();

};


// IFTTT payload to send by POST
class IftttPayload : public Payload
{
private:
  bool setValue1_ = false;
  bool setValue2_ = false;
  bool setValue3_ = false;

  std::string value1_;
  std::string value2_;
  std::string value3_;
  
public:

  void setValue1(const std::string& value);
  void setValue2(const std::string& value);
  void setValue3(const std::string& value);

  const std::string getValue1() const { return this->value1_; }
  const std::string getValue2() const { return this->value2_; }
  const std::string getValue3() const { return this->value3_; }

  const std::string get();
};

// toml table
class Section
{
public:
  using MustacheMap = std::map<const std::string, const std::string>;

private:
  std::string name_ = "";
  std::string url_ = "";
  int port_ = 0;
  std::string payload_ = "";
  MustacheMap mustache_ = {};

public:
  Section() = default;
  Section(const std::string& name, const std::string& url, const int port, const std::string& payload, const MustacheMap& mustache);

  const std::string getName() const { return this->name_; }
  const std::string getUrl() const { return this->url_; }
  const int getPort() const { return this->port_; }
  const std::string getPayload() const { return this->payload_; }
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

struct Utilities
{
  struct Destination
  {
    std::string protocol;
    std::string host;
    std::string target;
  };

  static const Destination getDestinationFromUrl(const std::string& url);
};

class SendByWebhook
{
private:
  Utilities::Destination dest_;

public:
  SendByWebhook(const std::string& url);

  bool send(const Payload& payload);

};

#endif /* __SBWH_HEADER_H__ */
