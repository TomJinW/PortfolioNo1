#include "mesh/compute_normal.h"
#include "common/Log.hpp"

enum MeshElement {
	MM_NONE = 0x00000000,
	MM_VERTCOORD = 0x00000001,
	MM_VERTNORMAL = 0x00000002,
	MM_VERTFLAG = 0x00000004,
	MM_VERTCOLOR = 0x00000008,
	MM_VERTQUALITY = 0x00000010,
	MM_VERTMARK = 0x00000020,
	MM_VERTFACETOPO = 0x00000040,
	MM_VERTCURV = 0x00000080,
	MM_VERTCURVDIR = 0x00000100,
	MM_VERTRADIUS = 0x00000200,
	MM_VERTTEXCOORD = 0x00000400,
	MM_VERTNUMBER = 0x00000800,

	MM_FACEVERT = 0x00001000,
	MM_FACENORMAL = 0x00002000,
	MM_FACEFLAG = 0x00004000,
	MM_FACECOLOR = 0x00008000,
	MM_FACEQUALITY = 0x00010000,
	MM_FACEMARK = 0x00020000,
	MM_FACEFACETOPO = 0x00040000,
	MM_FACENUMBER = 0x00080000,

	MM_WEDGTEXCOORD = 0x00100000,
	MM_WEDGNORMAL = 0x00200000,
	MM_WEDGCOLOR = 0x00400000,

	// 	Selection
	MM_VERTFLAGSELECT = 0x00800000,
	MM_FACEFLAGSELECT = 0x01000000,

	// Per Mesh Stuff....
	MM_CAMERA = 0x08000000,
	MM_TRANSFMATRIX = 0x10000000,
	MM_COLOR = 0x20000000,
	MM_POLYGONAL = 0x40000000,
	MM_UNKNOWN = 0x80000000,

	MM_ALL = 0xffffffff
};

void updateDataMask(CMeshO& cm, int neededDataMask) {
	if ((neededDataMask & MM_FACEFACETOPO) != 0) {
		cm.face.EnableFFAdjacency();
		tri::UpdateTopology<CMeshO>::FaceFace(cm);
	}
	if ((neededDataMask & MM_VERTFACETOPO) != 0) {
		cm.vert.EnableVFAdjacency();
		cm.face.EnableVFAdjacency();

		tri::UpdateTopology<CMeshO>::VertexFace(cm);
	}

	if (((neededDataMask & MM_WEDGTEXCOORD) != 0)) cm.face.EnableWedgeTexCoord();
	if (((neededDataMask & MM_FACECOLOR) != 0)) cm.face.EnableColor();
	if (((neededDataMask & MM_FACEQUALITY) != 0)) cm.face.EnableQuality();
	if (((neededDataMask & MM_FACEMARK) != 0)) cm.face.EnableMark();
	if (((neededDataMask & MM_VERTMARK) != 0)) cm.vert.EnableMark();
	if (((neededDataMask & MM_VERTCURV) != 0)) cm.vert.EnableCurvature();
	if (((neededDataMask & MM_VERTCURVDIR) != 0)) cm.vert.EnableCurvatureDir();
	if (((neededDataMask & MM_VERTRADIUS) != 0)) cm.vert.EnableRadius();
	if (((neededDataMask & MM_VERTTEXCOORD) != 0)) cm.vert.EnableTexCoord();
}

bool updateMask(CMeshO& cm, int mask) {
	if (mask & tri::io::Mask::IOM_VERTTEXCOORD)
		updateDataMask(cm, MM_VERTTEXCOORD);

	if (mask & tri::io::Mask::IOM_WEDGTEXCOORD)
		updateDataMask(cm, MM_WEDGTEXCOORD);
	if (mask & tri::io::Mask::IOM_VERTCOLOR) updateDataMask(cm, MM_VERTCOLOR);
	if (mask & tri::io::Mask::IOM_FACECOLOR) updateDataMask(cm, MM_FACECOLOR);
	if (mask & tri::io::Mask::IOM_VERTRADIUS) updateDataMask(cm, MM_VERTRADIUS);
	if (mask & tri::io::Mask::IOM_CAMERA) updateDataMask(cm, MM_CAMERA);
	if (mask & tri::io::Mask::IOM_VERTQUALITY) updateDataMask(cm, MM_VERTQUALITY);
	if (mask & tri::io::Mask::IOM_FACEQUALITY) updateDataMask(cm, MM_FACEQUALITY);
	if (mask & tri::io::Mask::IOM_BITPOLYGONAL) updateDataMask(cm, MM_POLYGONAL);
	return true;
}

