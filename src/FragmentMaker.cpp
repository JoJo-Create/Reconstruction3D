//
// Created by jojo on 01.01.20.
//

#include <Open3D/Registration/ColoredICP.h>
#include "FragmentMaker.h"
#include "LocalRefiner.h"
#include "GeometryMethods.h"
#include "RegistrationMethods.h"

using namespace Reconstruction;

void FragmentMaker::makeFragments(const FrameVector frameVector, const std::string config_file)
{
    Parser config;
    config.load(config_file);
    size_t n_frames_per_fragment = config.getValue<int>("n_frames_per_fragment");
    size_t n_fragments = std::ceil(frameVector.size()/n_frames_per_fragment) + 1;

    std::vector<FrameVector> fragments;
    size_t frame_id = 0;
    for(size_t i = 0; i < n_fragments; i++)
    {
        Reconstruction::FrameVector fragment;
        if(i != n_fragments-1)
        {
            for(int num = 0; num < n_frames_per_fragment; num++, frame_id++)
            {
                fragment.push_back(frameVector[frame_id]);
            }
        }
        else
        {
            for(; frame_id < frameVector.size(); frame_id++)
            {
                fragment.push_back(frameVector[frame_id]);
            }
        }
        fragments.push_back(fragment);
    }

    for(auto fragment : fragments)
    {
        static size_t i = 0;
        Reconstruction::FragmentMaker::processSingleFragment(i,fragment,config_file);
        i++;
    }
}


void FragmentMaker::processSingleFragment(size_t fragment_id, FrameVector frameVector, const std::string config_file)
{
    using namespace open3d;
    Parser config;
    config.load(config_file);
    std::vector<std::reference_wrapper<Frame>> frameVecRef;
    frameVecRef.insert(frameVecRef.end(),frameVector.begin(),frameVector.end());

    std::shared_ptr<geometry::PointCloud> pcd(new geometry::PointCloud);
//    *pcd = LocalRefiner::refineAndCreateMPointCloud(frameVecRef,config);
    makePoseGraph(fragment_id, frameVector, config);
//    optimizePoseGraph(fragment_id,config);
//    GeometryMethods::createPointCloundFromFrames(frameVector,config,pcd,true);
//    visualization::DrawGeometries({pcd});
    makePointCloud(fragment_id,frameVector,config);
    io::WritePointCloud(Parser::plyFileName(fragment_id),*pcd);

}

void FragmentMaker::makePoseGraph(size_t fragment_id, FrameVector frameVector, Parser config)
{
    using namespace open3d;
    registration::PoseGraph poseGraph;
    if(frameVector.empty())
        return;

    size_t n_keyFrames_per_fragment = config.getValue<int>("n_keyFrames_per_fragment");//to do
    size_t end_id = frameVector.size();
    size_t keyFrame_id_base = frameVector.size()/n_keyFrames_per_fragment;
    if(keyFrame_id_base == 0)
        keyFrame_id_base = 1;

    auto Tc0w = frameVector[0].getConstTcw();
    poseGraph.nodes_.push_back(registration::PoseGraphNode(Eigen::Matrix4d::Identity()));

    for(size_t s = 0; s < end_id; s++)
    {
        for(size_t t = s + 1; t < end_id; t++)
        {
            //odometry
            if(t == s + 1)
            {
                auto Tctw = frameVector[t].getConstTcw();
                auto Twcs = frameVector[s].getConstTwc();
//                auto Tc0ct = Tc0w * frameVector[t].getConstTwc();
                Eigen::Matrix4d Tctcs = Tctw * Twcs;


                std::shared_ptr<geometry::PointCloud> target_pcd;
                std::shared_ptr<geometry::PointCloud> source_pcd;

                GeometryMethods::createPointCloundFromFrames({frameVector[t]},config,target_pcd,true);
                GeometryMethods::createPointCloundFromFrames({frameVector[s]},config,source_pcd,true);

                auto voxel_size = config.getValue<double>("voxel_size");
                std::vector<double> voxel_sizes = {voxel_size,voxel_size/2.0,voxel_size/4.0};
                std::vector<size_t> max_iters = {50, 30, 14};

                Eigen::Matrix6d information = Eigen::Matrix6d::Identity();
                utility::SetVerbosityLevel(utility::VerbosityLevel::Debug);

                auto fitness = RegistrationMethods::mutiScaleICP(
                        config,
                        *source_pcd,
                        *target_pcd,
                        voxel_sizes,
                        max_iters,
                        Tctcs,
                        information,
                        false
                );
                auto Tc0cs = poseGraph.nodes_[s].pose_;

                if(fitness<0.5)
                {
                    Tctcs = Tctw * Twcs;
                }
                auto Tc0ct = Tc0cs * Tctcs.inverse();

                poseGraph.nodes_.push_back(registration::PoseGraphNode(Tc0ct));

                poseGraph.edges_.push_back(registration::PoseGraphEdge(s,t,Tctcs,information,false));
            }
            //local loop closure
//            else
//            {
//                //todo loopclosure detection
//                auto Tctw = frameVector[t].getConstTcw();
//                auto Twcs = frameVector[s].getConstTwc();
//                Eigen::Matrix4d Tctcs = Tctw * Twcs;
//
//                std::shared_ptr<geometry::PointCloud> target_pcd;
//                std::shared_ptr<geometry::PointCloud> source_pcd;
//
//                GeometryMethods::createPointCloundFromFrames({frameVector[t]},config,target_pcd,true);
//                GeometryMethods::createPointCloundFromFrames({frameVector[s]},config,source_pcd,true);
//
//                auto voxel_size = config.getValue<double>("voxel_size");
//                auto result = registration::RegistrationColoredICP(*source_pcd,*target_pcd,voxel_size,Tctcs,
//                                                                   registration::ICPConvergenceCriteria(1e-6,1e-6,50));
//
//                Tctcs = result.transformation_;
//
//                auto information = registration::GetInformationMatrixFromPointClouds(*source_pcd,
//                                                                                *target_pcd,
//                                                                                voxel_size*1.4,
//                                                                                Tctcs);
//                if(result.transformation_.trace() == 4.0
//                && information(5,5)/std::min(source_pcd->points_.size(),target_pcd->points_.size())<0.3)
//                    continue;
//
////                RegistrationMethods::draw_registration_result(*source_pcd,*target_pcd,Tctcs);
//                registration::PoseGraphEdge edge(s,t,Tctcs,information,true);
//                poseGraph.edges_.push_back(edge);
//            }
        }
    }
    io::WritePoseGraph(Parser::poseGraphName(fragment_id),poseGraph);
}

