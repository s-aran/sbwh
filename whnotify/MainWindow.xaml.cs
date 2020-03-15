using System.Windows;
using Windows.UI.Notifications;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Collections.Generic;
using System;
using System.IO;
using Microsoft.Win32;
using System.Windows.Input;
using System.Text.Json;

namespace whnotify
{
    public delegate void HttpServerCallback(HttpListenerContext context, HttpListenerRequest request, HttpListenerResponse response);

    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        HttpServer server_;
        Configure config_;

        public MainWindow()
        {
            InitializeComponent();


            config_ = new Configure();
            config_.Load("config.toml");
            IpAddressTextBox.Text = config_.IpAddress;
            PortTextBox.Text = config_.Port.ToString();
            LogFilepathLabel.Content = config_.LogFilepath;
            foreach (var item in config_.Languages)
            {
                LanguageComboBox.Items.Add(item);
            }
            LanguageComboBox.SelectedItem = config_.LanguageComboBoxSelectedIndex2Name(0);
        }

        private void PostCallback(HttpListenerContext context, HttpListenerRequest request, HttpListenerResponse response)
        {
            Console.WriteLine(request.HttpMethod);
            var body = new StreamReader(request.InputStream).ReadToEnd();
            var json = JsonSerializer.Deserialize<Dictionary<string, string>>(body);

            const string KeyName = "message";
            if (json.ContainsKey(KeyName))
            {
                Notify(json[KeyName]);
                response.StatusCode = 200;
                return;
            }

            response.StatusCode = 400;
        }

        private void Notify(string message)
        {
            var template = ToastNotificationManager.GetTemplateContent(ToastTemplateType.ToastText01);

            var textNodes = template.GetElementsByTagName("text");
            textNodes.Item(0).InnerText = message;

            var notifier = ToastNotificationManager.CreateToastNotifier("Webhook notifier");
            var notification = new ToastNotification(template);
            notifier.Show(notification);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Notify("This is test notification.");
        }

