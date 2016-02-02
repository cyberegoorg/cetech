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

            LogHandler.FileLog file_log = new LogHandler.FileLog("log.md");

            Log.LogEvent += LogHandler.console_log;
            Log.LogEvent += file_log.Log;


            Log.Info("dddd", "dsasadsadsad");
            Log.Error("dddd", "dsasadsadsad");
            Log.Warning("dddd", "dsasadsadsad");
            Log.Debug("dddd", "dsasadsadsad");
        }
    }
}
