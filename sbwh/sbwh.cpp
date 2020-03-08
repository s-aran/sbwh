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

// sbwh.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include "Header.h"

int main(int argc, char* argv[])
{
  if (argc == 2)
  {
    if (argv[1][0] == '-')
    {
      switch (argv[1][1])
      {
      case 'V':
        std::cout << "sbwh" << " " << Version::getVersion() << std::endl;
        return EXIT_SUCCESS;
      case 'l':
        std::cout << R"(See LICENSE file at github URL for license.)" << std::endl;
        for (const auto& itr : Utilities::getLibraries())
        {
          std::cout << "* " << itr.first << ": " << itr.second << std::endl;
        }
        return EXIT_SUCCESS;
      case 'h':
        [[fallthrough]];
      default:
        std::cout << argv[0] << " " << "[-Vhl] <name>" << std::endl;
        return EXIT_SUCCESS;
      }
    }
    else
    {
      try
      {
        std::string filePath = ".sbwhrc.toml";
        Configure conf(filePath);
        auto section = conf.getSection(argv[1]);

        auto payload = Payload(section.getPayload(), section.getMustache());

        SendByWebhook sbwh(section.getUrl());

        sbwh.send(payload);
      }
      catch (std::exception)
      {
        return EXIT_FAILURE;
      }

      return EXIT_SUCCESS;
    }
  }
  return EXIT_FAILURE;
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


Logger::LogLevel Logger::level_ = Logger::LogLevel::Trace;

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
  case Logger::LogLevel::Trace: return "Trace";
  case Logger::LogLevel::Info: return "Info";
  case Logger::LogLevel::Warn: return "Warn";
  case Logger::LogLevel::Error: return "ERROR";
  case Logger::LogLevel::Fatal: return "FATAL";
  default: return "???";
  }
}

void Logger::writeLog(Logger::LogLevel level, const std::string& message)
{
  Logger::writeLog(level, boost::format(message));
}

void Logger::writeLog(Logger::LogLevel level, const boost::format& format)
{
  if (Logger::getLevel() <= level)
  {
    std::cout << Logger::getLevelStringFromLogLevel(level) << ": " << format << std::endl;
  }
}

Payload::Payload(const Section::PayloadMap payload, const Section::MustacheMap mustache)
{
  mstch::map context;
  for (const auto& mitr : mustache)
  {
    context.emplace(mitr.first, mitr.second);
  }

  for (const auto& pitr : payload)
  {
    Logger::info(boost::format("processing %1%") % pitr.first);
    auto rendered = mstch::render(pitr.second, context);
    this->json_.emplace(pitr.first, rendered);
  }
}


nlohmann::json& Payload::getJson()
{
  return this->json_;
}

const std::string Payload::str() const
{
  static constexpr int indent = 2;
  static constexpr char indentChar = ' ';
  static constexpr bool ensureAscii = false;

  auto result = this->json_.dump(indent, indentChar, ensureAscii);
  return std::move(result);
}

Section::Section(const std::string& name, const std::string& url, const int port, const Section::PayloadMap& payload, const Section::MustacheMap& mustache):
  name_(name), url_(url), port_(port), payload_(payload), mustache_(mustache)
{
  // NOP
}

const std::map<const std::string, const std::string> Section::table2map(const toml::value& table)
{
  if (!table.is_table())
  {
    Logger::fatal("configure file parsing error.");
    throw std::exception();
  }

  auto result = std::map<const std::string, const std::string>();

  for (const auto& kv: table.as_table())
  {
    result.emplace(kv.first, kv.second.as_string());
  }

  return std::move(result);
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

  const auto payload = Section::table2payloadMap(toml::find(section, "payload"));
  const auto mustache = Section::table2mustacheMap(toml::find(section, "mustache"));

  auto result = Section(name, url, 0, payload, mustache);
  return std::move(result);
}

SendByWebhook::SendByWebhook(const std::string& url)
{
  this->dest_ = Utilities::getDestinationFromUrl(url);
}

