using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace VST {
    static class Program {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() {
            bool firstInstance;

            using (Mutex mutex = new Mutex(true, "Global\\VST", out firstInstance)) {
                if (!firstInstance) {
                    MessageBox.Show("Another instance is already running.", "VST");

                    return;
                }

                Application.EnableVisualStyles();
                Application.SetCompatibleTextRenderingDefault(false);

                Application.Idle += new EventHandler(IdleQueue.Process);

                Application.Run(new VstTrayIcon());
            }
        }
    }
}
