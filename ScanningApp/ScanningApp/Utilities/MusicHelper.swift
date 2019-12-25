//
//  MusicHelper.swift
//  ScanningApp
//
//  Created by Plex on 2019/2/19.
//  Copyright © 2019 Dgene. All rights reserved.
//

// This is used for allowing camera control using mic.
import Foundation
import AVFoundation

class MusicHelper {
    static let shared = MusicHelper()
    var audioPlayer: AVAudioPlayer?
    func playBackgroundMusic() {
        
        let docURL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).last! as URL
        let audioPath = docURL.appendingPathComponent("spice.m4a")
//        let aSound = URL(fileURLWithPath: Bundle.main.path(forResource: "spice", ofType: "m4a")!)
//        print(aSound)
        do {
            audioPlayer = try AVAudioPlayer(contentsOf:audioPath)
            audioPlayer!.numberOfLoops = -1
            audioPlayer!.prepareToPlay()
            audioPlayer!.play()
        } catch {
            print("Cannot play the file")
        }
    }
}
