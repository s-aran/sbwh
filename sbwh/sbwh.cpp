// sbwh.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "Header.h"

int main()
{
    std::cout << "Hello World!\n";

    std::string filePath = ".sbwhrc.toml";
    Configure conf(filePath);
    auto section = conf.getSection("ifttt");

    SendByWebhook sbwh("https://maker.ifttt.com//trigger/event/with/key/xxxx");

    IftttPayload payload;
    sbwh.send(payload);
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します


Logger::LogLevel Logger::level_ = Logger::LogLevel::Info;

void Logger::setLevel(Logger::LogLevel level)
{
  Logger::level_ = level;
}

Logger::LogLevel Logger::getLevel()
{
  return Logger::level_;
}

std::string Logger::getLevelStringFromLogLevel(Logger::LogLevel level)
{
  switch (level)
  {
  case Logger::LogLevel::Trace:
    return "Trace";
  case Logger::LogLevel::Info:
    return "Info";
  case Logger::LogLevel::Warn:
    return "Warn";
  case Logger::LogLevel::Error:
    return "ERROR";
  case Logger::LogLevel::Fatal:
    return "FATAL";
  default:
    return "???";
  }
}

void Logger::writeLog(Logger::LogLevel level, const std::string& message)
{
  Logger::writeLog(level, boost::format("%s") % message);
}

void Logger::writeLog(Logger::LogLevel level, const boost::format& format)
{
  if (Logger::getLevel() <= level)
  {
    std::cout << Logger::getLevelStringFromLogLevel(level) << ": " << format << std::endl;
  }
}

nlohmann::json& Payload::getJson()
{
  return this->json_;
}

Payload Payload::createFromString(const std::string& value)
{
  Payload result;

  nlohmann::json& json = result.getJson();


  return Payload();
}

const std::string Payload::get()
{
  static constexpr int indent = 2;
  static constexpr char indentChar = ' ';
  static constexpr bool ensureAscii = false;

  return this->json_.dump(indent, indentChar, ensureAscii);
}

void IftttPayload::setValue1(const std::string& value)
{
  this->setValue1_ = true;
  this->value1_ = value;
}

void IftttPayload::setValue2(const std::string& value)
{
  this->setValue2_ = true;
  this->value2_ = value;
}

void IftttPayload::setValue3(const std::string& value)
{
  this->setValue3_ = true;
  this->value3_ = value;
}

const std::string IftttPayload::getValue1() const
{
  return this->value1_;
}

const std::string IftttPayload::getValue2() const
{
  return this->value2_;
}

const std::string IftttPayload::getValue3() const
{
  return this->value3_;
}

const std::string IftttPayload::get()
{
  nlohmann::json& json = this->getJson();


  if (this->setValue1_)
  {
    json["value1"] = this->getValue1();
  }

  if (this->setValue2_)
  {
    json["value2"] = this->getValue2();
  }

  if (this->setValue3_)
  {
    json["value3"] = this->getValue3();
  }

  return Payload::get();
}

Section::Section(const std::string& name, const std::string& url, const int port, const std::string& payload, const Section::MustacheMap& mustache):
  name_(name), url_(url), port_(port), payload_(payload), mustache_(mustache)
{
  // NOP
}

Configure::Configure(const std::string& filePath)
{
  namespace fs = boost::filesystem;
  
  if (!fs::exists(filePath))
  {
    Logger::fatal(boost::format("%1% not found.") % (filePath.length() <=0 ? "configure file" : filePath));
    throw std::exception();
  }

  this->table_ = toml::parse(filePath);
}

Section Configure::getSection(const std::string& name) const
{
  const auto& section = toml::find(this->table_, name);
  const auto& url = toml::find<std::string>(section, "url");
  Logger::info(boost::format("%1%: %2%\n") % name % url.c_str());
  return Section();
}

SendByWebhook::SendByWebhook(const std::string& url)
{
  this->dest_ = Utilities::getDestinationFromUrl(url);
}

bool SendByWebhook::send(const Payload& payload)
{
  namespace beast = boost::beast;
  namespace http = beast::http;
  namespace net = boost::asio;
  using tcp = net::ip::tcp;


  net::io_context context;
  tcp::resolver resolver(context);
  beast::tcp_stream stream(context);

  const auto service = this->dest_.protocol;
  const auto host = this->dest_.host;
  const auto target = this->dest_.target;
  
  Logger::info(boost::format("service: %s") % service);
  Logger::info(boost::format("host: %s") % host);
  Logger::info(boost::format("target: %s") % target);

  auto const results = resolver.resolve(host, service);

  stream.connect(results);

  http::request<http::string_body> request{ http::verb::post, target, 11 };
  // http::request<http::string_body> request;
  // request.version(11);
  request.set(http::field::host, host);
  // request.target(target);
  request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  request.set(http::field::content_type, "application/json");
  request.keep_alive(true);
  // request.method(http::verb::post);

  request.body() = R"({"value1": "aaaaaa"})";
  request.prepare_payload();

  http::write(stream, request);

  beast::flat_buffer buffer;
  http::response<http::dynamic_body> response;
  http::read(stream, buffer, response);

  std::cout << response << std::endl;

  return true;
}

const Utilities::Destination Utilities::getDestinationFromUrl(const std::string& url)
{
  static const boost::regex re = boost::regex(R"(^(.+)://([^:]+?)(:([0-9]+))?/(.*)$)");
  boost::smatch match;

  auto result = Utilities::Destination();
  if (boost::regex_search(url, match, re))
  {
    result.protocol = match[1];
    result.host = match[2];
    result.target = match[5];
  }

  return std::move(result);
}
