using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Loksim3D.WetterEdit.Win32Wrapper
{
    public class Win32Exception : Exception
    {
        public Win32Exception(string msg, int errorCode) : base(msg)
        {
            ErrorCode = errorCode;
        }

        public int ErrorCode { get; set; }
    }
}
