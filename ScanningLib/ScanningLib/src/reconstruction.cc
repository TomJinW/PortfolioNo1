//
// Created by zhang on 8/9/2018.
//

#include "reconstruction.h"
#include "mesh/compute_normal.h"
#include "ext/PoissonRecon/Poisson.h"

namespace dgene {

	template <typename T>
	float Median(std::vector<T> *elems) {
		//CHECK(!elems->empty());
		const size_t mid_idx = elems->size() / 2;
		std::nth_element(elems->begin(), elems->begin() + mid_idx, elems->end());
		if (elems->size() % 2 == 0) {
			const float mid_element1 = static_cast<float>((*elems)[mid_idx]);
			const float mid_element2 = static_cast<float>(
				*std::max_element(elems->begin(), elems->begin() + mid_idx));
			return (mid_element1 + mid_element2) / 2.0f;
		}
		else {
			return static_cast<float>((*elems)[mid_idx]);
		}
	}

	// Only can be used for iPhoneX
	cv::Point3f Point2dTo3d(const cv::Point3f & point) {
		float x = float((point.x - 179.74148) * point.z / 478.98422);
		float y = float((point.y - 319.31653) * point.z / 478.98422);
		float z = point.z;
		float data[3] = { x, y, z };
		cv::Point3f pt;
		pt.x = x;
		pt.y = y;
		pt.z = z;
		//cout << "MAT " << pt << endl;
		return pt;
	}

	cv::Mat GetCentralized(std::vector<cv::Mat>& input) {

		float data[3] = { 0.0,0.0,0.0 };
		cv::Mat output = cv::Mat(3, 1, CV_32FC1, data);

		for (size_t i = 0; i < input.size(); i++) {
			output += input[i];
		}
		output = output / float(input.size());

		for (int i = 0; i < input.size(); i++) {
			input[i] -= output;
		}
		return output.clone();
	}

	bool ReconstructionOptions::Check() const {
	/*	CHECK_OPTION_LT(depth_min, depth_max);
		CHECK_OPTION_GE(depth_min, 0.0f);*/
		return true;
	}

	Reconstruction::Reconstruction(const ReconstructionOptions& options, Workspace* workspace)
		: options_(options), workspace_(workspace) {
		
		options_.Check();

		outputPointCloudPath = workspace_->GetOutputDir() + "/PointCloud.ply";
		outputMeshPath = workspace_->GetOutputDir() + "/Mesh.ply";
		outputMeshInvertPath = workspace_->GetOutputDir() + "/MeshInvert.ply";

		max_squared_reproj_error_ = options_.max_reproj_error * options_.max_reproj_error;

		r_vectors_.clear();
		t_vectors_.clear();
	}

	bool Reconstruction::ComputeMesh() {

		std::vector<std::string> args;

		args.push_back("./binary");

		args.push_back("--in");
		args.push_back(outputMeshPath.c_str());

		args.push_back("--out");
		args.push_back(outputMeshPath.c_str());

        args.push_back("--colors");
        args.push_back("--verbose");
        
//        args.push_back("--pointWeight");
//        args.push_back("1.0");
//
//        args.push_back("--depth");
//        args.push_back("8");
//
//        args.push_back("--color");
//        args.push_back("16.0");

		//args.push_back("--ascii");
//
//
//        args.push_back("--density");
//        args.push_back("--verbose");


		std::vector<const char*> args_cstr;
		args_cstr.reserve(args.size());
		for (const auto& arg : args) {
			args_cstr.push_back(arg.c_str());
		}

        Poisson::reconstruction(args_cstr.size(), const_cast<char**>(args_cstr.data()));
        return true;
        
		// Poisson::reconstruction(args_cstr.size(),const_cast<char**>(args_cstr.data()));

//        if (PoissonRecon(args_cstr.size(), const_cast<char**>(args_cstr.data())) !=
//            EXIT_SUCCESS) {
//            return false;
//        }
//        //exit(1);
		return true;
	}

