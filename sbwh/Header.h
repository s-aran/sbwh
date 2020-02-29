#ifndef __SBWH_HEADER_H__
#define __SBWH_HEADER_H__

#define _WIN32_WINNT 0x0601

#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/filesystem.hpp>
#include "../toml11/toml.hpp"
#include "../json/single_include/nlohmann/json.hpp"


class Payload
{
private:
  nlohmann::json json_;

protected:
  nlohmann::json& getJson();

public:
  const std::string get();

};


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

  const std::string getValue1() const;
  const std::string getValue2() const;
  const std::string getValue3() const;

  const std::string get();
};

class Configure
{
private:
  toml::table file_;

public:
  Configure() = default;
  Configure(const std::string& filepath);

  const std::string getTarget();
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

  void writeLog(const std::string& level, const std::string& message);

public:
  static void setLevel(LogLevel value);
  static LogLevel getLevel();

  void trace(const std::string& message);
  void info(const std::string& message);
  void warn(const std::string& message);
  void error(const std::string& message);
  void fatal(const std::string& message);
};

class SendByWebhook
{
private:
  Configure conf_;

public:
  SendByWebhook(Configure& conf);

  bool send(const Payload& payload);

};

#endif /* __SBWH_HEADER_H__ */
