//
//  ViewController.swift
//  ScanningApp
//
//  Created by Plex on 2018/9/11.
//  Copyright © 2018年 Dgene. All rights reserved.
//

import UIKit
import SceneKit
import ARKit

import JGProgressHUD

extension Date {
    var ticks: UInt64 {
        return UInt64((self.timeIntervalSince1970 + 62_135_596_800) * 10_000_000)
    }
}

class ViewController: UIViewController, ARSCNViewDelegate,ARSessionDelegate,AVAudioPlayerDelegate {
    
    
    @IBOutlet var sceneView: ARSCNView!
    var captureTapped = false
    var record = false
    var imgCount = 0
    var scale:CGFloat = 0.0
    var docURL,setURL,imgURL,depthURL: URL!
    var serialCaptureQueue:DispatchQueue = DispatchQueue(label: "queue")
    var timeStamp = ""
    
    func mkDir(){
        // Create Directory
        timeStamp = String(Date().ticks)
//        timeStamp = "Test/"
//        timeStamp = "Example/"
        docURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).last! as URL
        setURL = docURL.appendingPathComponent(timeStamp)
        imgURL = setURL.appendingPathComponent("img")
        depthURL = setURL.appendingPathComponent("depth")
        try? FileManager.default.createDirectory(at: setURL, withIntermediateDirectories: true, attributes: nil)
        try? FileManager.default.createDirectory(at: imgURL, withIntermediateDirectories: true, attributes: nil)
        try? FileManager.default.createDirectory(at: depthURL, withIntermediateDirectories: true, attributes: nil)
    }
    
    @IBAction func viewTapped(_ sender: UITapGestureRecognizer) {
        if record{
            record = false;
        }else{
            captureTapped = true
        }
    }
    
    @IBAction func longPress(_ sender: UILongPressGestureRecognizer) {
        if !record{
            record = true;
        }
    }
    
    @IBAction func clearTapped(_ sender: UIBarButtonItem) {

        mkDir()
        imgCount = 0
        navigationItem.title = "拍摄"
        let hud = JGProgressHUD(style: .dark)
        hud.textLabel.text = "已清除拍摄照片"
        hud.indicatorView = JGProgressHUDSuccessIndicatorView()
        hud.show(in: self.view, animated: false)
        hud.dismiss(afterDelay: 2)
    }
    
    func calStart(){
        UIApplication.shared.beginIgnoringInteractionEvents()
        let insArray:[Float] = [478.98422,0,179.74148,0,478.98422,319.31653,0,0,1]
        let intrinsics = UnsafeMutablePointer(mutating: insArray)
        var fininshed = false
        print(setURL.path)
        
        let hud = JGProgressHUD(style: .dark)
        hud.textLabel.text = "正在重建中，这需要一些时间\n0% 完成"
        hud.indicatorView = JGProgressHUDPieIndicatorView()
        
        DispatchQueue.global().async {
            while (!fininshed){
                let process = Wrapper.getProcess()
                DispatchQueue.main.async {
                    hud.progress = process
                    let percentage:Int = Int(process * 100)
                    if process != -1 {
                        hud.textLabel.text = "正在重建中，这需要一些时间\n\(percentage)% 完成"
                    }
                }
                usleep(10000)
            }
        }
        
        hud.show(in: self.view)
        
        
        DispatchQueue.global().async {
            Wrapper.scan(self.setURL.path, intrinsics, 720, 1280, 2.0, Int32(self.imgCount),true);
            //            Wrapper.scan(self.setURL.path, intrinsics, 720, 1280, 2.0, Int32(45),true);
            fininshed = true
            DispatchQueue.main.async {
                self.tabBarController?.tabBar.items?[1].badgeValue = "1"
                self.tabBarController?.tabBar.items?[2].badgeValue = "1"
                hud.textLabel.text = "重建完成"
                hud.indicatorView = JGProgressHUDSuccessIndicatorView()
                hud.dismiss(afterDelay: 1.5)
                
            }
            sleep(2)
            DispatchQueue.main.async {
                UIApplication.shared.endIgnoringInteractionEvents()
                self.tabBarController?.selectedIndex = 1
            }
        }
    }
    @IBAction func calTapped(_ sender: UIBarButtonItem) {
       calStart()
    }
    

    var audioPlayer:AVAudioPlayer?;
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        UIApplication.shared.beginReceivingRemoteControlEvents()
        self.becomeFirstResponder()
        
        MusicHelper.shared.playBackgroundMusic()
        
