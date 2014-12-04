using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Windows;
using System.Windows.Markup;
using System.Globalization;
using Microsoft.Shell;
using Loksim3D.WetterEdit.Settings;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.WindowsAPICodePack.Dialogs;
using Loksim3D.WetterEdit.Win32Wrapper;
using Loksim3D.WetterEdit.Resources;
using System.Windows.Shell;
using System.Reflection;

[assembly: CLSCompliant(true)]
namespace Loksim3D.WetterEdit
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application, ISingleInstanceApp
    {
        private const string Unique = "LoksimWetterEditor-7F9571D8-3151-4801-AAFF-40EEAD0EDA95";

        private MainWindow mainWnd;

        private static string EscapeCommandLineArguments(string[] args)
        {
            string arguments = "";
            foreach (string arg in args)
            {
                arguments += " \"" +
                    arg.Replace("\\", "\\\\").Replace("\"", "\\\"") +
                    "\"";
            }
            return arguments;
        }

        [STAThread]
        public static void Main()
        {
            try
            {
                CultureInfo cInf = new CultureInfo(RegistrySettings.Default.UiCulture);
                Loksim3D.WetterEdit.Resources.Strings.Culture = cInf;
            }
            catch (ArgumentException ex)
            {
                MessageBox.Show("Invalid Language Settings " + RegistrySettings.Default.UiCulture + "\n" + ex.Message);
            }


            if (Environment.OSVersion.Version.Major >= 6 && RegistrySettings.Default.DefaultAdminStartMode != RegistrySettings.AdminStartMode.DoNotStartAsAdmin)
            {
                // Unter Vista oder höher prüfen ob wir in das Datenverzeichnis lt. paths.ini schreiben dürfen
                // Wenn nicht schauen wir ob wir als Admin ausgeführt werden
                // Wenn nicht als Admin ausgeführt => User fragen ob als Admin ausführen bzw in Optionen nachschauen was user möchte
                
                string datadir = L3dFilePath.LoksimDirectory.AbsolutePath;
                
                try
                {
                    string file = TempFileUtility.GetTempFileName("l3d", 0, datadir/*Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location)*/);
                    File.Delete(file);
                }
                catch (Win32Exception ex)
                {
                    if (ex.ErrorCode == TempFileUtility.ERROR_ACCESS_DENIED)
                    {
                        IntPtr tkHandle;
                        IntPtr procHandle = Process.GetCurrentProcess().Handle;
                        Win32Utility.OpenProcessToken(procHandle,
                            (uint)System.Security.Principal.TokenAccessLevels.Read, out tkHandle);

                        uint TokenInfLength = 0;
                        bool Result;

                        // first call gets lenght of TokenInformation
                        Result = Win32Utility.GetTokenInformation(tkHandle, Win32Utility.TOKEN_INFORMATION_CLASS.TokenElevation, IntPtr.Zero, TokenInfLength, out TokenInfLength);
                        IntPtr TokenInformation = Marshal.AllocHGlobal((int)TokenInfLength);

                        Result = Win32Utility.GetTokenInformation(tkHandle, Win32Utility.TOKEN_INFORMATION_CLASS.TokenElevation, TokenInformation, TokenInfLength, out TokenInfLength);
                        if (Result)
                        {
                            Win32Utility.TOKEN_ELEVATION TokenElevation = (Win32Utility.TOKEN_ELEVATION)Marshal.PtrToStructure(TokenInformation, typeof(Win32Utility.TOKEN_ELEVATION));
                            TaskDialog dlg = new TaskDialog
                            {
                                Caption = Strings.UacDlgCaption,
                                InstructionText = Strings.UacDlgInstructionText,
                                Text = Strings.UacDlgText,
                                FooterCheckBoxChecked = false,
                                FooterCheckBoxText = Strings.UacDlgCheckBoxText,
                                Cancelable = false,
                            };
                            TaskDialogButton btn = new TaskDialogButton { Text = Strings.UacDlgBtnAdmin, UseElevationIcon = true, Default = true };
                            btn.Click += (sender, e) => dlg.Close(TaskDialogResult.Yes);
                            dlg.Controls.Add(btn);
                            btn = new TaskDialogButton { Text = Strings.UacDlgBtnNoAdmin };
                            btn.Click += (sender, e) => dlg.Close(TaskDialogResult.No);
                            dlg.Controls.Add(btn);
                            if (RegistrySettings.Default.DefaultAdminStartMode == RegistrySettings.AdminStartMode.StartAsAdmin || dlg.Show() == TaskDialogResult.Yes)
                            {
                                if (dlg.FooterCheckBoxChecked.GetValueOrDefault(false))
                                {
                                    RegistrySettings.Default.DefaultAdminStartMode = RegistrySettings.AdminStartMode.StartAsAdmin;
                                }

                                ProcessStartInfo info = new ProcessStartInfo();
                                info.FileName = System.Reflection.Assembly.GetCallingAssembly().Location;
                                info.UseShellExecute = true;
                                info.Verb = "runas"; // Provides Run as Administrator 
                                info.Arguments = EscapeCommandLineArguments(Environment.GetCommandLineArgs());
                                Process.Start(info);
                                Marshal.FreeHGlobal(TokenInformation);
                                return;
                            }
                            else if (RegistrySettings.Default.DefaultAdminStartMode == RegistrySettings.AdminStartMode.Ask && dlg.FooterCheckBoxChecked.GetValueOrDefault(false))
                            {
                                RegistrySettings.Default.DefaultAdminStartMode = RegistrySettings.AdminStartMode.DoNotStartAsAdmin;
                            }
                        }
                        Marshal.FreeHGlobal(TokenInformation);
                    }
                }
            }

            // Der Editor soll pro Installation nur 1x gestartet werden können.
            // Hat der User mehrere Editoren in unterschiedlichen Verzeichnissen oder mit unterschiedlichen Namen, kann er diese jeweils 1x starten
            // Deshalb ist in der Unique-ID auch noch die Assembly Location kodiert
            if (SingleInstance<App>.InitializeAsFirstInstance(Unique + "-" + System.Reflection.Assembly.GetCallingAssembly().Location.GetHashCode()))
            {
                var application = new App();

                application.InitializeComponent();
                application.Run();

                // Allow single instance code to perform cleanup operations
                SingleInstance<App>.Cleanup();
            }
        }

        static void Current_DispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            Application.Current.DispatcherUnhandledException -= Current_DispatcherUnhandledException;
            try
            {
                File.WriteAllText(Path.Combine(Path.GetDirectoryName(Assembly.GetEntryAssembly().Location), "error_wetter_edit.txt"), e.Exception.ToString());
            }
            catch (Exception)
            {
            }
            //MessageBox.Show(e.Exception.Message + "\n" + e.Exception.StackTrace, "Unhandled Error", MessageBoxButton.OK, MessageBoxImage.Error);

        }


        #region ISingleInstanceApp Members

        public bool SignalExternalCommandLineArgs(IList<string> args)
        {
            OpenNewWindow(args.Skip(1));
            return true;
        }

        #endregion

        private void OpenNewWindow(IEnumerable<string> args)
        {
            MainWindow newWnd = null;
            string fileToOpen = null;
            if (args != null && args.Count() > 0 && !string.IsNullOrWhiteSpace(args.First()))
            {
                fileToOpen = args.First();
                if (File.Exists(fileToOpen))
                {
                    JumpList.AddToRecentCategory(fileToOpen);
                }
            }
            if (mainWnd == null && fileToOpen != null)
            {
                newWnd = new MainWindow(fileToOpen);
                newWnd.Show();
            }
            else if (mainWnd == null)
            {
                newWnd = new MainWindow();
                newWnd.Show();
            }
            else if (fileToOpen != null && fileToOpen.Trim().StartsWith("-"))
            {
                mainWnd.CreateNewFile(fileToOpen);
            }
            else if (fileToOpen != null)
            {
                mainWnd.OpenFile(new L3dFilePath(fileToOpen));
            }
            else
            {
                if (mainWnd.WindowState == System.Windows.WindowState.Minimized)
                {
                    mainWnd.WindowState = System.Windows.WindowState.Normal;
                }
                mainWnd.Activate();
                //mainWnd.BringIntoView();
            }
            if (mainWnd == null)
            {
                mainWnd = newWnd;
            }
        }

        private void Application_Startup(object sender, StartupEventArgs e)
        {
            Application.Current.DispatcherUnhandledException += new System.Windows.Threading.DispatcherUnhandledExceptionEventHandler(Current_DispatcherUnhandledException);

            // Ensure the current culture passed into bindings 
            // is the OS culture. By default, WPF uses en-US 
            // as the culture, regardless of the system settings.
            FrameworkElement.LanguageProperty.OverrideMetadata(
                typeof(FrameworkElement),
                new FrameworkPropertyMetadata(
                    XmlLanguage.GetLanguage(CultureInfo.CurrentCulture.IetfLanguageTag)));

#if DEBUG
            //L3dFilePath.LoksimDirectory = new L3dFilePath(@"D:\Loksim3D");
#endif
            //Args = e.Args;

            OpenNewWindow(e.Args);
        }
    }
}
