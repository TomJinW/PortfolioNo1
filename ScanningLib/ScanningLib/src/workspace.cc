
#include "workspace.h"

#include <opencv2/aruco.hpp>
#include <opencv2/ml.hpp>
#include <cmath>

namespace dgene {

	int Fix(int input, int direction) {
		switch (direction) {
		case 1:
			if (input + 1 >= 4) {
				return 0;
			}
			return input + 1;
			break;

		case -1:
			if (input - 1 <0) {
				return 3;
			}
			return input - 1;
			break;
		default:
			return input;
			break;
		}
	}

	float Dist(const cv::Point3f& point) {
		return std::sqrt(point.x * point.x + point.y * point.y + point.z * point.z);
	}

	float DotProduct(const cv::Point3f& pt1, const cv::Point3f& pt2) {
		return pt1.x * pt2.x + pt1.y * pt2.y + pt1.z * pt2.z;
	}

    Workspace::Workspace(const std::string& data_path)
    :data_path_(data_path) {
        color_images_.clear();
        depth_images_.clear();

		intrinsic_ = cv::Mat::zeros(3, 3, CV_32FC1);

		/*
		[478.98422,0,179.74148,
		0,478.98422,319.31653,
		0,0,1]
		*/

		intrinsic_.at<float>(0, 0) = 478.98422;
		intrinsic_.at<float>(0, 2) = 179.74148;
		intrinsic_.at<float>(1, 1) = 478.98422;
		intrinsic_.at<float>(1, 2) = 319.31653;
		intrinsic_.at<float>(2, 2) = 1.0;

		std::cout << "Intrinsics: " << intrinsic_ << std::endl;
    }

    void Workspace::ReadData(const int image_num) {
        const std::string color_path = data_path_ + "/img/";
		const std::string depth_path = data_path_ + "/depth/";

        color_images_.resize(image_num);
        depth_images_.resize(image_num);
		all_corners_.resize(image_num);

        for (size_t i = 0; i < image_num; i++) {
            // Read color image
			const std::string image_path = color_path + "/" + std::to_string(i) + ".jpg";
            cv::Mat color_mat = cv::imread(image_path);
			// Resize the color to 2 * depth resolution
			cv::Size color_size;
			color_size.height = 1280;
			color_size.width = 720;

			cv::resize(color_mat, color_mat, color_size);
#ifdef DEBUG
			std::cout << "Image name: " << image_path << std::endl;
			std::cout << "Image size: " << color_mat.cols << " * " << color_mat.rows << std::endl;
			std::cout << "Image channels: " << color_mat.channels() << std::endl;
#endif
			color_images_[i] = color_mat;

			// Read depth image
			const std::string depth_p = depth_path + "/D" + std::to_string(i) + ".csv";

			cv::Ptr<cv::ml::TrainData> depth_csv = cv::ml::TrainData::loadFromCSV(depth_p, 0);

			cv::Mat depth_part1_mat = depth_csv->getSamples();

			depth_part1_mat.convertTo(depth_part1_mat, CV_32FC1);

			cv::Mat depth_part2_mat = depth_csv->getResponses();
			depth_part2_mat.convertTo(depth_part2_mat, CV_32FC1);
#ifdef DEBUG
			std::cout << "Depth name: " << depth_p << std::endl;
			std::cout << "Depth size: " << depth_part1_mat.cols << " * " << depth_part1_mat.rows << std::endl;
			std::cout << "Depth channels: " << depth_part1_mat.channels() << std::endl;
#endif

			cv::Size size;
			size.height = depth_part1_mat.rows;
			size.width = depth_part1_mat.cols + 1;

			cv::Mat depth_mat = cv::Mat(size, CV_32FC1);

			depth_part1_mat.copyTo(depth_mat(cv::Rect(0, 0, depth_part1_mat.cols, depth_part1_mat.rows)));
			depth_part2_mat.copyTo(depth_mat(cv::Rect(depth_part1_mat.cols, 0, depth_part2_mat.cols, depth_part2_mat.rows)));
#ifdef DEBUG
			std::cout << "Test data values: " << std::endl;
			std::cout << "Up-left: " << depth_mat.at<float>(0, 0) << " Down-left: " << depth_mat.at<float>(depth_mat.rows - 1, 0) <<
				" Up-right: " << depth_mat.at<float>(0, depth_mat.cols - 1) << " Down-right: " << depth_mat.at<float>(depth_mat.rows - 1, depth_mat.cols - 1) << std::endl;
#endif
			depth_images_[i] = depth_mat;

			// Prepare corner information
			all_corners_[i] = DetectCorners(color_mat, depth_mat);
		}
    }

