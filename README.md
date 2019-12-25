# Portfolio No.1: An iOS App using Swift, Objective-C and C++

## Demo Showcase

- [Video showing the result data on Youtube](https://youtu.be/wefZeN5HUUE) 

- The interface about the App. 

	![avatar](https://github.com/TomJinW/PortfolioImages/blob/master/1/img/img001.jpg?raw=true)
		
	![avatar](https://github.com/TomJinW/PortfolioImages/blob/master/1/img/img002.jpg?raw=true)

## The Main Purpose of the App

This partcular iOS App utilize the True Depth camera of the iPhone X to caputure the depth-map image of an object and finally generates 3D models of it with the following steps. It's a laboratory project when I was senior.

## The Main Procedures of the App
1. Take pictures with depth-map data. This is done by ARKit using Swift. Then pass the data through by programming a simple bridging Objective-C header so that the C++ code can recongnize it.
2. compute the point cloud position in real 3D world by using the intrinsics of the iPhone X camera.
3. Use OpenCV to detect feature points of each photo and match them together using algorithm like SIFT for SURF.
4. Combine the information of 2D feature points with their 3D location. Then use PnP algorithm to calculate the extrinsics of each camera using a pair of images.
5. Use the extrinsics to combine all of the point cloud together and remove some points for the time reduction of the algorithm. Then write out the .ply file of point cloud file.
6. Use the .ply file just generated and 3rd party library to achieve poisson face reconstruction which will turn point clouds into a 3D mesh,and clip the unnecessary faces,then finally generate the mesh ply for the App to share using Action Sheet and viewed by SCNView.

## Things I Have Done for This Project

- The App's interface and logic was designed by me using the Interface Builder integrated into the Xcode. 
- The App's behavior was programmed by me with all of the Swift code as well as the Objective-C bridging header used to communicate with the C++ part.
- Most of the process in step 2-6 was programmed by me using some 3rd party library.

- Files Programmed by Me (Swift/Objective-C Part):
	
	./ScanningApp/*
		
	./ScanningApp/ScanningApp/AppDelegate.swift

	./ScanningApp/ScanningApp/Common.swift

	./ScanningApp/ScanningApp/ProgressHUD.swift

	./ScanningApp/ScanningApp/Wrapper.h

	./ScanningApp/ScanningApp/Wrapper.mm

	./ScanningApp/ScanningApp/Utilities/Extension.swift

	./ScanningApp/ScanningApp/Utilities/MusicHelper.swift

	./ScanningApp/ScanningApp/Utilities/MyCustomWindow.swift

	./ScanningApp/ScanningApp/ViewControllers/ResultViewController.swift

	./ScanningApp/ScanningApp/ViewControllers/SettingsTableViewController.swift

	./ScanningApp/ScanningApp/ViewControllers/TereViewController.swift

	./ScanningApp/ScanningApp/ViewControllers/ViewController.swift

- Files Programmed by Me (C++ Part):

	./scanningLib/src/reconstruction.h

	./scanningLib/src/reconstruction.cc

	./scanningLib/src/scanning.h

	./scanningLib/src/scanning.mm

	./scanningLib/src/workspace.cc

	./scanningLib/src/workspace.h

	./scanningLib/src/mesh/compute_normal.cc

	./scanningLib/src/mesh/compute_normal.h
	
## 3rd Party Libraries and Files I used

- The icon assets are from Google.
- [OpenCV][https://github.com/opencv/opencv] 
- JGProgressHUD: https://github.com/JonasGessner/JGProgressHUD
- Files which belongs to the 3rd party libraries:

	./scanningLib/include/Eigen/*

	./scanningLib/scanningLib/src/common/*

	./scanningLib/scanningLib/src/ext/*

	./scanningLib/scanningLib/src/mesh/plylib.cc

	./scanningLib/scanningLib/src/mesh/plylib.h

	./scanningLib/scanningLib/src/mesh/mesh.h
