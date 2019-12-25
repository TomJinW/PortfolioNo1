//
//  MyCustomWindow.swift
//  ScanningApp
//
//  Created by Plex on 2019/2/15.
//  Copyright © 2019 Dgene. All rights reserved.
//

import UIKit

class MyCustomWindow: UIWindow {

    override func sendEvent(_ event: UIEvent) {
        if event.type == .remoteControl{
            print("Remove")
            exit(0)
        }
        else{
            super.sendEvent(event)
        }
    }
    /*
    // Only override draw() if you perform custom drawing.
    // An empty implementation adversely affects performance during animation.
    override func draw(_ rect: CGRect) {
        // Drawing code
    }
    */
    

}
