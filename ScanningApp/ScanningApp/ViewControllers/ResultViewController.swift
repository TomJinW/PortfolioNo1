//
//  ResultViewController.swift
//  ScanningApp
//
//  Created by Plex on 2018/9/13.
//  Copyright © 2018年 Dgene. All rights reserved.
//

import UIKit
import SceneKit
import JGProgressHUD

class ResultViewController: UIViewController {

    @IBOutlet weak var scnView: SCNView!
    var originalScale:Float = 0
    var panStatus = 0
    
    func rotateGesture(_ translate:CGPoint){
        if let geometryNode = (scnView.scene?.rootNode) {
            let translation = translate
            let x = Float(translation.x)
            let y = Float(-translation.y)
//            let y:Float = 0
            let anglePan = sqrt(pow(x,2)+pow(y,2))*(Float)(Double.pi)/180.0
            var rotationVector = SCNVector4()
            rotationVector.x = -y
            rotationVector.y = x
            rotationVector.z = 0
            rotationVector.w = anglePan
            geometryNode.rotation = rotationVector
        }
    }
    
    func loadModel(){

        
        guard let documentsUrl =  FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first as URL? else {return}
        
        let meshURL = documentsUrl.appendingPathComponent("/Example/Mesh.ply")
        let meshInvertURL = documentsUrl.appendingPathComponent("/Example/MeshInvert.ply")
        
        guard let meshScene = try? SCNScene(url: meshURL, options: nil) else{ return }
        guard let meshInvertScene = try? SCNScene(url: meshInvertURL, options: nil) else{ return }
        
        
        var centerURL:URL =  FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first as URL!
        centerURL = centerURL.appendingPathComponent("/Example/center.txt")
        guard let data = try? String(contentsOfFile: centerURL.path, encoding: .utf8) else {return}
        let myStrings = data.components(separatedBy: .newlines)
        let (x,y,z) =  (Float(myStrings[0])!,Float(myStrings[1])!,Float(myStrings[2])!)
        

        let light = SCNLight()
        let lightNode = SCNNode()
        light.type = .ambient
        light.color = UIColor.white
        lightNode.light = light
        lightNode.position = SCNVector3(-40, 40, 60)
        meshScene.rootNode.addChildNode(lightNode)
        meshScene.rootNode.addChildNode(meshInvertScene.rootNode)
        
        scnView.scene = meshScene;
        scnView.scene?.rootNode.pivot = SCNMatrix4MakeTranslation(x,y,z)
        scnView.scene?.rootNode.scale = SCNVector3(x: -1.0, y: -1.0, z: 1.0)
        


    }
    override func viewDidLoad() {
        super.viewDidLoad()

        // Do any additional setup after loading the view.
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        self.tabBarController?.tabBar.items?[1].badgeValue = nil
        
    }
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
//        for node in scnView.scene!.rootNode.childNodes{
//            node.removeFromParentNode()
//        }
//        scnView.reloadInputViews()
    }
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        let hud = JGProgressHUD(style: .dark)
        hud.textLabel.text = "正在加载模型"
        hud.show(in: self.view)
        
        DispatchQueue.global().async {
            self.loadModel()
            DispatchQueue.main.async {
                hud.dismiss()
            }
        }

        
    }

    
    @IBAction func panningAround(_ sender: UIPanGestureRecognizer) {
        switch sender.state {
        case .began:
            let translation = sender.translation(in: scnView)
            let y = Float(translation.y)
            let x = Float(translation.x)
            if abs(y) > abs(x) {
                panStatus = 2
            }else{
                panStatus = 1
            }
            
        case .changed:
            if panStatus == 1 {
                rotateGesture(sender.translation(in: scnView))
            }else{
                rotateGesture(sender.translation(in: scnView))
//                if let geometryNode = (scnView.scene?.rootNode){
//                    let translation = sender.translation(in: scnView)
//                    let y = Float(translation.y)
//                    geometryNode.position = SCNVector3(x:0,y:-y*0.001,z:0)
//                }
            }
        case .ended:
            if let geometryNode = (scnView.scene?.rootNode){
                let currentPivot = geometryNode.pivot
                let changePivot = SCNMatrix4Invert( geometryNode.transform)
                geometryNode.pivot = SCNMatrix4Mult(changePivot, currentPivot)
                geometryNode.transform = SCNMatrix4Identity
            }
        default: break;
        }
    }
    
    @IBAction func pinchGesture(_ sender: UIPinchGestureRecognizer) {
        
        if sender.state == .changed{
            let node = scnView.scene!.rootNode
            let pinchScaleX = Float((sender.scale)) * node.scale.x
            let pinchScaleY = Float((sender.scale)) * node.scale.y
            let pinchScaleZ = Float((sender.scale)) * node.scale.z
            node.scale = SCNVector3(x: pinchScaleX, y: pinchScaleY, z: pinchScaleZ)
            sender.scale = 1
        }
        
    }
    
    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

}
