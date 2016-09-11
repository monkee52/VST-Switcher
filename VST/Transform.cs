using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VST {
    static class Transform {
        static public float Forward(float x) {
            return (float)((5.0d * Math.Log10(x) + 10.0d) / 13.0d);
        }

        static public float Inverse(float x) {
            return (float)Math.Pow(10, 13.0d * x / 5.0d - 2.0d);
        }
    }
}
