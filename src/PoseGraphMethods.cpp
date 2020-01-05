//
// Created by jojo on 05.01.20.
//

#include "PoseGraphMethods.h"
#include "Open3D/Registration/GlobalOptimization.h"

using namespace Reconstruction;

void PoseGraphMethods::updatePoseGraph(
        const MatchingResult matching_result,
        Eigen::Matrix4d &Tcsw,
        open3d::registration::PoseGraph &poseGraph)
{
    using namespace open3d;
    auto s = matching_result.s;
    auto t = matching_result.t;
    auto information = matching_result.information;
    auto Tctcs = matching_result.Tctcs;
    //odometry
    if(t == s + 1 )
    {
        auto Tctw = matching_result.Tctcs * Tcsw;
        Tcsw = Tctw; //update
        registration::PoseGraphNode node(Tctw.inverse());
        registration::PoseGraphEdge edge(s,t,Tctcs,information,false);
        poseGraph.nodes_.push_back(node);
        poseGraph.edges_.push_back(edge);
    }
        //loop closure
    else
    {
        registration::PoseGraphEdge edge(s,t,Tctcs,information,true);
        poseGraph.edges_.push_back(edge);
    }
}

void PoseGraphMethods::createPoseGraphFromMatches(
        std::vector<PoseGraphMethods::MatchingResult> matchingResults,
        std::string poseGraphName)
{
    using namespace open3d;
    registration::PoseGraph poseGraph;
    poseGraph.nodes_.push_back(registration::PoseGraphNode(Eigen::Matrix4d::Identity())); //base node
    Eigen::Matrix4d current_Tcsw = Eigen::Matrix4d::Identity();
    for(auto matching_result : matchingResults)
    {
        if(matching_result.success)
        {
            updatePoseGraph(matching_result, current_Tcsw, poseGraph);
        }
    }
    io::WritePoseGraph(poseGraphName,poseGraph);
}

std::vector<PoseGraphMethods::MatchingResult> PoseGraphMethods::createMatchesFromPoseGraph(const std::string poseGraphName)
{
    using namespace open3d;
    registration::PoseGraph poseGraph;
    std::vector<MatchingResult> matchingResults;
    io::ReadPoseGraph(poseGraphName,poseGraph);

    for(auto edge : poseGraph.edges_)
    {

        auto s = edge.source_node_id_;
        auto t = edge.target_node_id_;
        auto trans = edge.transformation_;

        matchingResults.push_back(MatchingResult(s,t,trans));
    }

    return matchingResults;

}

void PoseGraphMethods::optimizePoseGraphForScene(const std::string config_file,const std::string source_poseGraphName, const std::string refined_poseGraphName)
{
    using namespace open3d;
    Parser config;
    config.load(config_file);

    utility::SetVerbosityLevel(utility::VerbosityLevel::Debug);
    registration::PoseGraph poseGraph;
    io::ReadPoseGraph(source_poseGraphName,poseGraph);
    auto method = registration::GlobalOptimizationLevenbergMarquardt();
    auto criteria = registration::GlobalOptimizationConvergenceCriteria();
    auto max_correspondence_distance = config.getValue<double>("voxel_size") * 1.4;
    auto preference_loop_closure = config.getValue<double>("preference_loop_closure");
    auto option = registration::GlobalOptimizationOption(max_correspondence_distance,
                                                         0.25,preference_loop_closure,0);
    registration::GlobalOptimization(poseGraph,method,criteria,option);
    io::WritePoseGraph(refined_poseGraphName,poseGraph);
    utility::SetVerbosityLevel(utility::VerbosityLevel::Error);
}