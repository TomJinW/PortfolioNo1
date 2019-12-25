//
//  Common.swift
//  newScan
//
//  Created by Plex on 2018/7/6.
//  Copyright © 2018年 Plex. All rights reserved.
//

import Foundation
import UIKit

enum PictureType:Int{
    case jpg
    case png
}

extension UIViewController {
    func hideKeyboardWhenTappedAround() {
        let tap: UITapGestureRecognizer = UITapGestureRecognizer(target: self, action: #selector(UIViewController.dismissKeyboard))
        tap.cancelsTouchesInView = false
        view.addGestureRecognizer(tap)
    }
    
    @objc func dismissKeyboard() {
        view.endEditing(true)
    }
}

extension CVPixelBuffer{
    func image()->UIImage {
        let ciImage = CIImage(cvPixelBuffer: self)
        let temporaryContext = CIContext(options: nil)
        let videoImage = temporaryContext.createCGImage(ciImage, from: CGRect(x: 0, y: 0, width: CVPixelBufferGetWidth(self), height: CVPixelBufferGetHeight(self)))
        let image = UIImage(cgImage: videoImage!)
        return image
    }
    
    func depthArray()->[Float]{
        let w = CVPixelBufferGetWidth(self)
        let h = CVPixelBufferGetHeight(self)
        
        
        CVPixelBufferLockBaseAddress(self, CVPixelBufferLockFlags(rawValue: 0))
        let depthPointer = unsafeBitCast(CVPixelBufferGetBaseAddress(self), to: UnsafeMutablePointer<Float32>.self)
        var depthData = Array<Float>(repeating: 0.0,count:w*h);
        for v in 0..<w{ // height
            for u in 0..<h{ // width
                let count = v * h + (u);
                let frCount = u * w + (v);
                
                if !depthPointer[frCount].isNaN{
                    depthData[count] = depthPointer[frCount]
                }
            }
        }
        return depthData
    }
    
    func copy() -> CVPixelBuffer? {
        let width = CVPixelBufferGetWidth(self)
        let height = CVPixelBufferGetHeight(self)
        let format = CVPixelBufferGetPixelFormatType(self)
        var pixelBufferCopyOptional:CVPixelBuffer?
        CVPixelBufferCreate(nil, width, height, format, nil, &pixelBufferCopyOptional)
        if let pixelBufferCopy = pixelBufferCopyOptional {
            CVPixelBufferLockBaseAddress(self, .readOnly)
            CVPixelBufferLockBaseAddress(pixelBufferCopy, CVPixelBufferLockFlags(rawValue: 0))
            let baseAddress = CVPixelBufferGetBaseAddress(self)
            let dataSize = CVPixelBufferGetDataSize(self)
            //print("dataSize: \(dataSize)")
            let target = CVPixelBufferGetBaseAddress(pixelBufferCopy)
            memcpy(target, baseAddress, dataSize)
            CVPixelBufferUnlockBaseAddress(pixelBufferCopy, CVPixelBufferLockFlags(rawValue: 0))
            CVPixelBufferUnlockBaseAddress(self, .readOnly)
        }
        return pixelBufferCopyOptional
    }
    
    
}
extension UIImage {
    
    func saveImageToDocumentDirectory(name:String,type:PictureType) -> URL {
        let directoryPath =  NSHomeDirectory().appending("/Documents/")
        if !FileManager.default.fileExists(atPath: directoryPath) {
            do {
                try FileManager.default.createDirectory(at: NSURL.fileURL(withPath: directoryPath), withIntermediateDirectories: true, attributes: nil)
            } catch {
                print(error)
            }
        }
        
        let filename:String
        switch (type){
        case .jpg:
            filename = name.appending(".jpg")
        case .png:
            filename = name.appending(".png")
        }
        
        let filepath = directoryPath.appending(filename)
        let url = NSURL.fileURL(withPath: filepath)
        do {
            switch (type){
            case .jpg:
                try self.jpegData(compressionQuality: 1.0)?.write(to: url, options: .atomic)
            case .png:
                try self.pngData()?.write(to: url, options: .atomic)
            }
            
            
        } catch {
            print(error)
            print("file cant not be save at path \(filepath), with error : \(error)");
            
        }
        return url
    }
    
