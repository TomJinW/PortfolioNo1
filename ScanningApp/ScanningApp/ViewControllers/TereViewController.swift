//
//  TereViewController.swift
//  ScanningApp
//
//  Created by Plex on 2019/1/11.
//  Copyright © 2019 Dgene. All rights reserved.
//

import UIKit
import GLKit
import TereEngine
//import JGProgressHUD


class TereViewController: GLKViewController {

    var engine:TereEngine!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        

        
        
        
        // Do any additional setup after loading the view.
    }
//    let hud = JGProgressHUD(style: .dark)
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        
        tabBarController?.tabBar.items?[2].badgeValue = nil
//        DispatchQueue.global().async {
//            DispatchQueue.main.async {
//                self.hud.textLabel.text = "正在加载模型"
//                self.hud.show(in: self.view)
//            }
//            sleep(1)
//        }
//        sleep(2)
    }
    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)
        
        
        guard let documentsUrl =  FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first as URL? else {return}
        let meshURL = documentsUrl.appendingPathComponent("/Example/profile.txt")
        self.engine = TereEngine(self, dataPath: meshURL.path)
//        self.hud.dismiss()
        navigationItem.title = "2.8D渲染"
//        navigationController?.navigationItem.title = "2.8D渲染"
    }
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        // VERT IMPORTANT
        self.engine = nil
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        navigationItem.title = "正在加载"
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
