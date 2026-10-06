using System;
using System.IO;
using MotionInputGrid;

internal static class Program
{
    private static void Main(string[] arguments)
    {
        using var tracker = new MigTracker(File.ReadAllText(arguments[0]));
        int accepted = 0;
        var packet = MigTracker.Packet.Empty();
        for (ulong sequence = 1; sequence <= 90; ++sequence)
        {
            SyntheticFrames.WriteLeftRaise(ref packet, sequence);
            tracker.Update(ref packet, (action, id) =>
            {
                if (action != "left_raise")
                {
                    throw new Exception("Unexpected managed action");
                }
                ++accepted;
            });
        }
        if (accepted != 1 || tracker.Coordinate(15) == null || tracker.Coordinate(15, 1) != null)
        {
            throw new Exception("Managed recognition/XYZ mismatch");
        }
        var point = new float[4];
        if (!tracker.TryCoordinate(15, point) || tracker.TryCoordinate(15, point, 1))
        {
            throw new Exception("Reusable coordinate query mismatch");
        }
        try
        {
            tracker.ImportJson("{}");
            throw new Exception("Invalid import accepted");
        }
        catch (InvalidOperationException)
        {
            tracker.Reset();
        }
        if (arguments.Length > 1)
        {
            tracker.ImportJson(File.ReadAllText(arguments[1]));
            int raised = 0;
            for (ulong sequence = 1; sequence <= 90; ++sequence)
            {
                SyntheticFrames.WriteRaisedHands(ref packet, sequence);
                tracker.Update(ref packet, (action, id) =>
                {
                    if (action != "left_raise" && action != "right_raise")
                    {
                        throw new Exception("Unexpected raised-hands action");
                    }
                    ++raised;
                });
            }
            if (raised != 2)
            {
                throw new Exception("Raised-hands fixture must trigger both wrists");
            }
        }
        tracker.Dispose();
        tracker.Dispose();
        Console.WriteLine("Managed C ABI integration passed");
    }
}
