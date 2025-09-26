using System.Configuration;
using System.Data;
using System.Windows;
using System.Windows.Forms;
using System.Drawing;


using Application = System.Windows.Application;

namespace theater;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    private NotifyIcon trayIcon = null!;

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        trayIcon = new NotifyIcon();
        trayIcon.Icon = new Icon(SystemIcons.Application, 40, 40);
        trayIcon.Visible = true;
        trayIcon.Text = "Theater";

        var menu = new ContextMenuStrip();
        menu.Items.Add("Open", null, (s, ev) => ShowMainWindow());
        menu.Items.Add("Exit", null, (s, ev) => ExitApp());
        trayIcon.ContextMenuStrip = menu;

        trayIcon.DoubleClick += (s, ev) => ShowMainWindow();

        Current.MainWindow = new MainWindow();
        Current.MainWindow.Hide();
    }

    protected void ShowMainWindow()
    {
        if (Current.MainWindow == null)
        {
            Current.MainWindow = new MainWindow();
        }

        Current.MainWindow.Show();
        Current.MainWindow.WindowState = WindowState.Normal;
        Current.MainWindow.Activate();
    }
    protected void ExitApp()
    {
        trayIcon.Visible = false;
        Current.Shutdown();
    }
}

