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

            Endpoint[] endpoints = controller.GetAudioEndpoints();

            this.comboDefaultDevice.DataSource = endpoints.Clone();
            this.comboTargetDevice.DataSource = endpoints.Clone();

            if (Settings.Default.DefaultDeviceName.Trim().Length == 0) {
                this.comboDefaultDevice.SelectedItem = controller.GetDefaultAudioEndpoint();
            } else {
                this.comboDefaultDevice.SelectedItem = endpoints.First((Endpoint e) => {
                    return e.Name == Settings.Default.DefaultDeviceName;
                });
            }

            if (Settings.Default.TargetDeviceName.Trim().Length != 0) {
                this.comboTargetDevice.SelectedItem = endpoints.First((Endpoint e) => {
                    return e.Name == Settings.Default.TargetDeviceName;
                });
            }

            this.txtExecutablePath.Text = Settings.Default.ExecutablePath;
        }

        private void btnBrowse_Click(object sender, EventArgs e) {
            if (dlgExecutablePath.ShowDialog() == DialogResult.OK) {
                this.txtExecutablePath.Text = dlgExecutablePath.FileName;
            }
        }

        private bool HasUnsavedChanges() {
            return (
                Settings.Default.DefaultDeviceName != ((Endpoint)comboDefaultDevice.SelectedItem).Name
                || Settings.Default.TargetDeviceName != ((Endpoint)comboTargetDevice.SelectedItem).Name
                || Settings.Default.ExecutablePath != txtExecutablePath.Text
            );
        }

        private void Save() {
            Settings.Default.DefaultDeviceName = ((Endpoint)comboDefaultDevice.SelectedItem).Name;
            Settings.Default.TargetDeviceName = ((Endpoint)comboTargetDevice.SelectedItem).Name;
            Settings.Default.ExecutablePath = txtExecutablePath.Text;

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
