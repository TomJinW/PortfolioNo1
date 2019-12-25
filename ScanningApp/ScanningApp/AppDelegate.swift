//
//  AppDelegate.swift
//  ScanningApp
//
//  Created by Plex on 2018/9/11.
//  Copyright © 2018年 Dgene. All rights reserved.
//

import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var customWindow: MyCustomWindow?
    var window: UIWindow?
//    {
//        get {
//            customWindow = customWindow ?? MyCustomWindow(frame: UIScreen.main.bounds)
//            return customWindow
//        }
//        set { }
//    }


    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        // Override point for customization after application launch.
        
        Common.shared.minDist = UserDefaults.standard.float(forKey: "minDist") != 0 ? UserDefaults.standard.float(forKey: "minDist") : 0.1
        Common.shared.maxDist = UserDefaults.standard.float(forKey: "maxDist") != 0 ? UserDefaults.standard.float(forKey: "maxDist") : 0.4
        Common.shared.maxImgCount = UserDefaults.standard.integer(forKey: "maxImgCount") != 0 ? UserDefaults.standard.integer(forKey: "maxImgCount") : 40
        Common.shared.interval = UserDefaults.standard.float(forKey: "interval") != 0 ? UserDefaults.standard.float(forKey: "interval") : 0.2
        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
        // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
    }

    func applicationDidEnterBackground(_ application: UIApplication) {
        // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
        // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    }

    func applicationWillEnterForeground(_ application: UIApplication) {
        // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    }

    func applicationWillTerminate(_ application: UIApplication) {
        // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    }


}