    void Reconstruction::writeProfile(cv::Point3d tSum){
        cv::Point3d tCenter = tSum / double(rgb_mats_.size());
        double dist = 0.0;
        for (size_t i = 0; i < rgb_mats_.size() - 1; i++) {
            cv::Point3d vecPt;
            vecPt.x = t_vectors_[i].at<double>(0);
            vecPt.y = t_vectors_[i].at<double>(1);
            vecPt.z = t_vectors_[i].at<double>(2);
            
            cv::Point3d centralized = vecPt - tCenter;
            dist += sqrt   (centralized.x * centralized.x +
                            centralized.y * centralized.y +
                            centralized.z * centralized.z );
        }
        dist = dist / double(rgb_mats_.size());
        ofstream profile;
        profile.open(workspace_->GetOutputDir()+"/profile.txt");
        profile << "camera_pose: extrinsics.txt\ncamera_intrinsic: intrinsics.txt\nimage_list: images.txt\nobj: Mesh.ply\n";
        profile << "N_REF_CAMERAS:" << rgb_mats_.size() << "\n";
        profile << "width_H:" << "720" << "\n";
        profile << "height_H:" << "1280" << "\n";
        profile << "width_L:" << "720" << "\n";
        profile << "height_L:" << "1280" << "\n";
        profile << "camera_center: " << tCenter.x << " "
        << tCenter.y << " "
        << tCenter.z << "\n";
        profile << "camera_radius: "<< dist << "\n";
        profile << "near: 0.2" << "\n";
        profile << "far: 1.2/" << "\n";
        profile << "mode: linear" << "\n";
        profile << "rows: 1"<< "\n";
        profile.close();

    }
    
    float steps[10] = {0.10,0.20,0.30,0.40,0.50,0.60,0.70,0.80,0.90,1.00};
	void Reconstruction::Run() {
		if (workspace_->NumColorImages() != workspace_->NumDepthImages()) {
			std::cerr << "Error: color image number is not matched to depth image number." << std::endl;
			return;
		}

        ofstream intrinsicsFile;
        intrinsicsFile.open(workspace_->GetOutputDir()+"/intrinsics.txt");
        ofstream imagesFile;
        imagesFile.open(workspace_->GetOutputDir()+"/images.txt");
        
        ostringstream oss;  //创建一个格式化输出流
        oss << workspace_->Intrinsics().at<float>(0,0)*2.0 << " "; //把值传递如流中
        oss << workspace_->Intrinsics().at<float>(1,1)*2.0 << " ";
        oss << workspace_->Intrinsics().at<float>(0,2)*2.0 << " ";
        oss << workspace_->Intrinsics().at<float>(1,2)*2.0 << " ";
        oss << "720" << " ";
        oss << "1280"  << " ";
        auto inStr = oss.str();
        
		for (size_t i = 0; i < workspace_->NumColorImages(); i++) {
			cv::Mat cur_color_mat = workspace_->ColorImage(i);
			cv::Mat cur_depth_mat = workspace_->DepthImage(i);

			cv::Size color_size;
			color_size.height = 1280 / 2;
			color_size.width = 720 / 2;

			cv::resize(cur_color_mat, cur_color_mat, color_size);

			rgb_mats_.push_back(cur_color_mat);
			depth_mats_.push_back(cur_depth_mat);
            
            // output intrinsics
            intrinsicsFile << i << " " << inStr << "\n";
            imagesFile << i << " "<< "img/" << i << ".jpg" << "\n";
            
            float cur = i;
            float all = workspace_->NumColorImages() - 1;
            process =  cur/all * 0.10;
            //std::cout << "Prepare image " << i << std::endl;
		}
        intrinsicsFile.close();
        imagesFile.close();
        
		SolveCoarseRT();

		FusePointCloud();

        process = 0.6;
		ComputePCNormal(outputPointCloudPath, outputMeshPath);
        process = 0.7;

		ComputeMesh();
        process = 0.9;

		smoothMesh(outputMeshPath,outputMeshPath,false);
        process = 0.95;
		invertMeshNormal(outputMeshPath,outputMeshInvertPath,false);
        process = 1.0;
		// WritePointCloud();

		// BuildTracks();
	}

