# Read-only inventory for the disposable Windows CI desktop. Do not guess a
# resolution the virtual adapter cannot expose. Microsoft API reference:
# https://learn.microsoft.com/windows/win32/api/winuser/nf-winuser-enumdisplaysettingsw
$ErrorActionPreference = 'Stop'
if (-not ('KuubikDisplayInventory' -as [type])) {
    Add-Type -TypeDefinition @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

public static class KuubikDisplayInventory {
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    public struct Mode {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string deviceName;
        public short specVersion, driverVersion, size, driverExtra;
        public uint fields;
        public int positionX, positionY;
        public uint orientation, fixedOutput;
        public short color, duplex, yResolution, ttOption, collate;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)] public string formName;
        public short logPixels;
        public uint bitsPerPel, width, height, displayFlags, frequency;
        public uint icmMethod, icmIntent, mediaType, ditherType, reserved1, reserved2;
        public uint panningWidth, panningHeight;
    }
    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool EnumDisplaySettings(string device, uint index, ref Mode mode);
    public static Mode[] Read() {
        var result = new List<Mode>();
        for (uint index = 0; ; ++index) {
            var mode = new Mode();
            mode.size = (short)Marshal.SizeOf(typeof(Mode));
            if (!EnumDisplaySettings(null, index, ref mode)) break;
            result.Add(mode);
        }
        return result.ToArray();
    }
}
'@
}
[KuubikDisplayInventory]::Read() | ForEach-Object {
    [pscustomobject]@{ Width = [int]$_.width; Height = [int]$_.height;
                       BitsPerPixel = [int]$_.bitsPerPel; Frequency = [int]$_.frequency }
} | Sort-Object Width,Height,BitsPerPixel,Frequency -Unique
