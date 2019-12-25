//
// Created by zhang on 8/9/2018.
//

#ifndef SCANNING_SRC_RECONSTRUCTION_H
#define SCANNING_SRC_RECONSTRUCTION_H

#include "workspace.h"

namespace dgene {

	struct FusedPoint {
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float nx = 0.0f;
		float ny = 0.0f;
		float nz = 0.0f;
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
	};

	struct ReconstructionOptions {
		// Max thread num to be used in stereo matching process, -1 <-> machine
		// supported max thread number
		int max_thread_num = -1;

		// Max gpu num to be used in stereo matching process, -1 <-> machine
		// supported max gpu number
		int max_gpu_num = -1;

		// Depth range in which to randomly sample depth hypotheses.
		double depth_min = 0.0f;
		double depth_max = 0.5f;

		// Minimum number of fused pixels to produce a point.
		int min_num_pixels = 3;

		// Maximum number of pixels to fuse into a single point.
		int max_num_pixels = 10000;

		// Maximum depth in consistency graph traversal.
		int max_traversal_depth = 100;

		// Maximum relative difference between measured and projected pixel.
		double max_reproj_error = 2.0f;

		// Maximum relative difference between measured and projected depth.
		double max_depth_error = 0.01f;

		bool Check() const;
	};

    class Reconstruction {
	public:
        
        float process = 0;
		Reconstruction(const ReconstructionOptions& options, Workspace* workspace);

		void Run();
	private:
		void SolveCoarseRT();

		void FusePointCloud();

		void FusePerPoint();
		
		bool ComputeMesh();

		void writeProfile(cv::Point3d tSum);

		void GetMatched2D3DPoints(const std::unordered_map<int, cv::Point2f>& corners1,
							  const std::unordered_map<int, cv::Point2f>& corners2, 
							  std::vector<cv::Point2f>& points1, std::vector<cv::Point3f>& points2,
							  const cv::Mat& depth_mat1, const cv::Mat& depth_mat2);

		ReconstructionOptions options_;

		Workspace* workspace_;

		std::vector<cv::Mat> r_vectors_;
		std::vector<cv::Mat> t_vectors_;

		float max_squared_reproj_error_;
		std::vector<bool> used_images_;
		std::vector<bool> fused_images_;
		std::vector<cv::Mat> fused_pixel_masks_;
		std::vector<cv::Matx34d> P_;
		std::vector<cv::Matx34d> inv_P_;
		std::vector<cv::Matx33d> inv_R_;

		struct FusionData {
			int image_id = -1;
			int row = 0;
			int col = 0;
			int traversal_depth = -1;
			bool operator()(const FusionData& data1, const FusionData& data2) {
				return data1.image_id > data2.image_id;
			}
		};

		std::vector<FusionData> fusion_queue_;
		std::vector<FusedPoint> fused_points_;
		std::vector<float> fused_points_x_;
		std::vector<float> fused_points_y_;
		std::vector<float> fused_points_z_;
		std::vector<float> fused_points_nx_;
		std::vector<float> fused_points_ny_;
		std::vector<float> fused_points_nz_;
		std::vector<unsigned char> fused_points_r_;
		std::vector<unsigned char> fused_points_g_;
		std::vector<unsigned char> fused_points_b_;

		std::vector<cv::Mat> rgb_mats_;
		std::vector<cv::Mat> depth_mats_;

		std::string outputPointCloudPath;
		std::string outputMeshPath;
		std::string outputMeshInvertPath;
    };

	// Write the point cloud to PLY file.
    void WritePlyText(const std::string& path, const std::vector<FusedPoint>& points,dgene::Workspace * workspace);

	void WritePlyBinary(const std::string& path, const std::vector<FusedPoint>& points);

	cv::Matx34d ComposeProjectionMatrix(const cv::Mat& K, const cv::Mat& R,
		const cv::Mat& T);

	cv::Matx34d ComposeInverseProjectionMatrix(const cv::Mat& K, const cv::Mat& R,
		const cv::Mat& T);
}

#endif //OBJECTSCANNING_RECONSTRUCTION_H