void ComputePCNormal(const std::string& input_name, const std::string& output_name) {
	CMeshO mesh;
	tri::io::PlyInfo pi;
	

	int plymask;
	tri::io::ImporterPLY<CMeshO>::LoadMask(input_name.c_str(), plymask);


	// small patch to allow the loading of per wedge color into faces.
	if (plymask & tri::io::Mask::IOM_WEDGCOLOR)
		plymask |= tri::io::Mask::IOM_FACECOLOR;
	// plymask |= tri::io::Mask::IOM_VERTNORMAL;
	updateMask(mesh, plymask);
	mesh.face.EnableMark();
	mesh.vert.EnableMark();
	mesh.face.EnableWedgeTexCoord();

	int result =
		tri::io::ImporterPLY<CMeshO>::Open(mesh, input_name.c_str(), plymask);
	if (result != 0)  // all the importers return 0 on success
	{
		if (tri::io::ImporterPLY<CMeshO>::ErrorCritical(result)) {
			if (tri::io::ImporterPLY<CMeshO>::ErrorCritical(result)) {
				//cerr << "Load file error: " << input_name << ", "
				//	<< tri::io::ImporterPLY<CMeshO>::ErrorMsg(result) << flush;
				LOGW("Load file error: %s, %s\n", input_name.c_str(),
					tri::io::ImporterPLY<CMeshO>::ErrorMsg(result));
				exit(0);
			}
		}
	}



	vcg::tri::Clean<CMeshO>::RemoveDuplicateVertex(mesh);
	vcg::tri::Clean<CMeshO>::RemoveDegenerateVertex(mesh);

	tri::PointCloudNormal<CMeshO>::Param p;
	p.fittingAdjNum = 15;
	p.smoothingIterNum = 0;
	p.viewPoint = Point3f(0, 0, 0);
	p.useViewPoint = false;
	tri::PointCloudNormal<CMeshO>::Compute(mesh, p);

	tri::UpdateBounding<CMeshO>::Box(mesh);
	plymask |= tri::io::Mask::IOM_VERTNORMAL;
	updateMask(mesh, plymask);


	PoissonClean(mesh, false, true);
	tri::io::ExporterPLY<CMeshO>::Save(mesh, output_name.c_str(), plymask, true);
}

// 判断点在平面的哪侧
bool ifOnPlaneSide(float * plane,vcg::Point3f &point, float offset){
	float d = plane[0] * point.X() + plane[1] * point.Y() + plane[2] * point.Z() +plane[3];
	if (d < offset){
		return true;
	}
	return false;
}

void UpdateBoxAndNormals(CMeshO & mesh){
	tri::UpdateBounding<CMeshO>::Box(mesh);
	if(mesh.fn>0) {
		tri::UpdateNormal<CMeshO>::PerFaceNormalized(mesh);
		tri::UpdateNormal<CMeshO>::PerVertexAngleWeighted(mesh);
	}
}

void smoothMesh(const std::string& input_name, const std::string& output_name,bool binary){
	// 初始化并读取图片
	CMeshO m;
	tri::io::PlyInfo pi;
	int plymask;
	tri::io::ImporterPLY<CMeshO>::LoadMask(input_name.c_str(), plymask);
	
	// small patch to allow the loading of per wedge color into faces.
	if (plymask & tri::io::Mask::IOM_WEDGCOLOR)
		plymask |= tri::io::Mask::IOM_FACECOLOR;
	// plymask |= tri::io::Mask::IOM_VERTNORMAL;
	updateMask(m, plymask);
	m.face.EnableMark();
	m.vert.EnableMark();
	m.face.EnableWedgeTexCoord();
	
	if (tri::io::ImporterPLY<CMeshO>::Open(m,input_name.c_str(),plymask) != 0){
		cout << "READ FAIL" << endl;
	}

	cout << "SMOOTHING" << endl;
	for (int i = 0; i <= 5; i++){
		tri::UpdateFlags<CMeshO>::FaceBorderFromNone(m);
		tri::Smooth<CMeshO>::VertexCoordLaplacianHC(m,1,false);
		UpdateBoxAndNormals(m);
	}

	tri::Clean<CMeshO>::FlipNormalOutside(m);
	UpdateBoxAndNormals(m);
	tri::io::ExporterPLY<CMeshO>::Save(m, output_name.c_str(),plymask, binary);
}