	std::unordered_map<int, cv::Point2f> Workspace::DetectCorners(const cv::Mat& image, const cv::Mat& depth) {
		std::unordered_map<int, cv::Point2f> corners;

		std::vector<int> marker_ids;
		std::vector< std::vector<cv::Point2f> > marker_corners, rejected_candidates;

		cv::Ptr<cv::aruco::Dictionary> dict = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_100);

		cv::aruco::detectMarkers(image, dict, marker_corners, marker_ids);

		/*cv::aruco::drawDetectedMarkers(image, marker_corners, marker_ids);
		cv::imshow("out", image);
		cv::waitKey(0);*/

		int pt_num = 0;
		for (size_t i = 0; i < marker_corners.size(); i++) {
			
			// corners.insert(std::pair<int, cv::Point2f>(marker_ids[i], marker_corners[i]));
			for (size_t j = 0; j < 4; j++) {
				// Draw circles for testing
				//cv::circle(image, marker_corners[i][j], 2, cv::Scalar(255, 0, 0), 5);

				// Filtering pts
//				cv::Point3f pc = Get3DPointFromDepth(depth, marker_corners[i][Fix(j, +0)]);
//				cv::Point3f p1 = Get3DPointFromDepth(depth, marker_corners[i][Fix(j, -1)]);
//				cv::Point3f p2 = Get3DPointFromDepth(depth, marker_corners[i][Fix(j, +1)]);
//
//				float dist1 = Dist(p1 - pc);
//				float dist2 = Dist(p2 - pc);
//#ifdef DEBUG
//				std::cout << "DISTANCE " << dist1 << std::endl;
//				std::cout << "DISTANCE " << dist2 << std::endl;
//#endif
//
//				if ((dist1 >= 0.0120 && dist1 <= 0.0180) && (dist2 >= 0.0120 && dist2 <= 0.0180)) {
//					std::cout << "ADDED" << std::endl;
//					float aCosine = DotProduct(p1 - pc, p2 - pc) / Dist(p1 - pc) / Dist(p2 - pc);
//					std::cout << "ADDED aCosine" << std::endl;
//					float angle = acos(aCosine) * 180 / float(M_PI);
//					std::cout << "ADDED angle" << std::endl;
//					if (angle >= 80 && angle <= 100) {
						int index = marker_ids[i] * 4 + j;
//
						//corners.insert(std::pair<int, cv::Point2f>(index, marker_corners[i][j]));
//
						corners.emplace(index, marker_corners[i][j]);
//
//						pt_num++;
//					}
//
//				}
				/*cv::imshow("out", image);
				cv::waitKey(0);*/
			}
			/*cv::imshow("out", image);
			cv::waitKey(0);*/
		}

#ifdef DEBUG
		// std::cout << pt_num << " Corners ADDED" << std::endl;
#endif

		return corners;
	}

	cv::Point3f Workspace::Get3DPointFromDepth(const cv::Mat& depth, const cv::Point2f& coords) {
		// Color resolusion is 2 * depth resolution
		float d = depth.at<float>(coords.y / 2.0, coords.x / 2.0);

		cv::Point3f point; point.x = int(coords.x / 2.0); point.y = int(coords.y / 2.0); point.z = d;
		
		return point;
	}
}