void FragmentMaker::optimizePoseGraph(size_t fragment_id, Parser config)
{
    using namespace open3d;
    auto max_correspondence_distance = config.getValue<double>("max_depth_diff");
    auto preference_loop_closure = config.getValue<double>("preference_loop_closure_local");
    utility::SetVerbosityLevel(utility::VerbosityLevel::Debug);
    registration::PoseGraph poseGraph;
    io::ReadPoseGraph(TemplatePoseGraphName(fragment_id),poseGraph);
    auto method = registration::GlobalOptimizationLevenbergMarquardt();
    auto criteria = registration::GlobalOptimizationConvergenceCriteria();
    auto option = registration::GlobalOptimizationOption(max_correspondence_distance,0.25,preference_loop_closure,0);

    registration::GlobalOptimization(poseGraph,method,criteria,option);
    io::WritePoseGraph(TemplatePoseGraphName(fragment_id),poseGraph);
    utility::SetVerbosityLevel(utility::VerbosityLevel::Error);

}

void FragmentMaker::makePointCloud(size_t fragment_id,FrameVector frameVector, Parser config)
{
    using namespace open3d;
    registration::PoseGraph poseGraph;
    io::ReadPoseGraph(TemplatePoseGraphName(fragment_id),poseGraph);

    size_t intergrate_n_frame_per_fragment = config.getValue<int>("intergrate_n_frame_per_fragment");
    size_t n_frame_per_fragment = frameVector.size();
    size_t frame_id_base = std::ceil(n_frame_per_fragment/intergrate_n_frame_per_fragment);
    if(frame_id_base == 0)
        frame_id_base = 1;


    double voxel_size = config.getValue<double>("volume_size")/config.getValue<double>("resolution");
    double depth_factor = config.getValue<double>("depth_factor");
    double depth_truncate = config.getValue<double>("depth_truncate");

    integration::ScalableTSDFVolume volume(voxel_size,0.04,open3d::integration::TSDFVolumeColorType::RGB8);


    int width = config.getValue<int>("Camera.width");
    int height = config.getValue<int>("Camera.height");
    double fx = config.getValue<double>("Camera.fx");
    double fy = config.getValue<double>("Camera.fy");
    double cx = config.getValue<double>("Camera.cx");
    double cy = config.getValue<double>("Camera.cy");
    camera::PinholeCameraIntrinsic intrinsic;
    intrinsic.SetIntrinsics(width,height,fx,fy,cx,cy);


    for(size_t i = 0; i < frameVector.size(); i++)
    {
        if(i % frame_id_base == 0)
        {
            std::cout<<"fragment:"<<fragment_id<<" integrating "<<i<<"frame\n";
            auto frame = frameVector[i];
            auto pose = poseGraph.nodes_[i].pose_;

            geometry::RGBDImage rgbd;
            GeometryMethods::createRGBDImageFromFrame(frame,config,rgbd,false);
            volume.Integrate(rgbd,intrinsic,pose.inverse());
        }
    }
//    auto mesh = volume.ExtractTriangleMesh();
//    mesh->ComputeVertexNormals();
    auto pcd = volume.ExtractPointCloud();

    io::WritePointCloud(Parser::plyFileName(fragment_id),*pcd);
}
//
//std::shared_ptr<open3d::geometry::PointCloud> FragmentMaker::createPoinCloudFromFrame(Frame frame, Parser config)
//{
//    using namespace open3d;
//    int width = config.getValue<int>("Camera.width");
//    int height = config.getValue<int>("Camera.height");
//    double fx = config.getValue<double>("Camera.fx");
//    double fy = config.getValue<double>("Camera.fy");
//    double cx = config.getValue<double>("Camera.cx");
//    double cy = config.getValue<double>("Camera.cy");
//    camera::PinholeCameraIntrinsic intrinsic;
//    intrinsic.SetIntrinsics(width,height,fx,fy,cx,cy);
//
//    double depth_factor = config.getValue<double>("depth_factor");
//    double depth_truncate = config.getValue<double>("depth_truncate");
//
//    geometry::Image depth;
//    geometry::Image infraRed;
//    bool read = false;
//    read = io::ReadImage(frame.getDepthImagePath().c_str(), depth);
//    if(!read)
//        return NULL;
//    read = io::ReadImageFromPNG(frame.getInfraRedImagePath().c_str(),infraRed);
//    if(!read)
//        return NULL;
//    auto rgbd = geometry::RGBDImage::CreateFromColorAndDepth(
//            infraRed, depth, depth_factor,
//            depth_truncate, true);
//
//    return geometry::PointCloud::CreateFromRGBDImage(*rgbd,intrinsic);
//}