//        docURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).last! as URL
//        let audioPath = docURL.appendingPathComponent("spice.m4a")
//        audioPlayer = try? AVAudioPlayer(contentsOf: audioPath)
//        audioPlayer!.delegate = self
//        audioPlayer!.prepareToPlay()
//        audioPlayer!.play()
        

        
    }
    override func viewDidLoad() {
        super.viewDidLoad()
        mkDir();

        scale = UIScreen.main.scale;

        // Set the view's delegate
        sceneView.delegate = self
        sceneView.session.delegate = self
        
        // Show statistics such as fps and timing information
        sceneView.showsStatistics = true
        sceneView.preferredFramesPerSecond = 30
        
        guard ARFaceTrackingConfiguration.isSupported else {
            print("not iPhone X.")
            return
        }
        
        // Create a session configuration
        let configuration = ARFaceTrackingConfiguration()
        // Run the view's session
        sceneView.session.run(configuration)
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        let configuration = ARFaceTrackingConfiguration()
        sceneView.session.run(configuration)
        
        UIApplication.shared.endReceivingRemoteControlEvents()
        self.resignFirstResponder()
    }
    
    override func remoteControlReceived(with event: UIEvent?){
        captureTapped = true
    }
    
    override func viewWillDisappear(_ animated: Bool) {
        
        super.viewWillDisappear(animated)
        // Pause the view's session
        sceneView.session.pause()
    }

    func saveSinglePicture(frame:ARFrame,DepthPixelBuf:CVPixelBuffer){
        
        let intrinsics = frame.camera.intrinsics
        print(intrinsics)
        
        let deviceID = String(UIDevice.current.modelName.split(separator: ",")[0])
        
        if (deviceID == "iPhone10"){
            // X
            var testArray = DepthPixelBuf.depthArray()
            let data = NSData(bytes: &testArray, length: 640*360*4)
            data.write(to: depthURL.appendingPathComponent("\(self.imgCount).depth"), atomically: true)
            
//            writeDepthCSV(depthData: DepthPixelBuf.depthArray(), width: 360, height: 640, filename: self.timeStamp + "/depth/D\(self.imgCount).csv",minD:Common.shared.minDist,maxD:Common.shared.maxDist)
            frame.capturedImage.copy()!.image().resizeImage(size: CGSize(width: CGFloat(1279/self.scale), height: CGFloat(719/self.scale))).rotated(by: Measurement(value: 90.0, unit: .degrees),options: [.flipOnHorizontalAxis])!.saveImageToDocumentDirectory(name: self.timeStamp + "/img/\(self.imgCount)", type: .jpg)
        }else{
            // XS XR XS Max
            var testArray = DepthPixelBuf.depthArray()
            let data = NSData(bytes: &testArray, length: 640*480*4)
            data.write(to: depthURL.appendingPathComponent("\(self.imgCount).depth"), atomically: true)
            
//            writeDepthCSV(depthData: DepthPixelBuf.depthArray(), width: 480, height: 640, filename: self.timeStamp + "/depth/D\(self.imgCount).csv",minD:Common.shared.minDist,maxD:Common.shared.maxDist)
            frame.capturedImage.copy()!.image().resizeImage(size: CGSize(width: CGFloat(1439/self.scale), height: CGFloat(1079/self.scale))).rotated(by: Measurement(value: 90.0, unit: .degrees),options: [.flipOnHorizontalAxis])!.saveImageToDocumentDirectory(name: self.timeStamp + "/img/\(self.imgCount)", type: .jpg)
        }

        

        self.imgCount += 1
        DispatchQueue.main.async {
            self.navigationItem.title = "\(self.imgCount) 张已拍摄"
        }
    }
    func session(_ session: ARSession, didUpdate frame: ARFrame) {
        
        if (!captureTapped && !record){return}
        
        guard let DepthPixelBuf = frame.capturedDepthData?.depthDataMap.copy() else {return}
        
        
        if (captureTapped){
            // Vibrate
            self.captureTapped = false
            let impact = UIImpactFeedbackGenerator()
//            impact.impactOccurred()
            sceneView.flash(numberOfFlashes: 1.0)
            if #available(iOS 9.0, *) {
                AudioServicesPlaySystemSoundWithCompletion(SystemSoundID(1108), nil)
            } else {
                AudioServicesPlaySystemSound(1108)
            }
            
            // Save File
            serialCaptureQueue.async {
                self.saveSinglePicture(frame: frame, DepthPixelBuf: DepthPixelBuf)
            }
        }else{
            if (self.imgCount >= Common.shared.maxImgCount){
                record = false
//                DispatchQueue.main.async {
//                    self.calStart()
//                }
                return
            }
            self.serialCaptureQueue.async {
                autoreleasepool{
                    if (self.imgCount >= Common.shared.maxImgCount){
                        self.record = false
                        return
                    }
                    self.saveSinglePicture(frame: frame, DepthPixelBuf: DepthPixelBuf)
                    usleep(useconds_t(Common.shared.interval*1000.0))
                }
            }
        }
        
        
        


    }
    
    
    // MARK: - ARSCNViewDelegate
    
/*
    // Override to create and configure nodes for anchors added to the view's session.
    func renderer(_ renderer: SCNSceneRenderer, nodeFor anchor: ARAnchor) -> SCNNode? {
        let node = SCNNode()
     
        return node
    }
*/
    
    func session(_ session: ARSession, didFailWithError error: Error) {
        // Present an error message to the user
        
    }
    
    func sessionWasInterrupted(_ session: ARSession) {
        // Inform the user that the session has been interrupted, for example, by presenting an overlay
        
    }
    
    func sessionInterruptionEnded(_ session: ARSession) {
        // Reset tracking and/or remove existing anchors if consistent tracking is required
        
    }
    
    func renderer(_ renderer: SCNSceneRenderer, didAdd node: SCNNode, for anchor: ARAnchor) {
        
    }
}
