using System;
using System.Drawing;
using System.Threading;
using System.Windows.Forms;
using SharpBgfx;

namespace Common {
    public class Sample {
        EventQueue eventQueue = new EventQueue();
        Form form;
        Thread thread;

        public int WindowWidth {
            get;
            private set;
        }

        public int WindowHeight {
            get;
            private set;
        }

        public Sample (string name, int windowWidth, int windowHeight) {
            WindowWidth = windowWidth;
            WindowHeight = windowHeight;

            form = new Form {
                Text = name,
                ClientSize = new Size(windowWidth, windowHeight)
            };

            form.ClientSizeChanged += (o, e) => eventQueue.Post(new SizeEvent(windowWidth, windowHeight));
            form.FormClosing += OnFormClosing;
            form.FormClosed += (o, e) => eventQueue.Post(new Event(EventType.Exit));

            Bgfx.SetWindowHandle(form.Handle);
        }

        public void Run (Action<Sample> renderThread) {
            thread = new Thread(() => renderThread(this));
            thread.Start();

            Application.Run(form);
        }

        public bool ProcessEvents (ResetFlags resetFlags) {
            Event ev;
            bool resizeRequired = false;

            while ((ev = eventQueue.Poll()) != null) {
                switch (ev.Type) {
                    case EventType.Exit:
                        return false;

                    case EventType.Size:
                        var size = (SizeEvent)ev;
                        WindowWidth = size.Width;
                        WindowHeight = size.Height;
                        resizeRequired = true;
                        break;
                }
            }

            if (resizeRequired)
                Bgfx.Reset(WindowWidth, WindowHeight, resetFlags);

            return true;
        }

        void OnFormClosing (object sender, FormClosingEventArgs e) {
            // kill all rendering and shutdown before closing the
            // window, or we'll get errors from the graphics driver
            eventQueue.Post(new Event(EventType.Exit));
            thread.Join();
        }
    }
}
