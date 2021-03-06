//
// Created by jojo on 07.01.20.
//

#ifndef RECONSTRUCTION3D_GEOMETRYMETHODS_H
#define RECONSTRUCTION3D_GEOMETRYMETHODS_H

#include "Frame.h"
#include "util/Parser.h"
#include "Open3D/Open3D.h"

namespace Reconstruction
{
    class GeometryMethods
    {
    public:
        static bool createRGBDImageFromFrame(const Frame frame, const Parser config, open3d::geometry::RGBDImage& rgbd, bool useIRImg = true);
        static bool createMeshAndPointCloudFromFrames(
                const FrameVector frameVector,
                const Parser config,
                std::shared_ptr<open3d::geometry::TriangleMesh>& mesh,
                std::shared_ptr<open3d::geometry::PointCloud>& pcd,
                bool color = false);
        static bool createPointCloundFromFrames(
                const FrameVector frameVector,
                const Parser config,
                std::shared_ptr<open3d::geometry::PointCloud>& pcd,
                bool color = false
                );
        static bool createMeshFromFrames(
                const FrameVector frameVector,
                const Parser config,
                std::shared_ptr<open3d::geometry::TriangleMesh>& mesh,
                bool color = false);
    };
}



#endif //RECONSTRUCTION3D_GEOMETRYMETHODS_H
