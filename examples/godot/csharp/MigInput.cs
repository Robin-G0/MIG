using System;
using System.Runtime.InteropServices;
using Godot;
using MotionInputGrid;

public partial class MigInput : Node3D
{
    [Export(PropertyHint.File, "*.json")]
    public string ProfilePath { get; set; } = "";
    [Export]
    public bool UseSyntheticDemo { get; set; }
    [Signal]
    public delegate void MotionActionEventHandler(string action, string inputId);
    private MigTracker tracker;
    private ulong demoSequence;
    private double demoTime;
    private MigTracker.Packet demoPacket = MigTracker.Packet.Empty();
    private readonly float[] wrist = new float[4];
    private Action<string, string> actionHandler;
    protected virtual bool RaisedHands => false;
    private Label status;
    private bool actionInFrame;
    private static bool libraryResolverInstalled;

    private static void InitializeNativeLibrary()
    {
        if (libraryResolverInstalled)
        {
            return;
        }
        // Godot executes assemblies from .godot/, not beside project.godot.
        // Resolve the loose ABI library from project resources instead of cwd.
        NativeLibrary.SetDllImportResolver(typeof(MigTracker).Assembly, (name, assembly, searchPath) =>
        {
            if (name != "mig-c")
            {
                return IntPtr.Zero;
            }
            string filename = OperatingSystem.IsWindows() ? "mig-c.dll" : "libmig-c.so";
            string path = ProjectSettings.GlobalizePath("res://" + filename);
            return NativeLibrary.Load(path);
        });
        libraryResolverInstalled = true;
    }

    public override void _Ready()
    {
        CreateInterface();
        try
        {
            InitializeNativeLibrary();
            tracker = new MigTracker("{\"schema_version\":2,\"tracking\":{\"hands\":true},\"inputs\":[]}");
            var path = RaisedHands ? "res://raised-hands.json" : ProfilePath;
            if (path.Length > 0)
            {
                ImportProfile(path);
            }
            actionHandler = HandleAction;
        }
        catch (Exception error)
        {
            GD.PushError(error.Message);
        }
    }

    private void CreateInterface()
    {
        var layer = new CanvasLayer();
        AddChild(layer);
        var panel = new PanelContainer { Position = new Vector2(12, 12) };
        layer.AddChild(panel);
        var stack = new VBoxContainer();
        panel.AddChild(stack);
        status = new Label { Text = RaisedHands ? "Lower your hands, then raise either wrist."
            : "Import a profile, then keep shoulders visible." };
        if (!RaisedHands)
        {
            var dialog = new FileDialog { FileMode = FileDialog.FileModeEnum.OpenFile,
                Access = FileDialog.AccessEnum.Filesystem, Filters = new[] { "*.json ; MIG profiles" } };
            layer.AddChild(dialog);
            dialog.FileSelected += ImportProfile;
            var button = new Button { Text = "Import JSON profile" };
            button.Pressed += () => dialog.PopupCenteredRatio();
            stack.AddChild(button);
        }
        stack.AddChild(status);
    }

    public void ImportProfile(string path)
    {
        try
        {
            using var profile = Godot.FileAccess.Open(path, Godot.FileAccess.ModeFlags.Read);
            if (profile == null)
            {
                throw new InvalidOperationException("Cannot open MIG profile");
            }
            tracker.ImportJson(profile.GetAsText());
            status.Text = "Profile imported. Recalibrating.";
        }
        catch (Exception error)
        {
            status.Text = error.Message;
            GD.PushError(error.Message);
        }
    }

    public override void _Process(double delta)
    {
        if (!UseSyntheticDemo || tracker == null || demoSequence >= 90)
        {
            return;
        }
        demoTime += delta;
        if (demoTime >= 0.02)
        {
            demoTime = 0;
            if (RaisedHands)
            {
                SyntheticFrames.WriteRaisedHands(ref demoPacket, ++demoSequence);
            }
            else
            {
                SyntheticFrames.WriteLeftRaise(ref demoPacket, ++demoSequence);
            }
            SubmitFrame(demoPacket);
        }
    }

    // Connect a pose provider to this method. Call on the main thread only.
    public void SubmitFrame(MigTracker.Packet packet)
    {
        if (tracker == null)
        {
            return;
        }
        actionInFrame = false;
        tracker.Update(ref packet, actionHandler);
        if (tracker != null && tracker.TryCoordinate(15, wrist, 2))
        {
            Position = new Vector3(wrist[0], wrist[1], -wrist[2]);
        }
    }

    private void HandleAction(string action, string inputId)
    {
        if (!actionInFrame)
        {
            status.Text = "";
            actionInFrame = true;
        }
        status.Text += (status.Text.Length == 0 ? "" : "\n") + $"{action} (input {inputId})";
        GD.Print($"MIG {inputId}: {action}");
        EmitSignal(SignalName.MotionAction, action, inputId);
    }

    public override void _ExitTree()
    {
        tracker?.Dispose();
        tracker = null;
    }
}
