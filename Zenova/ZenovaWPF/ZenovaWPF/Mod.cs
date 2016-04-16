using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace ZenovaGUI
{
    partial class Utils
    {
        public static ImageSource getResource(string psAssemblyName, string psResourceName)
        {
            Uri oUri = new Uri("pack://application:,,,/" + psAssemblyName + ";component/" + psResourceName, UriKind.RelativeOrAbsolute);
            return BitmapFrame.Create(oUri);
        }
    }

    public class ModItem
    {
        public string ModName { get; }
        public string ModAuthor { get; }
        public string ModDescription { get; }
        public Uri ModIconPath { get; }

        public ModItem(string name, string author, string description, string iconPath)
        {
            ModName = name;
            ModAuthor = author;
            ModDescription = description;
            if(iconPath.Length == 0)
            {
                ModIconPath = new Uri("pack://application:,,,/ZenovaWPF;component/Resources/DefaultIcon.png", UriKind.RelativeOrAbsolute);
            }
            else
            {
                ModIconPath = new Uri("pack://application:,,,/ZenovaWPF;component/Resources/DefaultIcon.png", UriKind.RelativeOrAbsolute);
            }
        }
    }

    public class Mods :
    System.Collections.ObjectModel.ObservableCollection<ModItem>
    {
        public Mods()
        {
            Add(new ModItem("Super Haxs", "LeetHacker99", "Adds Cool hacks!", ""));
            Add(new ModItem("Super Haxs", "LeetHacker99", "Adds Cool hacks!", ""));
            Add(new ModItem("Super Haxs", "LeetHacker99", "Adds Cool hacks!", ""));
            Add(new ModItem("Super Haxs", "LeetHacker99", "Adds Cool hacks!", ""));
            Add(new ModItem("Super Haxs", "LeetHacker99", "Adds Cool hacks!", ""));
        }
    }
}