	void Reconstruction::SolveCoarseRT() {

		// We set the first camera as the base camera, all other cameras are aligned to the first camera

        ofstream extrinsicsFile;
        extrinsicsFile.open(workspace_->GetOutputDir()+"/extrinsics.txt");
        extrinsicsFile << "0\t-0.000000 -1.000000 -0.000000\t-0.000000 -0.000000 -1.000000\t0.000000 0.000000 0.000000\n";
        cv::Point3d tSum; tSum.x = 0.0; tSum.y=0.0;tSum.z = 0.0;
        
		for (size_t i = 0; i < rgb_mats_.size() - 1; i++) {
            
            float cur = i;
            float all = rgb_mats_.size() - 2;
            process =  0.10 + cur/all * 0.10;
            
			cv::Mat cur_depth_mat = depth_mats_[i];
			std::unordered_map<int, cv::Point2f> cur_corners = workspace_->GetCorners(i);

			// Next image
			cv::Mat next_depth_mat = depth_mats_[i + 1];
			std::unordered_map<int, cv::Point2f> next_corners = workspace_->GetCorners(i + 1);

			// Image to image registration
			std::vector<cv::Point2f> matched_2dpoints1;
			std::vector<cv::Point3f> matched_3dpoints2;
			GetMatched2D3DPoints(cur_corners, next_corners, matched_2dpoints1, matched_3dpoints2, cur_depth_mat, next_depth_mat);

			/*cv::Mat cPt1 = GetCentralized(matched_points1);
			cv::Mat cPt2 = GetCentralized(matched_points2);*/

			cv::Mat distCoeffs(4, 1, cv::DataType<double>::type);
			distCoeffs.at<double>(0) = 0;
			distCoeffs.at<double>(1) = 0;
			distCoeffs.at<double>(2) = 0;
			distCoeffs.at<double>(3) = 0;

			cv::Mat rvec(3, 1, cv::DataType<double>::type);
			cv::Mat tvec(3, 1, cv::DataType<double>::type);

			float err = 0.0;
            cv::solvePnPRansac(matched_3dpoints2, matched_2dpoints1, workspace_->Intrinsics(), distCoeffs, rvec, tvec,false,5000,1.0,0.4,cv::noArray(),cv::SOLVEPNP_UPNP);
			/*cv::Mat R = GetRotation(matched_points1, matched_points2, cPt1, cPt2, 85, 250, err);
			std::cout << "Transform " << i + 1 << " to " << i << std::endl;
			std::cout << "ERROR " << err << std::endl;*/


			//cv::Mat t = cPt1 - R * cPt2;
			////// Combining
			//float data[4] = { 0.0,0.0,0.0,1 };
			//cv::Mat lastRow = cv::Mat(1, 4, CV_32F, data);
			//cv::hconcat(R, t, T);
			//T.push_back(lastRow); 

			cv::Mat rmat;
			cv::Rodrigues(rvec, rmat);

			std::cout << "ID: " << i << std::endl;
			std::cout << rmat << std::endl;
			std::cout << tvec << std::endl;
			std::cout << std::endl;

			if (r_vectors_.size() > 0) {
				cv::Mat last_r = r_vectors_.back();
				cv::Mat last_t = t_vectors_.back();

				rmat = last_r * rmat;
				tvec = last_r * tvec + last_t;

				std::cout << "After integrate: " << std::endl;
				std::cout << rmat << std::endl;
				std::cout << tvec << std::endl;
				std::cout << std::endl;
			}

			r_vectors_.push_back(rmat);
			t_vectors_.push_back(tvec);
    
            // Write ext
            cv::Mat r_trans = rmat;
            cv::Mat t_trans = tvec;
            extrinsicsFile << i+1 << "\t" <<
            -r_trans.at<double>(0, 1) << " " << -r_trans.at<double>(1, 1) << " " << -r_trans.at<double>(2, 1) << "\t" <<
            -r_trans.at<double>(0, 2) << " " << -r_trans.at<double>(1, 2) << " " << -r_trans.at<double>(2, 2) << "\t" <<
            t_trans.at<double>(0) << " "  << t_trans.at<double>(1) <<  " "<< t_trans.at<double>(2) << "\n";
            cv::Point3d tvecPt;
            tvecPt.x = t_trans.at<double>(0); tvecPt.y = t_trans.at<double>(1); tvecPt.z = t_trans.at<double>(2);
            tSum += tvecPt;
			// WritePointCloud(cur_depth_mat, next_depth_mat, rmat, tvec);
		}
        extrinsicsFile.close();
        writeProfile(tSum);
	}

