using System;
using System.Runtime.InteropServices;
using System.Text;

namespace MotionInputGrid
{
    // A reusable bridge for Unity and Godot .NET. One owning thread per tracker.
    public sealed class MigTracker : IDisposable
    {
        private const string Library = "mig-c";
        private IntPtr handle;

        [StructLayout(LayoutKind.Sequential)]
        public struct Packet
        {
            public long TimestampMs;
            public ulong Sequence;
            public float Aspect;
            public uint HandCount;
            public uint HandWorldMask;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 264)]
            public float[] Body;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 252)]
            public float[] Hands;

            public static Packet Empty()
            {
                return new Packet { Aspect = 1, Body = new float[264], Hands = new float[252] };
            }
        }

        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern uint mig_abi_version();
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern UIntPtr mig_packet_size();
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr mig_last_error();
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr mig_create(byte[] json);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern void mig_destroy(IntPtr tracker);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern int mig_load(IntPtr tracker, byte[] json);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern int mig_update(IntPtr tracker, ref Packet packet);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr mig_event_action(IntPtr tracker, uint index);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern IntPtr mig_event_id(IntPtr tracker, uint index);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern int mig_active(IntPtr tracker, uint index);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern int mig_coordinate(IntPtr tracker, int joint, int system, [Out] float[] point);
        [DllImport(Library, CallingConvention = CallingConvention.Cdecl)]
        private static extern void mig_reset(IntPtr tracker, int recalibrate);

        private static byte[] Utf8(string value)
        {
            if (value.IndexOf('\0') >= 0)
            {
                throw new ArgumentException("Embedded NUL is not allowed");
            }
            return Encoding.UTF8.GetBytes(value + "\0");
        }

        private static string ReadUtf8(IntPtr pointer)
        {
            if (pointer == IntPtr.Zero)
            {
                return "";
            }
            int length = 0;
            while (Marshal.ReadByte(pointer, length) != 0)
            {
                ++length;
            }
            byte[] bytes = new byte[length];
            Marshal.Copy(pointer, bytes, 0, length);
            return Encoding.UTF8.GetString(bytes);
        }

        private static void Check(int result)
        {
            if (result < 0)
            {
                throw new InvalidOperationException(ReadUtf8(mig_last_error()));
            }
        }

        private void RequireOpen()
        {
            if (handle == IntPtr.Zero)
            {
                throw new ObjectDisposedException(nameof(MigTracker));
            }
        }

        public MigTracker(string json)
        {
            if (mig_abi_version() != 1 || mig_packet_size().ToUInt64() != (ulong)Marshal.SizeOf<Packet>())
            {
                throw new InvalidOperationException("MIG packet ABI mismatch");
            }
            handle = mig_create(Utf8(json));
            if (handle == IntPtr.Zero)
            {
                throw new InvalidOperationException(ReadUtf8(mig_last_error()));
            }
        }

        public void ImportJson(string json)
        {
            RequireOpen();
            Check(mig_load(handle, Utf8(json)));
        }

        public void Update(ref Packet packet, Action<string, string> onAction)
        {
            RequireOpen();
            if (packet.Body == null || packet.Body.Length != 264 || packet.Hands == null || packet.Hands.Length != 252)
            {
                throw new ArgumentException("Packet arrays must have their fixed ABI lengths");
            }
            int count = mig_update(handle, ref packet);
            Check(count);
            if (count == 0 || onAction == null)
            {
                return;
            }
            // Copy all strings before callbacks; a callback may import/reset/dispose.
            string[] actions = new string[count];
            string[] ids = new string[count];
            for (uint index = 0; index < count; ++index)
            {
                actions[index] = ReadUtf8(mig_event_action(handle, index));
                ids[index] = ReadUtf8(mig_event_id(handle, index));
            }
            for (int index = 0; index < count; ++index)
            {
                onAction?.Invoke(actions[index], ids[index]);
            }
        }

        public float[] Coordinate(int joint, int system = 0)
        {
            float[] point = new float[4];
            return TryCoordinate(joint, point, system) ? point : null;
        }

        public bool TryCoordinate(int joint, float[] point, int system = 0)
        {
            RequireOpen();
            if (point == null || point.Length < 4)
            {
                throw new ArgumentException("Coordinate buffer needs four floats");
            }
            return mig_coordinate(handle, joint, system, point) == 1;
        }

        public bool Active(uint input)
        {
            RequireOpen();
            return mig_active(handle, input) != 0;
        }

        public void Reset(bool recalibrate = false)
        {
            RequireOpen();
            mig_reset(handle, recalibrate ? 1 : 0);
        }

        public void Dispose()
        {
            if (handle != IntPtr.Zero)
            {
                mig_destroy(handle);
                handle = IntPtr.Zero;
            }
        }
    }
}
