using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.CodeDom.Compiler;
using System.Reflection;
using Microsoft.JScript;
using System.Windows.Forms;

namespace VST {
    public class Transform {
        public Func<float, float> Forward;
        public Func<float, float> Inverse;

        static public Transform FromUserCode(string code) {
            CodeDomProvider compiler = CodeDomProvider.CreateProvider("JScript");

            CompilerParameters options = new CompilerParameters() {
                GenerateInMemory = false
            };

            string newCode = "package VST { class TransformCode { public static function GetUserTransformCode() : Object {\n" + code + "\n return { \"forward\": forward, \"inverse\": inverse }; } } }";

            CompilerResults results = compiler.CompileAssemblyFromSource(options, newCode);

            if (results.Errors.Count > 0) {
                foreach (CompilerError error in results.Errors) {
                    MessageBox.Show(error.ToString(), "Compile error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                }

                return new Transform() {
                    Forward = (float x) => x,
                    Inverse = (float x) => x
                };
            }

            Assembly userAssembly = results.CompiledAssembly;
            Type userType = userAssembly.GetType("VST.TransformCode");

            object instance = Activator.CreateInstance(userType);
            
            JSObject userTransformCode = (JSObject)userType.InvokeMember("GetUserTransformCode", BindingFlags.InvokeMethod, null, instance, null);

            return new Transform() {
                Forward = (float x) => (float)(double)((ScriptFunction)userTransformCode["forward"]).Invoke(null, (double)x),
                Inverse = (float x) => (float)(double)((ScriptFunction)userTransformCode["inverse"]).Invoke(null, (double)x)
            };
        }
    }
}
