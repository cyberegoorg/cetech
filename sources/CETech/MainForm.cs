using System;
using System.Windows.Forms;
using CELib;

namespace CETech
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            var file_log = new LogHandler.FileLog("log.md");

            Log.LogEvent += LogHandler.console_log;
            Log.LogEvent += file_log.Log;
            Log.LogEvent += ListLog;

            Log.Info("MainForm", "Info");
            Log.Error("MainForm", "Error");
            Log.Warning("MainForm", "Warning");
            Log.Debug("MainForm", "Debug");
        }

        private void ListLog(Log.Level level, DateTime time, int workerId, string where, string msg,
            params object[] args)
        {
            listBox1.Items.Add(string.Format(LogHandler.LogFormat, level, where, time, workerId,
                string.Format(msg, args)));
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Log.Info("MainForm", sender.ToString());
        }
    }
}