	void Reconstruction::FusePointCloud() {
		fused_points_.clear();

		used_images_.resize(rgb_mats_.size(), false);
		fused_images_.resize(rgb_mats_.size(), false);
		fused_pixel_masks_.resize(rgb_mats_.size());
		P_.resize(rgb_mats_.size());
		inv_P_.resize(rgb_mats_.size());
		inv_R_.resize(rgb_mats_.size());

		int register_num = 0;
		// Start from 1
		for (size_t id = 0; id < rgb_mats_.size(); id++) {
            


			used_images_.at(id) = true;

			cv::Mat cur_color_mat = rgb_mats_[id];

			cv::Mat r_trans = cv::Mat::zeros(3, 3, CV_32FC1);
			cv::Mat t_trans = cv::Mat::zeros(3, 1, CV_32FC1);

			r_trans.at<float>(0, 0) = 1.0;
			r_trans.at<float>(1, 1) = 1.0;
			r_trans.at<float>(2, 2) = 1.0;
            
			if (register_num > 0) {
				r_trans = r_vectors_[register_num - 1];
				t_trans = t_vectors_[register_num - 1];

				r_trans.convertTo(r_trans, CV_32FC1);
				t_trans.convertTo(t_trans, CV_32FC1);

				// Add inverse
				r_trans = r_trans.inv();
				t_trans = -r_trans * t_trans;

				std::cout << r_trans << std::endl;
				std::cout << t_trans << std::endl;
			}

			fused_pixel_masks_.at(id) = cv::Mat(cur_color_mat.rows, cur_color_mat.cols, CV_8UC1);
			fused_pixel_masks_.at(id).setTo(cv::Scalar(0));

			P_.at(id) = ComposeProjectionMatrix(workspace_->Intrinsics(), r_trans, t_trans);

			inv_P_.at(id) = ComposeInverseProjectionMatrix(workspace_->Intrinsics(), r_trans, t_trans);

			cv::Matx33d r_matrix;
			cv::Mat rt = r_trans.t();
			rt.convertTo(r_matrix, CV_64F);
			inv_R_.at(id) = r_matrix;

			register_num++;
		}

		size_t num_fused_images = 0;
		// for (size_t id = 1; id < rgb_mats_.size(); id++) {
		for (size_t id = 0; id < rgb_mats_.size(); id++) {
            
            float cur = id;
            float all = rgb_mats_.size() - 1;
            process =  0.20 + cur/all * 0.30;
            
			std::cout << "Fusing image [" << id + 1 << "/" << 
				rgb_mats_.size() << "]" << std::endl;

			const auto &fused_pixel_mask = fused_pixel_masks_.at(id);

			FusionData data;
			data.image_id = id;
			data.traversal_depth = 0;

			cv::Mat cur_color_mat = rgb_mats_[id];

			for (data.row = 0; data.row < cur_color_mat.rows; data.row+=3) {
				for (data.col = 0; data.col < cur_color_mat.cols;
					data.col+=3) {
					if (fused_pixel_mask.at<unsigned char>(data.row, data.col)) {
						continue;
					}

					fusion_queue_.push_back(data);

					FusePerPoint();
				}
			}

			num_fused_images += 1;
			fused_images_.at(id) = true;
		}

		fused_points_.shrink_to_fit();

		if (fused_points_.empty()) {
			std::cout << "WARNING: Could not fuse any points. This is likely caused by "
				"incorrect settings - filtering must be enabled for the last "
				"call to patch match stereo."
				<< std::endl;
		}

		std::cout << "Number of fused points: " << fused_points_.size() << std::endl;

		WritePlyText(outputPointCloudPath, fused_points_,workspace_);
        
	}



