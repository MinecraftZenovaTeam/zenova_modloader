using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace ZenovaWPF
{
    public enum PACKAGE_EXECUTION_STATE
    {
        PES_UNKNOWN = 0, // The app is likely not running
        PES_RUNNING = 1, // The app is running
        PES_SUSPENDING = 2, // The app is being suspended
        PES_SUSPENDED = 3, // The app is suspended
        PES_TERMINATED = 4, // The app is terminated
        PES_ERROR = 5 // There was an error in getting the execution state
    }

    class StateChangeCallback
    {
        private MainWindow window;
        private delegate void Callback(int state);
        private Callback mInstance;
        private PACKAGE_EXECUTION_STATE currentState = PACKAGE_EXECUTION_STATE.PES_UNKNOWN;

        [DllImport("ZenovaLauncher.dll")]
        private static extern void SetStateChangeCallback(Callback fn);

        [DllImport("ZenovaLauncher.dll")]
        private static extern void UnregisterStateChanges();

        public StateChangeCallback(MainWindow someWindow)
        {
            window = someWindow;
            mInstance = new Callback(Handler);
            SetStateChangeCallback(mInstance);
        }

        private void Handler(int state)
        {
            currentState = (PACKAGE_EXECUTION_STATE)state;
            window.updatePlayText(LauncherWrapper.executionStateAsString(currentState));
        }

        public PACKAGE_EXECUTION_STATE getState()
        {
            return currentState;
        }

        public void unregister()
        {
            UnregisterStateChanges();
        }
    }

    public partial class LauncherWrapper
    {
        /*
         * Import functions from ZenovaLauncher.dll
         */
        [DllImport("ZenovaLauncher.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void LaunchMinecraft(bool forceRestart);

        [DllImport("ZenovaLauncher.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void OpenMinecraftFolder();

        [DllImport("ZenovaLauncher.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void OpenModsFolder();

        [DllImport("ZenovaLauncher.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern PACKAGE_EXECUTION_STATE GetMinecraftExecutionState();

        public static string executionStateAsString(PACKAGE_EXECUTION_STATE state)
        {
            switch (state)
            {
                case PACKAGE_EXECUTION_STATE.PES_UNKNOWN:
                    return "Unknown";
                case PACKAGE_EXECUTION_STATE.PES_RUNNING:
                    return "Running";
                case PACKAGE_EXECUTION_STATE.PES_SUSPENDING:
                    return "Suspending";
                case PACKAGE_EXECUTION_STATE.PES_SUSPENDED:
                    return "Suspended";
                case PACKAGE_EXECUTION_STATE.PES_TERMINATED:
                    return "Terminated";
                case PACKAGE_EXECUTION_STATE.PES_ERROR:
                    return "Error";
                default:
                    return "Invalid State";
            }
        }
    }
}
