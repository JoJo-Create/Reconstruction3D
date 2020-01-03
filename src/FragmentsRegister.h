//
// Created by jojo on 03.01.20.
//

#ifndef RECONSTRUCTION3D_FRAGMENTSREGISTER_H
#define RECONSTRUCTION3D_FRAGMENTSREGISTER_H

#include <list>
#include "util/Parser.h"
#include "Open3D/Open3D.h"
#include "Open3D/Registration/GlobalOptimization.h"
#include "Open3D/Registration/ColoredICP.h"

namespace Reconstruction
{
    class FragmentsRegister
    {
    public:
        static void registerFragments(std::string config_file, size_t n_fragments);

    private:
        struct MatchingResult
        {
            MatchingResult(size_t s_, size_t t_)
            {
                s = s_;
                t = t_;
                Tcsw = Eigen::Matrix4d::Identity();
                Tctcs = Eigen::Matrix4d::Identity();
                information = Eigen::Matrix6d ::Identity();
            }
            size_t s;
            size_t t;
            bool success;
            Eigen::Matrix4d Tcsw;
            Eigen::Matrix4d Tctcs;
            Eigen::Matrix6d information;
        };

        static void makePoseGraphForScene(Parser config, size_t n_fragments);
        static void registerPoincloudPair(Parser config, MatchingResult& matching_result);
        static void preprocessPointCloud(Parser config, const open3d::geometry::PointCloud pcd,
                                         open3d::geometry::PointCloud& pcd_down,
                                         open3d::registration::Feature& pcd_fpfh);
        static bool computeInitialRegistration(const Parser config, const size_t s, const size_t t,
                                               const open3d::geometry::PointCloud source_pcd_down,
                                               const open3d::geometry::PointCloud target_pcd_down,
                                               const open3d::registration::Feature source_fpfh,
                                               const open3d::registration::Feature target_fpfh,
                                               Eigen::Matrix4d& Tctcs, Eigen::Matrix6d& information);
//        static void mutiScaleICP();
        static bool register_point_cloud_fpfh();
        static void updatePoseGraph(Parser config, const MatchingResult matching_result, open3d::registration::PoseGraph& poseGraph);
        static void optimizePoseGraph();
    };
}



#endif //RECONSTRUCTION3D_FRAGMENTSREGISTER_H