void invertMeshNormal(const std::string& input_name, const std::string& output_name,bool binary){
	// 初始化并读取图片
	CMeshO m;
	tri::io::PlyInfo pi;
	int plymask;
	tri::io::ImporterPLY<CMeshO>::LoadMask(input_name.c_str(), plymask);
	
	// small patch to allow the loading of per wedge color into faces.
	if (plymask & tri::io::Mask::IOM_WEDGCOLOR)
		plymask |= tri::io::Mask::IOM_FACECOLOR;
	// plymask |= tri::io::Mask::IOM_VERTNORMAL;
	updateMask(m, plymask);
	m.face.EnableMark();
	m.vert.EnableMark();
	m.face.EnableWedgeTexCoord();
	
	if (tri::io::ImporterPLY<CMeshO>::Open(m,input_name.c_str(),plymask) != 0){
		cout << "READ FAIL" << endl;
	}

	tri::Clean<CMeshO>::FlipNormalOutside(m);
	UpdateBoxAndNormals(m);
	tri::io::ExporterPLY<CMeshO>::Save(m, output_name.c_str(),plymask, binary);
}

// 剪裁平面
void cutFaces(const std::string& input_name, const std::string& output_name,float * plane,bool useMarker){

	// 初始化并读取图片
	CMeshO m;
	tri::io::PlyInfo pi;
	int plymask;
	tri::io::ImporterPLY<CMeshO>::LoadMask(input_name.c_str(), plymask);
	
	// small patch to allow the loading of per wedge color into faces.
	if (plymask & tri::io::Mask::IOM_WEDGCOLOR)
		plymask |= tri::io::Mask::IOM_FACECOLOR;
	// plymask |= tri::io::Mask::IOM_VERTNORMAL;
	updateMask(m, plymask);
	m.face.EnableMark();
	m.vert.EnableMark();
	m.face.EnableWedgeTexCoord();
	
	if (tri::io::ImporterPLY<CMeshO>::Open(m,input_name.c_str(),plymask) != 0){
		cout << "READ FAIL" << endl;
	}

	// 初始化 Faces
	if (useMarker){
		CMeshO::FaceIterator fi;
    	for(fi=m.face.begin();fi!=m.face.end();++fi){
		
		bool keeped = true;
		// 遍历 Mesh 中的所有 Faces
		for (int i = 0; i < 3;i++){
			// Face 里的三个点中只要有一个点不在上方就会被删除
        	auto vertex = *(*fi).V(i);
        	auto point = vertex.P();
			keeped = keeped && ifOnPlaneSide(plane,point,-0.0015);
		}
		if (!keeped){
			// 删除平面
			tri::Allocator<CMeshO>::DeleteFace(m, *fi);
		}
    }
	}

	


	try{
		cout << "CLEANING" << endl;
		m.face.EnableFFAdjacency();
		vcg::tri::UpdateTopology<CMeshO>::FaceFace(m);
		tri::Clean<CMeshO>::RemoveSmallConnectedComponentsDiameter(m,0.10);
		//tri::Clean<CMeshO>::RemoveSmallConnectedComponentsSize(m,4000.0);
	}catch (vcg::MissingComponentException e){
		LOGI("ERROR ");
		LOGI(e.what());
		LOGI("\n");
	}
	
	int xxx = tri::Clean<CMeshO>::RemoveUnreferencedVertex(m);
	if (xxx != 0) {
		UpdateBoxAndNormals(m);
	}

	cout << "SMOOTHING" << endl;

	for (int i = 0; i <= 5; i++){
		tri::UpdateFlags<CMeshO>::FaceBorderFromNone(m);
		tri::Smooth<CMeshO>::VertexCoordLaplacianHC(m,1,false);
		UpdateBoxAndNormals(m);
	}

	int delvert = tri::Clean<CMeshO>::RemoveUnreferencedVertex(m);
	if (delvert != 0) {
		UpdateBoxAndNormals(m);
	}

	tri::Clean<CMeshO>::FlipNormalOutside(m);
	UpdateBoxAndNormals(m);
	tri::io::ExporterPLY<CMeshO>::Save(m, output_name.c_str(),plymask, true);
}