    func buffer() -> CVPixelBuffer? {
        let attrs = [kCVPixelBufferCGImageCompatibilityKey: kCFBooleanTrue, kCVPixelBufferCGBitmapContextCompatibilityKey: kCFBooleanTrue] as CFDictionary
        var pixelBuffer : CVPixelBuffer?
        let status = CVPixelBufferCreate(kCFAllocatorDefault, Int(self.size.width), Int(self.size.height), kCVPixelFormatType_32ARGB, attrs, &pixelBuffer)
        guard (status == kCVReturnSuccess) else {
            return nil
        }
        
        CVPixelBufferLockBaseAddress(pixelBuffer!, CVPixelBufferLockFlags(rawValue: 0))
        let pixelData = CVPixelBufferGetBaseAddress(pixelBuffer!)
        
        let rgbColorSpace = CGColorSpaceCreateDeviceRGB()
        let context = CGContext(data: pixelData, width: Int(self.size.width), height: Int(self.size.height), bitsPerComponent: 8, bytesPerRow: CVPixelBufferGetBytesPerRow(pixelBuffer!), space: rgbColorSpace, bitmapInfo: CGImageAlphaInfo.noneSkipFirst.rawValue)
        
        context?.translateBy(x: 0, y: self.size.height)
        context?.scaleBy(x: 1.0, y: -1.0)
        
        UIGraphicsPushContext(context!)
        self.draw(in: CGRect(x: 0, y: 0, width: self.size.width, height: self.size.height))
        UIGraphicsPopContext()
        CVPixelBufferUnlockBaseAddress(pixelBuffer!, CVPixelBufferLockFlags(rawValue: 0))
        
        return pixelBuffer
    }
    
    
    func resizeImage(size:CGSize)->UIImage{
        UIGraphicsBeginImageContextWithOptions(size, false, 0.0)
        let rect = CGRect(x: 0, y: 0, width: size.width, height: size.height)
        self.draw(in:rect)
        let output = UIGraphicsGetImageFromCurrentImageContext()!.copy() as! UIImage
        UIGraphicsEndImageContext()
        return output
    }
    
    var colorData: [UInt8] {
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        
        guard let cgImage = cgImage else {
            return []
        }
        let width = Int(size.width)
        let height = Int(size.height)
        
        var rawData = [UInt8](repeating: 0, count: width * height * 4)
        let bytesPerPixel = 4
        let bytesPerRow = bytesPerPixel * width
        let bytesPerComponent = 8
        
        let bitmapInfo = CGImageAlphaInfo.premultipliedLast.rawValue | CGBitmapInfo.byteOrder32Big.rawValue
        
        let context = CGContext(data: &rawData,
                                width: width,
                                height: height,
                                bitsPerComponent: bytesPerComponent,
                                bytesPerRow: bytesPerRow,
                                space: colorSpace,
                                bitmapInfo: bitmapInfo)
        
        let drawingRect = CGRect(origin: .zero, size: CGSize(width: width, height: height))
        context?.draw(cgImage, in: drawingRect)
        return rawData
    }
    struct RotationOptions: OptionSet {
        let rawValue: Int
        
        static let flipOnVerticalAxis = RotationOptions(rawValue: 1)
        static let flipOnHorizontalAxis = RotationOptions(rawValue: 2)
    }
    
    func rotated(by rotationAngle: Measurement<UnitAngle>, options: RotationOptions = []) -> UIImage? {
        guard let cgImage = self.cgImage else { return nil }
        
        let rotationInRadians = CGFloat(rotationAngle.converted(to: .radians).value)
        let transform = CGAffineTransform(rotationAngle: rotationInRadians)
        var rect = CGRect(origin: .zero, size: self.size).applying(transform)
        rect.origin = .zero
        
        let renderer = UIGraphicsImageRenderer(size: rect.size)
        return renderer.image { renderContext in
            renderContext.cgContext.translateBy(x: rect.midX, y: rect.midY)
            renderContext.cgContext.rotate(by: rotationInRadians)
            
            let x = options.contains(.flipOnVerticalAxis) ? -1.0 : 1.0
            let y = options.contains(.flipOnHorizontalAxis) ? 1.0 : -1.0
            renderContext.cgContext.scaleBy(x: CGFloat(x), y: CGFloat(y))
            
            let drawRect = CGRect(origin: CGPoint(x: -self.size.width/2, y: -self.size.height/2), size: self.size)
            renderContext.cgContext.draw(cgImage, in: drawRect)
        }
    }
}

extension String {
    func appendLineToURL(fileURL: URL) throws {
        try (self + "\n").appendToURL(fileURL: fileURL)
    }
    
