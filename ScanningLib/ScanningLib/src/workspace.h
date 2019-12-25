
#ifndef SCANNING_SRC_WORKSPACE_H_
#define SCANNING_SRC_WORKSPACE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <opencv2/opencv.hpp>

namespace dgene {

    class Workspace {

        public:
            Workspace(const std::string& data_path);

			inline const cv::Mat& Intrinsics() const;

			inline int NumColorImages() const;
			inline const cv::Mat& ColorImage(const int id) const;

			inline int NumDepthImages() const;
			inline const cv::Mat& DepthImage(const int id) const;

			inline const std::unordered_map<int, cv::Point2f>& GetCorners(const int id) const;

            void ReadData(const int image_num);

			inline std::string GetOutputDir() const;
			
        private:
			// Can't use const & here
			std::unordered_map<int, cv::Point2f> DetectCorners(const cv::Mat& image, const cv::Mat& depth);

			cv::Point3f Get3DPointFromDepth(const cv::Mat& depth, const cv::Point2f& coords);

            std::string data_path_;

			std::vector<cv::Mat> color_images_;
			std::vector<cv::Mat> depth_images_;
			std::vector<std::unordered_map<int, cv::Point2f>> all_corners_;

			cv::Mat intrinsic_;
    };

	const cv::Mat& Workspace::Intrinsics() const {
		return intrinsic_;
	}

	int Workspace::NumColorImages() const {
		return color_images_.size();
	}

	const cv::Mat& Workspace::ColorImage(const int id) const {
		return color_images_[id];
	}

	int Workspace::NumDepthImages() const {
		return depth_images_.size();
	}

	const cv::Mat& Workspace::DepthImage(const int id) const {
		return depth_images_[id];
	}

	const std::unordered_map<int, cv::Point2f>& Workspace::GetCorners(const int id) const {
		return all_corners_[id];
	}

	std::string Workspace::GetOutputDir() const {
	return data_path_;
	}
}

#endif