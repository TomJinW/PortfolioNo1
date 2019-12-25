//
// Created by zhang on 8/9/2018.
//
#import <vector>
#import "reconstruction.h"
#import "workspace.h"
#import "scanning.h"


//int main(int argc, char** argv) {
//    std::string data_path = "/Data/AndroidData/";
//
//    dgene::Workspace workspace(data_path);
//    workspace.ReadData(36);
//
//    dgene::ReconstructionOptions options;
//
//    dgene::Reconstruction recon(options, &workspace);
//
//    recon.Run();
//
//    return 0;
//}




dgene::Reconstruction * recon;

@implementation Scanning

+ (bool) scan: (NSString * _Nonnull) dataPath : (float * _Nonnull) intrinsics :
(int) width : (int) height : (float) scale : (int) imgCount : (bool) useMarker{
    std::string data_path = [dataPath UTF8String];
    dgene::Workspace workspace(data_path);
    workspace.ReadData(imgCount);
    dgene::ReconstructionOptions options;
    
    recon = new dgene::Reconstruction(options, &workspace);
    if (recon != NULL){
        recon->Run();
    }
    
    if (recon != NULL){
        delete recon;
        recon = NULL;
    }
    
    return true;
}

+ (float) getProcess{
    if (recon != NULL){
        return recon->process;
    }
    return -1;
}


@end
