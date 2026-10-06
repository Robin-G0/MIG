using System;
using MotionInputGrid;
using UnityEngine;
using UnityEngine.Events;

public class MigInput : MonoBehaviour
{
    public TextAsset Profile;
    public bool UseSyntheticDemo;
    public UnityEvent<string> OnAction = new UnityEvent<string>();
    private MigTracker tracker;
    private ulong demoSequence;
    private float demoTime;
    private MigTracker.Packet demoPacket = MigTracker.Packet.Empty();
    private readonly float[] wrist = new float[4];
    private Action<string, string> actionHandler;
    protected virtual bool RaisedHands => false;
    private string status = "Import a profile, then keep shoulders visible.";
    private string profilePath = "";
    private bool actionInFrame;

    protected virtual void OnEnable()
    {
        try
        {
            var profile = RaisedHands ? Resources.Load<TextAsset>("MIG/raised-hands") : Profile;
            if (RaisedHands && profile == null)
            {
                throw new InvalidOperationException("Copy Resources/MIG/raised-hands.json into Assets.");
            }
            tracker = new MigTracker(profile != null ? profile.text :
                "{\"schema_version\":2,\"tracking\":{\"hands\":true},\"inputs\":[]}");
            status = RaisedHands ? "Lower your hands, then raise either wrist." : status;
            actionHandler = HandleAction;
            demoSequence = 0;
            demoTime = 0;
        }
        catch (Exception error)
        {
            Debug.LogError(error.Message, this);
            enabled = false;
        }
    }

    protected virtual void Update()
    {
        if (!UseSyntheticDemo || tracker == null || demoSequence >= 90)
        {
            return;
        }
        demoTime += Time.deltaTime;
        if (demoTime >= 0.02f)
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

    // Call from your pose provider on Unity's main thread, once per new camera
    // frame, using monotonic milliseconds and increasing sequence numbers.
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
            // World metres, Y-up. Relative to the hips, not a camera-world origin.
            transform.localPosition = new Vector3(wrist[0], wrist[1], wrist[2]);
        }
    }

    private void HandleAction(string action, string inputId)
    {
        if (!actionInFrame)
        {
            status = "";
            actionInFrame = true;
        }
        status += (status.Length == 0 ? "" : "\n") + $"{action} (input {inputId})";
        Debug.Log($"MIG {inputId}: {action}", this);
        OnAction.Invoke(action);
    }

    public void ImportProfile(string path)
    {
        try
        {
            tracker.ImportJson(System.IO.File.ReadAllText(path));
            status = "Profile imported. Recalibrating.";
        }
        catch (Exception error)
        {
            status = error.Message;
        }
    }

    protected virtual void OnGUI()
    {
        GUILayout.BeginArea(new Rect(12, 12, 760, 160), GUI.skin.box);
        if (!RaisedHands)
        {
            profilePath = GUILayout.TextField(profilePath);
            if (GUILayout.Button("Import JSON profile"))
            {
                ImportProfile(profilePath);
            }
        }
        GUILayout.Label(status);
        GUILayout.EndArea();
    }

    protected virtual void OnDisable()
    {
        tracker?.Dispose();
        tracker = null;
    }
}
