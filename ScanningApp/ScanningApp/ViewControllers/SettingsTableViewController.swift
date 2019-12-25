//
//  SettingsTableViewController.swift
//  ScanningApp
//
//  Created by Plex on 2018/9/14.
//  Copyright © 2018年 Dgene. All rights reserved.
//

import UIKit
import JGProgressHUD

class SettingsTableViewController: UITableViewController {

    @IBOutlet weak var txfMinDist: UITextField!
    @IBOutlet weak var txfMaxDist: UITextField!
    @IBOutlet weak var txfMaxImgCount: UITextField!
    @IBOutlet weak var txfInterval: UITextField!
    
    @IBAction func swMarkerChanged(_ sender: UISwitch) {
        Common.shared.useMarker = sender.isOn;
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        hideKeyboardWhenTappedAround()
        

        
        
        // Uncomment the following line to preserve selection between presentations
        // self.clearsSelectionOnViewWillAppear = false

        // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
        // self.navigationItem.rightBarButtonItem = self.editButtonItem
    }
    
    func saveOptions(){
        Common.shared.minDist = Float(txfMinDist.text!)!
        Common.shared.maxDist = Float(txfMaxDist.text!)!
        Common.shared.maxImgCount = Int(txfMaxImgCount.text!)!
        Common.shared.interval = Float(txfInterval.text!)!
        
        UserDefaults.standard.set(Common.shared.minDist, forKey: "minDist")
        UserDefaults.standard.set(Common.shared.maxDist, forKey: "maxDist")
        UserDefaults.standard.set(Common.shared.maxImgCount, forKey: "maxImgCount")
        UserDefaults.standard.set(Common.shared.interval, forKey: "interval")
        
        self.dismissKeyboard()
        
        DispatchQueue.global().async {
            usleep(250000)
            DispatchQueue.main.async {
                let hud = JGProgressHUD(style: .dark)
                hud.textLabel.text = "已保存"
                hud.indicatorView = JGProgressHUDSuccessIndicatorView()
                hud.show(in: self.view, animated: false)
                hud.dismiss(afterDelay: 1.0, animated: true)
            }
        }

    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        txfMinDist.text = String(Common.shared.minDist)
        txfMaxDist.text = String(Common.shared.maxDist)
        txfMaxImgCount.text = String(Common.shared.maxImgCount)
        txfInterval.text = String(Common.shared.interval)
    }
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
    }
    @IBAction func saveTapped(_ sender: UIButton) {
        saveOptions()
    }
    @IBAction func saveTapped2(_ sender: UIBarButtonItem) {
        saveOptions()
    }
    
    // MARK: - Table view data source

//    override func numberOfSections(in tableView: UITableView) -> Int {
//        // #warning Incomplete implementation, return the number of sections
//        return 0
//    }
//
//    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
//        // #warning Incomplete implementation, return the number of rows
//        return 0
//    }

    /*
    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "reuseIdentifier", for: indexPath)

        // Configure the cell...

        return cell
    }
    */

    /*
    // Override to support conditional editing of the table view.
    override func tableView(_ tableView: UITableView, canEditRowAt indexPath: IndexPath) -> Bool {
        // Return false if you do not want the specified item to be editable.
        return true
    }
    */

    /*
    // Override to support editing the table view.
    override func tableView(_ tableView: UITableView, commit editingStyle: UITableViewCellEditingStyle, forRowAt indexPath: IndexPath) {
        if editingStyle == .delete {
            // Delete the row from the data source
            tableView.deleteRows(at: [indexPath], with: .fade)
        } else if editingStyle == .insert {
            // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
        }    
    }
    */

    /*
    // Override to support rearranging the table view.
    override func tableView(_ tableView: UITableView, moveRowAt fromIndexPath: IndexPath, to: IndexPath) {

    }
    */

    /*
    // Override to support conditional rearranging of the table view.
    override func tableView(_ tableView: UITableView, canMoveRowAt indexPath: IndexPath) -> Bool {
        // Return false if you do not want the item to be re-orderable.
        return true
    }
    */

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

}
