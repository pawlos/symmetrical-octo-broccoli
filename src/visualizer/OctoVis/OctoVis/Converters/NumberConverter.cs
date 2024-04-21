using System.Globalization;
using System.Windows.Data;

namespace OctoVis.Converters;

public class NumberConverter : IValueConverter
{
    public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
    {
        if (value is double)
        {
            return $"{(double)value:0.00}";
        }

        return value;
    }

    public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}