	void Reconstruction::FusePerPoint() {
		if (fusion_queue_.size() != 1) {
			return;
		}

		cv::Vec4d fused_ref_point;

		fused_points_x_.clear();
		fused_points_y_.clear();
		fused_points_z_.clear();
		fused_points_nx_.clear();
		fused_points_ny_.clear();
		fused_points_nz_.clear();
		fused_points_r_.clear();
		fused_points_g_.clear();
		fused_points_b_.clear();

		while (!fusion_queue_.empty()) {
			const auto data = fusion_queue_.back();
			const int image_id = data.image_id;
			const int row = data.row;
			const int col = data.col;

			// std::cout << "Image Id: " << image_id << std::endl;
			// std::cout << "Row: " << row << std::endl;
			// std::cout << "Col: " << col << std::endl;

			const int traversal_depth = data.traversal_depth;

			fusion_queue_.pop_back();

			// Check if pixel already fused.
			auto &fused_pixel_mask = fused_pixel_masks_.at(image_id);
			if (fused_pixel_mask.at<unsigned char>(data.row, data.col)) {
				continue;
			}

			cv::Mat depth_mat = depth_mats_[image_id];

			const float depth = depth_mat.at<float>(row, col);

			// Pixels with negative depth are filtered.
			if (depth <= options_.depth_min || depth > options_.depth_max) {
				continue;
			}

			// If the traversal depth is greater than zero, the initial reference
			// pixel has already been added and we need to check for consistency.
			if (traversal_depth > 0) {
				// Project reference point into current view.
				const cv::Vec3d proj = P_.at(image_id) * fused_ref_point;

				// Depth error of reference depth with current depth.
				const float depth_error = std::abs((proj(2) - depth) / depth);
				if (depth_error > options_.max_depth_error) {
					continue;
				}

				// Reprojection error reference point in the current view.
				const float col_diff = proj(0) / proj(2) - col;
				const float row_diff = proj(1) / proj(2) - row;
				const float squared_reproj_error =
					col_diff * col_diff + row_diff * row_diff;
				if (squared_reproj_error > max_squared_reproj_error_) {
					continue;
				}
			}

			// Determine 3D location of current depth value.

			cv::Vec4d ab_xyz;
			ab_xyz(0) = col * depth;
			ab_xyz(1) = row * depth;
			ab_xyz(2) = depth;
			ab_xyz(3) = 1.0;

			const cv::Vec3d xyz = inv_P_.at(image_id) * ab_xyz;

			// Read the color of the pixel.
			cv::Mat color_mat = rgb_mats_[image_id];

			cv::Vec3b color = color_mat.at<cv::Vec3b>(row, col);

			// Set the current pixel as visited.
			fused_pixel_mask.at<unsigned char>(row, col) = true;

			// Accumulate statistics for fused point.
			fused_points_x_.push_back(xyz(0));
			fused_points_y_.push_back(xyz(1));
			fused_points_z_.push_back(xyz(2));
			// fused_points_nx_.push_back(normal(0));
			// fused_points_ny_.push_back(normal(1));
			// fused_points_nz_.push_back(normal(2));
			fused_points_r_.push_back(color(0));
			fused_points_g_.push_back(color(1));
			fused_points_b_.push_back(color(2));

			if (traversal_depth == 0) {
				fused_ref_point(0) = xyz(0);
				fused_ref_point(1) = xyz(1);
				fused_ref_point(2) = xyz(2);
				fused_ref_point(3) = 1.0;
				// fused_ref_normal = normal;
			}

			// std::cout << "Fused points x size: " << fused_points_x_.size() <<
			// std::endl;

			if (fused_points_x_.size() >=
				static_cast<size_t>(options_.max_num_pixels)) {
				break;
			}

			FusionData next_data;
			next_data.traversal_depth = traversal_depth + 1;

			if (next_data.traversal_depth >= options_.max_traversal_depth) {
				continue;
			}

			for (auto next_image_id = image_id + 1;
				next_image_id <= image_id + options_.min_num_pixels; next_image_id++) {
				if (next_image_id >= rgb_mats_.size()) {
					next_data.image_id =
						next_image_id - rgb_mats_.size();
				}
				else {
					next_data.image_id = next_image_id;
				}

				if (!used_images_.at(next_data.image_id) ||
					fused_images_.at(next_data.image_id)) {
					continue;
				}

				cv::Vec4d np;
				np(0) = xyz(0);
				np(1) = xyz(1);
				np(2) = xyz(2);
				np(3) = 1.0;

				const cv::Vec3d next_proj = P_.at(next_data.image_id) * np;
				next_data.col = static_cast<int>(std::round(next_proj(0) / next_proj(2)));
				next_data.row = static_cast<int>(std::round(next_proj(1) / next_proj(2)));

				// const auto &depth_map_size = depth_map_sizes_.at(next_image_id);
				if (next_data.col < 0 || next_data.row < 0 ||
					next_data.col >= depth_mats_[image_id].cols ||
					next_data.row >= depth_mats_[image_id].rows) {
					continue;
				}

				fusion_queue_.push_back(next_data);
			}
		}

		fusion_queue_.clear();

		const size_t num_pixels = fused_points_x_.size();
		if (num_pixels >= static_cast<size_t>(options_.min_num_pixels)) {
			FusedPoint fused_point;

			// Eigen::Vector3f fused_normal;
			// fused_normal.x() = internal::Median(&fused_points_nx_);
			// fused_normal.y() = internal::Median(&fused_points_ny_);
			// fused_normal.z() = internal::Median(&fused_points_nz_);
			// const float fused_normal_norm = fused_normal.norm();
			// if (fused_normal_norm < std::numeric_limits<float>::epsilon()) {
			//   return;
			// }

			fused_point.x = Median(&fused_points_x_);
			fused_point.y = Median(&fused_points_y_);
			fused_point.z = Median(&fused_points_z_);

			// fused_point.nx = fused_normal.x() / fused_normal_norm;
			// fused_point.ny = fused_normal.y() / fused_normal_norm;
			// fused_point.nz = fused_normal.z() / fused_normal_norm;

			fused_point.r = (std::round(Median(&fused_points_r_)));
			fused_point.g = (std::round(Median(&fused_points_g_)));
			fused_point.b = (std::round(Median(&fused_points_b_)));

			fused_points_.push_back(fused_point);
		}

	}

