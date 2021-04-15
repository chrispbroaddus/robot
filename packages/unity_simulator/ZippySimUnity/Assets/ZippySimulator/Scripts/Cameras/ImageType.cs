
namespace Zippy {
    /// <summary>
    /// Image type.
    /// A Camera may generate multiple images. What type of image is being dealt with.
    /// Changes to this must also be reflected in the zippy_image_interop.h plugin api header file
    /// </summary>
    public enum ImageType {
        Greyscale = 0,  //Greyscale image
        Color = 1,      //Color image
        Depth = 2,      //Depth image
        XYZ = 3,        //XYZ Pointcloud
        //Segmented     //TODO in the future
    }
}
