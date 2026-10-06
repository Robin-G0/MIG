using System;

namespace MotionInputGrid
{
    // Learning fixture for configs/default.json; this is not camera detection.
    public static class SyntheticFrames
    {
        public static MigTracker.Packet LeftRaise(ulong sequence)
        {
            var packet = MigTracker.Packet.Empty();
            WriteLeftRaise(ref packet, sequence);
            return packet;
        }

        public static void WriteLeftRaise(ref MigTracker.Packet packet, ulong sequence)
        {
            if (packet.Body == null || packet.Body.Length != 264 ||
                packet.Hands == null || packet.Hands.Length != 252)
            {
                throw new ArgumentException("Create the reusable packet with Packet.Empty()");
            }
            Array.Clear(packet.Body, 0, packet.Body.Length);
            Array.Clear(packet.Hands, 0, packet.Hands.Length);
            packet.Aspect = 1;
            packet.HandCount = packet.HandWorldMask = 0;
            packet.Sequence = sequence;
            packet.TimestampMs = (long)sequence * 20;
            SetJoint(ref packet, 11, 0.65f, 0.45f);
            SetJoint(ref packet, 12, 0.35f, 0.45f);
            float row = sequence < 60 ? 5.5f : sequence < 70 ? 4.5f : 2.5f;
            SetJoint(ref packet, 15, 0.62f, 0.45f + (row - 3.5f) * 0.06f);
            SetJoint(ref packet, 16, 0.25f, 0.65f);
        }

        public static void WriteRaisedHands(ref MigTracker.Packet packet, ulong sequence)
        {
            WriteLeftRaise(ref packet, sequence);
            float row = sequence < 60 ? 5.5f : Math.Max(1.5f, 5.5f - (sequence - 59) / 5f);
            float y = 0.45f + (row - 3.5f) * 0.06f;
            SetJoint(ref packet, 15, 0.62f, y);
            SetJoint(ref packet, 16, 0.38f, y);
        }

        private static void SetJoint(ref MigTracker.Packet packet, int joint, float x, float y)
        {
            int offset = joint * 8;
            packet.Body[offset] = x;
            packet.Body[offset + 1] = y;
            packet.Body[offset + 3] = 1;
        }
    }
}
