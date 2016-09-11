using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Drawing;
using System.Windows.Forms;
using VST.Properties;
using VST.Audio;
using System.Diagnostics;

namespace VST {
    class VstTrayIcon : ApplicationContext {
        private NotifyIcon trayIcon = null;
        private Controller controller;

        public DialogResult Configure() {
            Configuration configDialog = new Configuration(this.controller);

            return configDialog.ShowDialog();
        }

        public VstTrayIcon() {
            this.controller = new Controller();

            if (Settings.Default.DefaultDeviceName.Trim().Length == 0 || Settings.Default.TargetDeviceName.Trim().Length == 0 || Settings.Default.ExecutablePath.Trim().Length == 0) {
                MessageBox.Show("This program has not been configured. Press OK to configure.", "VST Switcher Configuration", MessageBoxButtons.OK, MessageBoxIcon.Information);

                DialogResult result = this.Configure();

                if (result != DialogResult.OK) {
                    IdleQueue.FireOnce(() => this.Exit(null, null));

                    return;
                }
            }

            EventHandler configureEventHandler = new EventHandler((object sender, EventArgs e) => {
                this.Configure();
            });

            this.trayIcon = new NotifyIcon() {
                Icon = Resources.AppIcon,
                Visible = true,
                Text = "VST Switcher",
                ContextMenu = new ContextMenu(new MenuItem[] {
                    new MenuItem("Configure", configureEventHandler)
                })
            };

            Endpoint[] endpoints = this.controller.GetAudioEndpoints();
            Endpoint currentEndpoint = this.controller.GetDefaultAudioEndpoint();

            Endpoint defaultEndpoint = endpoints.First((Endpoint e) => {
                return e.Name == Settings.Default.DefaultDeviceName;
            });

            Endpoint targetEndpoint = endpoints.First((Endpoint e) => {
                return e.Name == Settings.Default.TargetDeviceName;
            });

            string executablePath = Settings.Default.ExecutablePath;

            if (targetEndpoint == defaultEndpoint) {
                MessageBox.Show("The target endpoint is the same as the default endpoint.", "VST Switcher", MessageBoxButtons.OK, MessageBoxIcon.Error);

                Settings.Default.TargetDeviceName = String.Empty;

                Settings.Default.Save();

                IdleQueue.FireOnce(() => this.Exit(null, null));

                return;
            }

            if (currentEndpoint != defaultEndpoint) {
                if (MessageBox.Show("The current endpoint is not the default endpoint. Continue?", "VST Switcher", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No) {
                    IdleQueue.FireOnce(() => this.Exit(null, null));

                    return;
                }
            }

            Transform transform = Transform.FromUserCode(Settings.Default.Transform);

            float volume = currentEndpoint.GetVolume();
            bool mute = currentEndpoint.GetMute();

            targetEndpoint.SetVolume(transform.Forward(volume));
            targetEndpoint.SetMute(mute);

            this.controller.SetDefaultAudioEndpoint(targetEndpoint);

            currentEndpoint.SetVolume(1.0f);
            currentEndpoint.SetMute(false);

            Action<object, EventArgs> restoreDevice = (object sender, EventArgs e) => {
                volume = targetEndpoint.GetVolume();
                mute = targetEndpoint.GetMute();

                currentEndpoint.SetVolume(transform.Inverse(volume));
                currentEndpoint.SetMute(mute);

                this.controller.SetDefaultAudioEndpoint(currentEndpoint);
            };

            // Launch program
            Process proc = new Process();

            try {
                proc.StartInfo.FileName = executablePath;
                proc.StartInfo.UseShellExecute = false;
                proc.EnableRaisingEvents = true;

                proc.Exited += new EventHandler((object sender, EventArgs e) => {
                    restoreDevice(sender, e);
                    this.Exit(null, null);
                });

                proc.Start();
            } catch (Exception e) {
                restoreDevice(null, null);

                IdleQueue.FireOnce(() => this.Exit(null, null));

                throw e;
            }
        }

        void Exit(object sender, EventArgs e) {
            if (trayIcon != null) {
                trayIcon.Visible = false;
            }

            Application.Exit();
        }
    }
}
