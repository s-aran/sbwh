// sbwh.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "Header.h"

int main()
{
    std::cout << "Hello World!\n";

    Configure conf("");
    SendByWebhook sbwh(conf);

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

void Logger::writeLog(const std::string& level, const std::string& message)
{
  std::cout << level << ": " << message << std::endl;
}

void Logger::trace(const std::string& message)
{
  if (this->getLevel() >= Logger::LogLevel::Trace)
  {
    this->writeLog("Trace", message);
  }
}

void Logger::info(const std::string& message)
{
  if (this->getLevel() >= Logger::LogLevel::Info)
  {
    this->writeLog("Info", message);
  }
}

void Logger::warn(const std::string& message)
{
  if (this->getLevel() >= Logger::LogLevel::Warn)
  {
    this->writeLog("Warn", message);
  }
}

void Logger::error(const std::string& message)
{
  if (this->getLevel() >= Logger::LogLevel::Error)
  {
    this->writeLog("ERROR", message);
  }
}

void Logger::fatal(const std::string& message)
{
  if (this->getLevel() >= Logger::LogLevel::Fatal)
  {
    this->writeLog("FATAL", message);
  }
}

nlohmann::json& Payload::getJson()
{
  return this->json_;
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

Configure::Configure(const std::string& filePath)
{

}

SendByWebhook::SendByWebhook(Configure& conf)
{

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

  const char* const host = R"(maker.ifttt.com)";
  const char* const port = "80";
  const char* const target = R"(/trigger/event/with/key/xxxx)";
  auto const results = resolver.resolve(host, port);

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

