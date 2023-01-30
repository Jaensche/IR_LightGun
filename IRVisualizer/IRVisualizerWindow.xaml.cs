using System.IO.Ports;
using System.Windows;
using System.Linq;
using System.Windows.Media;
using System.Windows.Controls;
using System.ComponentModel;
using System.Collections.Generic;
using System.Windows.Shapes;

namespace IRVisualizer
{
    public partial class IRVisualizerWindow : Window, System.IDisposable
    {
        readonly SerialPort serialPort;
        readonly List<Ellipse> dots;
        private const double horizontalOffset = -100;
        private const double verticalOffset = 100;

        public IRVisualizerWindow()
        {
            InitializeComponent();
            CompositionTarget.Rendering += CompositionTarget_Rendering;
            dots = new List<Ellipse>
            {
                a,
                b,
                c,
                d
            };

            var serialPorts = SerialPort.GetPortNames();

            if (serialPorts.Length > 0)
            {
                serialPort = new SerialPort(serialPorts[0], 19200, Parity.None, 8, StopBits.One);
                serialPort.RtsEnable = true;
                serialPort.Open();
            }
        }

        public void CompositionTarget_Rendering(object sender, System.EventArgs e)
        {
            if(serialPort != null)
            {
                serialPort.WriteLine("");

                if (serialPort.BytesToRead > 0)
                {
                    var currentLine = serialPort.ReadLine();
                    if (currentLine.Length < 1)
                        return;

                    var splitValues = currentLine.Split(',');
                    if (splitValues.Length > 7 && !splitValues.Any(x => x == string.Empty))
                    {
                        var values = splitValues.Select(x => double.Parse(x) / 1023 * Width).ToList();

                        if (values.Count > 7)
                        {
                            int i = 0;
                            foreach(var dot in dots)
                            {
                                dot.SetValue(Canvas.LeftProperty, Width - values[i++] + horizontalOffset);
                                dot.SetValue(Canvas.TopProperty, values[i++] + verticalOffset);
                            }
                        }
                    }
                }
            }
        }

        void DataWindow_Closing(object sender, CancelEventArgs e)
        {
            if (serialPort != null && serialPort.IsOpen)
            {
                serialPort.Close();
            }
        }

        public void Dispose()
        {
            if (serialPort != null && serialPort.IsOpen)
            {
                serialPort.Close();
                serialPort.Dispose();
            }
        }
    }    
}