	void Reconstruction::GetMatched2D3DPoints(const std::unordered_map<int, cv::Point2f>& corners1,
		const std::unordered_map<int, cv::Point2f>& corners2,
		std::vector<cv::Point2f>& points1, std::vector<cv::Point3f>& points2,
		const cv::Mat& depth_mat1, const cv::Mat& depth_mat2) {
		for (auto pair2 : corners2) {

			auto pair1 = corners1.find(pair2.first);

			if (pair1 != corners1.end()) {
				// Pair found

				// Get Depth Data.	
				cv::Point2f p1 = pair1->second;
				cv::Point2f p2 = pair2.second;

				//std::cout << "Pt1 " << p1 << std::endl;
				//std::cout << "Pt2 " << p2 << std::endl;

				float d1 = depth_mat1.at<float>(p1.y / 2.0, p1.x / 2.0);
				float d2 = depth_mat2.at<float>(p2.y / 2.0, p2.x / 2.0);

				if (d1 != 0.0 && d2 != 0.0) {

					cv::Point2f point1; point1.x = int(p1.x / 2.0); point1.y = int(p1.y / 2.0);

					// cv::Mat pd1 = Point2dTo3d(point1);
					//cout << "Pd1 " << pd1 << endl;
					points1.push_back(point1);

					cv::Point3f point2; point2.x = int(p2.x / 2.0); point2.y = int(p2.y / 2.0); point2.z = d2;

					cv::Point3f pd2 = Point2dTo3d(point2);
					//cout << "Pd1 " << pd1 << endl;
					points2.push_back(pd2);
				}
			}
		}
	}

	void WritePlyText(const std::string &path,
		const std::vector<FusedPoint> &points,dgene::Workspace * workspace) {
		std::ofstream file(path);
		//CHECK(file.is_open()) << path;

		file << "ply" << std::endl;
		file << "format ascii 1.0" << std::endl;
		file << "element vertex " << points.size() << std::endl;
		file << "property float x" << std::endl;
		file << "property float y" << std::endl;
		file << "property float z" << std::endl;
		// file << "property float nx" << std::endl;
		// file << "property float ny" << std::endl;
		// file << "property float nz" << std::endl;
		file << "property uchar red" << std::endl;
		file << "property uchar green" << std::endl;
		file << "property uchar blue" << std::endl;
		file << "end_header" << std::endl;

        float X = 0;float Y = 0;float Z = 0;float count = 0;
        for (const auto &point : points) {
            X += point.x;
            Y += point.y;
            Z += point.z;
            count += 1.0f;
        }
        
        X = X / count; Y = Y / count; Z = Z / count;
        ofstream centerFile;
        centerFile.open(workspace->GetOutputDir()+"/center.txt");
        centerFile  << X << "\n"
        << Y << "\n"
        << Z << "\n";
        centerFile.close();

        X = 0; Y = 0; Z = 0;
		for (const auto &point : points) {
//            float cur = count;
//            float all = points.size();
            
            
			file << (point.x) << " " << (point.y) << " " << (point.z) << " "
				<< static_cast<unsigned int>(point.b) << " "
				<< static_cast<unsigned int>(point.g) << " "
				<< static_cast<unsigned int>(point.r) << std::endl;
		}

		file.close();
	}

