using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using VST.Audio;
using VST.Properties;

namespace VST {
    public partial class Configuration : Form {
        public Configuration(Controller controller) {
            InitializeComponent();

            Device[] endpoints = controller.GetAudioDevices(DeviceType.Render, DeviceState.Active);

            this.comboDefaultDevice.DataSource = endpoints.Clone();
            this.comboTargetDevice.DataSource = endpoints.Clone();

            if (Settings.Default.DefaultDeviceName.Trim().Length == 0) {
                this.comboDefaultDevice.SelectedItem = controller.GetDefaultAudioDevice(DeviceType.Render, DeviceRole.Multimedia);
            } else {
                this.comboDefaultDevice.SelectedItem = endpoints.First((Device e) => {
                    return e.FriendlyName == Settings.Default.DefaultDeviceName;
                });
            }

            if (Settings.Default.TargetDeviceName.Trim().Length != 0) {
                this.comboTargetDevice.SelectedItem = endpoints.First((Device e) => {
                    return e.FriendlyName == Settings.Default.TargetDeviceName;
                });
            }

            this.txtExecutablePath.Text = Settings.Default.ExecutablePath;
            this.txtTransform.Text = Settings.Default.Transform;
        }

        private void btnBrowse_Click(object sender, EventArgs e) {
            if (dlgExecutablePath.ShowDialog() == DialogResult.OK) {
                this.txtExecutablePath.Text = dlgExecutablePath.FileName;
            }
        }

        private bool HasUnsavedChanges() {
            return (
                Settings.Default.DefaultDeviceName != ((Device)this.comboDefaultDevice.SelectedItem).FriendlyName
                || Settings.Default.TargetDeviceName != ((Device)this.comboTargetDevice.SelectedItem).FriendlyName
                || Settings.Default.ExecutablePath != this.txtExecutablePath.Text
                || Settings.Default.Transform != this.txtTransform.Text
            );
        }

        private void Save() {
            Settings.Default.DefaultDeviceName = ((Device)this.comboDefaultDevice.SelectedItem).FriendlyName;
            Settings.Default.TargetDeviceName = ((Device)this.comboTargetDevice.SelectedItem).FriendlyName;
            Settings.Default.ExecutablePath = this.txtExecutablePath.Text;
            Settings.Default.Transform = this.txtTransform.Text;

            Settings.Default.Save();
        }

        private void btnSave_Click(object sender, EventArgs e) {
            this.Save();
            this.Close();
        }

        private void Configuration_FormClosing(object sender, FormClosingEventArgs e) {
            if (this.HasUnsavedChanges()) {
                DialogResult result = MessageBox.Show("There are unsaved changes, do you want to save?", this.Text, MessageBoxButtons.YesNoCancel);

                if (result == DialogResult.Yes) {
                    this.Save();
                } else if (result == DialogResult.Cancel) {
                    e.Cancel = true;
                }
            }
        }
    }
}