        private void PortTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            e.Handled = !Configure.IsPortNumeric(e.Text);
        }

        private void StartButton_Click(object sender, RoutedEventArgs e)
        {
            StartButton.IsEnabled = false;
            StopButton.IsEnabled = true;
            server_ = new HttpServer(config_.IpAddress, config_.Port);
            server_.AddCallback(PostCallback);
            server_.Start();
        }

        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            StartButton.IsEnabled = true;
            StopButton.IsEnabled = false;
            server_.Stop();
            server_ = null;
        }

        private void PortTextBox_PreviewLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!Configure.IsPortNumeric(PortTextBox.Text))
            {
                e.Handled = true;
                return;
            }

            config_.Port = int.Parse(PortTextBox.Text);
            Console.WriteLine("port=" + config_.Port.ToString());
        }

        private void IpAddressTextBox_PreviewLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            if (!Configure.IsIpAddress(IpAddressTextBox.Text))
            {
                e.Handled = true;
                return;
            }

            config_.IpAddress = IpAddressTextBox.Text;
            Console.WriteLine("ipaddress=" + config_.IpAddress.ToString());
        }

        private void SelectLogFilepathButton_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog
            {
                Filter = "Log files (*.log)|*.log|Text files (*.txt)|*.txt|All files (*.*)|*.*",
                RestoreDirectory = true
            };

            if (dialog.ShowDialog() == true)
            {
                LogFilepathLabel.Content = dialog.FileName;
                config_.LogFilepath = dialog.FileName;
            }
        }

        private void LanguageComboBox_PreviewLostKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            config_.Language = LanguageComboBox.Text;
        }

        private void Main_Closed(object sender, EventArgs e)
        {
            config_.Save("config.toml");
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            Close();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            MessageBox.Show(
                @"Webhook notifier ver0.1

Copyright 2020 Sumiishi Aran (s-aran)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the ""Software""), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Libraries:
* Microsoft.Toolkit.Uwp.Notifications
* Microsoft.Windows.Compatibility
* Microsoft.Windows.SDK.Contracts
* Nett
* System.Text.Json
", "About", MessageBoxButton.OK, MessageBoxImage.Information);
        }
    }

    public class Configure
    {
        public string IpAddress { get; set; } = "127.0.0.1";
        public int Port { get; set; } = 8080;
        public string LogFilepath { get; set; } = "";

        public string Language { get; set; } = "";
        public List<string> Languages { get; } = new List<string>() { "English" };

        public bool Load(string filepath)
        {
            if (!File.Exists(filepath))
            {
                return false;
            }

            var table = Nett.Toml.ReadFile(filepath);

            IpAddress = table["IpAddress"].ToString();
            string tmpPort = table["Port"].ToString();
            if (!IsPortNumeric(tmpPort))
            {
                MessageBox.Show("Port is not numeric.", null, MessageBoxButton.OK, MessageBoxImage.Error);
                return false;
            }
            int.TryParse(tmpPort, out int port);
            Port = port;

            return true;
        }

        public bool Save(string filepath)
        {
            var table = Nett.Toml.Create();
            Nett.TomlObjectFactory.Add(table, "IpAddress", IpAddress);
            Nett.TomlObjectFactory.Add(table, "Port", Port.ToString());
            Nett.Toml.WriteFile(table, filepath);
            return true;
        }

        public static bool IsPortNumeric(string portString)
        {
            int _;
            return int.TryParse(portString, out _);
        }

        public static bool IsIpAddress(string ipAddressString)
        {
            IPAddress _;
            return IPAddress.TryParse(ipAddressString, out _);
        }

        public string LanguageComboBoxSelectedIndex2Name(int index)
        {
            if (index > 0 && Languages.Count < index)
            {
                return Languages[0];
            }
            return Languages[index];
        }
    }

    public class Logger
    {
        private readonly FileStream file_;
        private readonly StreamWriter writer_;

        public enum Logging
        {
            Trace,
            Debug,
            Info,
            Warning,
            Warn = Warning,
            Error,
            Fatal,
        }

        private static Dictionary<Logging, string> loggingDict_ = new Dictionary<Logging, string>()
        {
            {Logging.Trace, "Trace"},
            {Logging.Debug,"Debug"},
            {Logging.Info, "Info"},
            {Logging.Warning, "Warn" },
            {Logging.Error, "ERROR" },
            {Logging.Fatal, "FATAL" },
        };

        public Logging Level { get; set; } = Logging.Trace;

        public Logger(string filepath)
        {
            file_ = new FileStream(filepath, FileMode.OpenOrCreate | FileMode.Append);
            writer_ = new StreamWriter(file_, Encoding.UTF8);
        }

        ~Logger()
        {
            writer_.Close();
            file_.Close();
        }

        private void Write(Logging level, string message)
        {
            if (level < Level)
            {
                return;
            }

            writer_.Write(string.Format("{0}: {1}", loggingDict_[level], message));
        }

        public void Trace(string message)
        {
            Write(Logging.Trace, message);
        }

        public void Debug(string message)
        {
            Write(Logging.Debug, message);
        }

        public void Info(string message)
        {
            Write(Logging.Info, message);
        }

        public void Warning(string message)
        {
            Write(Logging.Warning, message);
        }

        public void Error(string message)
        {
            Write(Logging.Error, message);
        }

        public void Fatal(string message)
        {
            Write(Logging.Fatal, message);
        }
    }


    public class HttpServer
    {
        private readonly int port_;
        private readonly string bindIp_;
        private readonly HttpListener listener_;

        private readonly List<HttpServerCallback> callbacks_;
        public HttpServer(string bindIp, int port=80)
        {
            port_ = port;
            bindIp_ = bindIp;

            callbacks_ = new List<HttpServerCallback>();
            listener_ = new HttpListener();
        }

        ~HttpServer()
        {
            listener_.Stop();
            listener_.Close();
        }

        public void AddCallback(HttpServerCallback callback)
        {
            callbacks_.Add(callback);
        }

        public async void Start()
        {
            await AsyncStart();
        }

        private Task AsyncStart()
        {
            string service = "";
            if (port_ == 80)
            {
                service = "http";
            }
            else if (port_ == 443)
            {
                service = "https";
            }

            string url;
            if (service == "")
            {
                url = @"http://" + bindIp_ + ":" + port_.ToString();
            }
            else
            {
                url = @"" + service + "://" + bindIp_;
            }

            url += "/";

            Console.WriteLine("Bind: " + url);

            listener_.Prefixes.Clear();
            listener_.Prefixes.Add(url);

            listener_.Start();


            return Task.Run(() =>
            {
                Console.WriteLine("taskLoop!");
                while (true)
                {
                    HttpListenerContext context = listener_.GetContext();
                    HttpListenerRequest request = context.Request;
                    HttpListenerResponse response = context.Response;

                    if (request == null)
                    {
                        response.StatusCode = 500;
                        continue;
                    }

                    foreach (var callback in callbacks_)
                    {
                        callback(context, request, response);
                    }

                    response.Close();
                }
            });
        }

        public void Stop()
        {
            listener_.Stop();
        }
    }
}
