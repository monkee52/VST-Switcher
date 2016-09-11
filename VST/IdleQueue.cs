using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VST {
    static class IdleQueue {
        static private List<Tuple<Action, bool, int>> actions = new List<Tuple<Action, bool, int>>();

        static public void FireOnce(Action e) {
            IdleQueue.FireMultiple(e, 1);
        }

        static public void FireMultiple(Action e, int n) {
            IdleQueue.actions.Add(new Tuple<Action, bool, int>(e, false, n));
        }

        static public void FireForever(Action e) {
            IdleQueue.actions.Add(new Tuple<Action, bool, int>(e, true, 0));
        }

        static public void Process(object sender, EventArgs e) {
            List<Tuple<Action, bool, int>> newActions = new List<Tuple<Action, bool, int>>();

            for (int i = 0; i < IdleQueue.actions.Count; i++) {
                Tuple<Action, bool, int> action = IdleQueue.actions[i];

                if (action.Item2) {
                    action.Item1();

                    newActions.Add(action);
                } else {
                    action.Item1();

                    if (action.Item3 > 1) {
                        newActions.Add(new Tuple<Action, bool, int>(action.Item1, false, action.Item3 - 1));
                    }
                }
            }

            IdleQueue.actions = newActions;
        }
    }
}
