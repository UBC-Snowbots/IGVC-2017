/**
 * Created by: Valerian Ratu
 * Created on: 2018/01/13
 * Description: A class which pre-processed pointcloud input from the zed
 *              into the right frame and colourspace
 */

#include <ColourspaceConverter.h>
#include <pcl/point_types_conversion.h>

ColourspaceConverter::ColourspaceConverter()
{

}

PointCloud<PointXYZHSV>::Ptr ColourspaceConverter::Process(PointCloud<PointXYZRGB>::ConstPtr& input) {
    PointCloud<PointXYZHSV>::Ptr output(new PointCloud<PointXYZHSV>());
    // Do processing here rather than using:
    // PointCloudXYZRGBtoXYZHSV(*input, *output);
    // since above does not like constant inputs
    // Test speed
    output->width = input->width;
    output->height = input->width;
    for (size_t i = 0; i < input->points.size(); i++)
    {
        PointXYZHSV p;
        ColourspaceConverter::PointXYZRGBAtoXYZHSV(input->points[i], p);
        output->points.push_back(p);
    }
    return output;
}

void ColourspaceConverter::PointXYZRGBAtoXYZHSV(const PointXYZRGB &in, PointXYZHSV &out) {

    // Bugfixin'
    out.x = in.x;
    out.y = in.y;
    out.z = in.z;

    const unsigned char max = std::max (in.r, std::max (in.g, in.b));
    const unsigned char min = std::min (in.r, std::min (in.g, in.b));

    out.v = static_cast <float> (max) / 255.f;

    if (max == 0) // division by zero
    {
        out.s = 0.f;
        out.h = 0.f; // h = -1.f;
        return;
    }

    const float diff = static_cast <float> (max - min);
    out.s = diff / static_cast <float> (max);

    if (min == max) // diff == 0 -> division by zero
    {
        out.h = 0;
        return;
    }

    if      (max == in.r) out.h = 60.f * (      static_cast <float> (in.g - in.b) / diff);
    else if (max == in.g) out.h = 60.f * (2.f + static_cast <float> (in.b - in.r) / diff);
    else                  out.h = 60.f * (4.f + static_cast <float> (in.r - in.g) / diff); // max == b

    if (out.h < 0.f) out.h += 360.f;
}