    func appendToURL(fileURL: URL) throws {
        let data = self.data(using: String.Encoding.utf8)!
        try data.append(fileURL: fileURL)
    }
}



extension Data {
    func append(fileURL: URL) throws {
        if let fileHandle = FileHandle(forWritingAtPath: fileURL.path) {
            defer {
                fileHandle.closeFile()
            }
            fileHandle.seekToEndOfFile()
            fileHandle.write(self)
        }
        else {
            try write(to: fileURL, options: .atomic)
        }
    }
}

extension Array {
    func parallelMap<R>(striding n: Int, f: @escaping (Element) -> R, completion: @escaping ([R]) -> ()) {
        let N = self.count
        
        let res = UnsafeMutablePointer<R>.allocate(capacity: N)
        
        DispatchQueue.concurrentPerform(iterations: N/n) { k in
            for i in (k * n)..<((k + 1) * n) {
                res[i] = f(self[i])
                print("i \(i)")
            }
        }
        
        
        for i in (N - (N % n))..<N {
            res[i] = f(self[i])
        }
        
        let finalResult = Array<R>(UnsafeBufferPointer(start: res, count: N))
        res.deallocate()
        
        DispatchQueue.main.async {
            completion(finalResult)
        }
    }
}

extension Array{
    func writeDepthTxt(width:Int,height:Int,filename:String)->URL{
        let dir: URL = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).last! as URL
        let url = dir.appendingPathComponent(filename)
        writeTxtFile(fileName: filename, textToWrite: "")
        do{
            let fileHandle = try FileHandle(forWritingTo: url)
            for j in 0..<height{
                for i in 0..<width{
                    let count = j * width + i
                    let tmpData = "\(self[count]),".data(using: String.Encoding.utf8, allowLossyConversion: false)!
                    fileHandle.seekToEndOfFile()
                    fileHandle.write(tmpData)
                }
                let tmpData = "\n".data(using: String.Encoding.utf8, allowLossyConversion: false)!
                fileHandle.seekToEndOfFile()
                fileHandle.write(tmpData)
            }
            fileHandle.closeFile()
        }catch{
            
        }
        return url
    }
}

func writeTxtFile(fileName:String,textToWrite:String)->URL?{
    if let dir = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first {
        let fileURL = dir.appendingPathComponent(fileName)
        //writing
        do {
            try textToWrite.write(to: fileURL, atomically: false, encoding: .utf8)
        }
        catch {/* error handling here */}
        return fileURL
    }
    return nil
}

func writeDepthCSV(depthData:[Float],width:Int,height:Int,filename:String,minD:Float,maxD:Float)->URL{
    var tmpText = ""
    
    for j in 0..<height{
        for i in 0..<width{
            let count = j * width + i
            if (depthData[count] > minD && depthData[count] < maxD){
               tmpText += "\(depthData[count])"
            }else{
                tmpText += "0"
            }
            if (i != width - 1){
                tmpText += ","
            }
        }
        tmpText += "\n"
    }
    return writeTxtFile(fileName: filename, textToWrite: tmpText)!
}

extension UIView {
    func flash(numberOfFlashes: Float) {
        let flash = CABasicAnimation(keyPath: "opacity")
        flash.duration = 0.2
        flash.fromValue = 1
        flash.toValue = 0.1
        flash.timingFunction = CAMediaTimingFunction(name: CAMediaTimingFunctionName.easeInEaseOut)
        flash.autoreverses = true
        flash.repeatCount = numberOfFlashes
        layer.add(flash, forKey: nil)
    }
}

extension Array where Element == Float {
    func writeBinary(url:URL){
        
    }
}

extension UIDevice {
    var modelName: String {
        var systemInfo = utsname()
        uname(&systemInfo)
        let machineMirror = Mirror(reflecting: systemInfo.machine)
        let identifier = machineMirror.children.reduce("") { identifier, element in
            guard let value = element.value as? Int8, value != 0 else { return identifier }
            return identifier + String(UnicodeScalar(UInt8(value)))
        }
        return identifier
    }
}