void SendByWebhook::load_root_certificates(boost::asio::ssl::context& context)
{
  boost::system::error_code ec;

#if defined (_WIN32)
  PCCERT_CONTEXT pcert_context = nullptr;
  LPCTSTR pzstore_name = TEXT("ROOT");

  // Try to open root certificate store
  // If it succeeds, it'll return a handle to the certificate store
  // If it fails, it'll return NULL
  auto hstore_handle = CertOpenSystemStore(NULL, pzstore_name);
  char* data = nullptr;
  std::string certificates;
  X509* x509 = nullptr;
  BIO* bio = nullptr;
  if (hstore_handle != nullptr)
  {
    // Extract the certificates from the store in a loop
    while ((pcert_context = CertEnumCertificatesInStore(hstore_handle, pcert_context)) != NULL)
    {
      x509 = d2i_X509(nullptr, const_cast<const BYTE**>(&pcert_context->pbCertEncoded), pcert_context->cbCertEncoded);
      bio = BIO_new(BIO_s_mem());
      if (PEM_write_bio_X509(bio, x509))
      {
        auto len = BIO_get_mem_data(bio, &data);
        if (certificates.size() == 0)
        {
          certificates = { data, static_cast<std::size_t>(len) };
          context.add_certificate_authority(boost::asio::buffer(certificates.data(), certificates.size()), ec);
          if (ec)
          {
            BIO_free(bio);
            X509_free(x509);
            CertCloseStore(hstore_handle, 0);
            return;
          }
        }
        else
        {
          certificates.append(data, static_cast<std::size_t>(len));
          context.add_certificate_authority(boost::asio::buffer(certificates.data(), certificates.size()), ec);
          if (ec)
          {
            BIO_free(bio);
            X509_free(x509);
            CertCloseStore(hstore_handle, 0);
            return;
          }
        }
      }
      BIO_free(bio);
      X509_free(x509);
    }
    CertCloseStore(hstore_handle, 0);
  }
  const std::string certs{ certificates };
#elif defined (__linux__)
  
  const auto filename = R"(/etc/ssl/certs/ca-bundle.crt)";
  std::ifstream ifs(filename);
  std::string content{std::istreambuf_iterator<char>{ifs}, {}};
  context.add_certificate_authority(boost::asio::buffer(content.data(), content.size()), ec);

#endif /* _WIN32 */


  if (ec)
  {
    throw boost::system::system_error{ ec };
  }
}

bool SendByWebhook::send(const Payload& payload)
{
  namespace beast = boost::beast;
  namespace http = beast::http;
  namespace net = boost::asio;
  namespace ssl = net::ssl;
  using tcp = net::ip::tcp;

  net::io_context context;

  const auto service = this->dest_.protocol;
  const auto host = this->dest_.host;
  const auto target = this->dest_.target;
  
  Logger::info(boost::format("service: %s") % service);
  Logger::info(boost::format("host: %s") % host);
  Logger::info(boost::format("target: %s") % target);

  if (service == "https")
  {
    try
    {
      ssl::context ssl_context(ssl::context::tlsv12_client);
      load_root_certificates(ssl_context);
      ssl_context.set_verify_mode(ssl::verify_peer);

      tcp::resolver resolver(context);
      beast::ssl_stream<beast::tcp_stream> stream(context, ssl_context);

      if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
      {
        beast::error_code ec{ static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() };
        throw beast::system_error{ ec };
      }

      auto const results = resolver.resolve(host, service);

      beast::get_lowest_layer(stream).connect(results);
      stream.handshake(ssl::stream_base::client);

      http::request<http::string_body> request{ http::verb::post, target, 11 };
      // http::request<http::string_body> request;
      // request.version(11);
      request.set(http::field::host, host);
      // request.target(target);
      request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      request.set(http::field::content_type, "application/json");
      // request.keep_alive(true);
      // request.method(http::verb::post);

      request.body() = payload.str();
      request.prepare_payload();

      Logger::info(boost::format("ssl payload: %s") % payload.str());

      http::write(stream, request);

      beast::flat_buffer buffer;
      http::response<http::dynamic_body> response;
      http::read(stream, buffer, response);

      beast::error_code ec;
      stream.shutdown(ec);
      if (ec == net::error::eof)
      {
        ec = {};
      }
      if (ec)
      {
        throw beast::system_error{ ec };
      }
    }
    catch (std::exception const& e)
    {
      Logger::error(boost::format("%1%") % e.what());
    }
  }
  else
  {
    beast::tcp_stream stream(context);
    tcp::resolver resolver(context);
    auto const results = resolver.resolve(host, service);
    stream.connect(results);

    http::request<http::string_body> request{ http::verb::post, target, 11 };
    // http::request<http::string_body> request;
    // request.version(11);
    request.set(http::field::host, host);
    // request.target(target);
    request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request.set(http::field::content_type, "application/json");
    // request.keep_alive(true);
    // request.method(http::verb::post);

    request.body() = payload.str();
    request.prepare_payload();

    Logger::info(boost::format("payload: %s") % payload.str());

    http::write(stream, request);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> response;
    http::read(stream, buffer, response);
  }

  // std::cout << response << std::endl;

  return true;
}

const Utilities::Destination Utilities::getDestinationFromUrl(const std::string& url)
{
  static const boost::regex re = boost::regex(R"(^(.+)://([^:]+?)(:([0-9]+))?(/.*)$)");
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

const std::map<const std::string_view, const std::string_view> Utilities::getLibraries()
{
  const auto result = std::map<const std::string_view, const std::string_view>({
    {"boost", "https://www.boost.org/"},
    {"nlohmann/json", "https://github.com/nlohmann/json"},
    {"ToruNiina/toml11 ", "https://github.com/ToruNiina/toml11"},
    {"no1msd/mstch ", "https://github.com/no1msd/mstch"},
    });
  return result;
}