	void WritePlyBinary(const std::string &path,
		const std::vector<FusedPoint> &points) {
		std::fstream text_file(path, std::ios::out);
		//CHECK(text_file.is_open()) << path;

		text_file << "ply" << std::endl;
		text_file << "format binary_little_endian 1.0" << std::endl;
		text_file << "element vertex " << points.size() << std::endl;
		text_file << "property float x" << std::endl;
		text_file << "property float y" << std::endl;
		text_file << "property float z" << std::endl;
		text_file << "property float nx" << std::endl;
		text_file << "property float ny" << std::endl;
		text_file << "property float nz" << std::endl;
		text_file << "property uchar red" << std::endl;
		text_file << "property uchar green" << std::endl;
		text_file << "property uchar blue" << std::endl;
		text_file << "end_header" << std::endl;
		text_file.close();

		std::fstream binary_file(path,
			std::ios::out | std::ios::binary | std::ios::app);
		//CHECK(binary_file.is_open()) << path;

		for (const auto &point : points) {
			// WriteBinaryLittleEndian<float>(&binary_file, point.x);
			// WriteBinaryLittleEndian<float>(&binary_file, point.y);
			// WriteBinaryLittleEndian<float>(&binary_file, point.z);
			// WriteBinaryLittleEndian<float>(&binary_file, point.nx);
			// WriteBinaryLittleEndian<float>(&binary_file, point.ny);
			// WriteBinaryLittleEndian<float>(&binary_file, point.nz);
			// WriteBinaryLittleEndian<uint8_t>(&binary_file, point.r);
			// WriteBinaryLittleEndian<uint8_t>(&binary_file, point.g);
			// WriteBinaryLittleEndian<uint8_t>(&binary_file, point.b);
		}
		binary_file.close();
	}

	cv::Matx34d ComposeProjectionMatrix(const cv::Mat &K, const cv::Mat &R,
		const cv::Mat &T) {
		cv::Mat p_mat = cv::Mat(3, 4, CV_64F);

		// cv::Rect(x, y, width, height)
		R.copyTo(p_mat(cv::Rect(0, 0, 3, 3)));
		T.copyTo(p_mat(cv::Rect(3, 0, 1, 3)));

		cv::Matx34d p_max;
		p_mat.convertTo(p_max, CV_64F);

		cv::Matx33d k_max;
		K.convertTo(k_max, CV_64F);

		p_max = k_max * p_max;

		return p_max;
	}

	cv::Matx34d ComposeInverseProjectionMatrix(const cv::Mat &K, const cv::Mat &R,
		const cv::Mat &T) {
		cv::Mat K_INV_MAT = K.inv();
		cv::Matx33d k_inv_max;
		K_INV_MAT.convertTo(k_inv_max, CV_64F);

		cv::Matx33d r_matrix;
		cv::Mat rt = R.t();
		rt.convertTo(r_matrix, CV_64F);

		cv::Vec3d t_matrix;
		t_matrix(0) = T.at<float>(0, 0);
		t_matrix(1) = T.at<float>(1, 0);
		t_matrix(2) = T.at<float>(2, 0);

		t_matrix = -r_matrix * t_matrix;

		cv::Mat P_inv_mat = cv::Mat(3, 4, CV_64F);
		cv::Mat r_k_inv_mat;
		r_k_inv_mat = rt * K_INV_MAT;
		r_k_inv_mat.copyTo(P_inv_mat(cv::Rect(0, 0, 3, 3)));
		P_inv_mat.at<double>(0, 3) = t_matrix(0);
		P_inv_mat.at<double>(1, 3) = t_matrix(1);
		P_inv_mat.at<double>(2, 3) = t_matrix(2);

		cv::Matx34d P_inv;
		P_inv_mat.convertTo(P_inv, CV_64F);
		return P_inv;
	}
}
