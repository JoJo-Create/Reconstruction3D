//
// Created by jojo on 05.01.20.
//

#ifndef RECONSTRUCTION3D_POSEGRAPHMETHODS_H
#define RECONSTRUCTION3D_POSEGRAPHMETHODS_H

#include "Open3D/Open3D.h"
#include "util/Parser.h"
#include "Frame.h"

namespace Reconstruction
{
    class PoseGraphMethods
    {
    public:
        struct MatchingResult
        {
            MatchingResult(size_t s_, size_t t_, Eigen::Matrix4d Tctcs_ = Eigen::Matrix4d::Identity())
            {
                s = s_;
                t = t_;
                success = false;
                Tctcs = Tctcs_;
                information = Eigen::Matrix6d ::Identity();
            }
            size_t s;
            size_t t;
            bool success;
            Eigen::Matrix4d Tctcs;
            Eigen::Matrix6d information;
        };
        static std::vector<MatchingResult> createMatchesFromPoseGraph(const std::string poseGraphName);
        static open3d::registration::PoseGraph createGlobalPoseGraphFromFrames(FrameVector frameVector, const std::string config_file);
        static void createPoseGraphFromMatches(std::vector<MatchingResult> matchingResults, const std::string poseGraphName);

        static void updatePoseGraph(
                const MatchingResult matching_result,
                Eigen::Matrix4d& Tcsw,
                open3d::registration::PoseGraph& poseGraph);
        static void optimizePoseGraphForScene(const std::string config_file, const std::string source_poseGraphName, const std::string refined_poseGraphName);
        static void optimizePoseGraphForFragment(const std::string config_file, const std::string source_poseGraphName, const std::string refined_poseGraphName);

    };
}



#endif //RECONSTRUCTION3D_POSEGRAPHMETHODS_H
