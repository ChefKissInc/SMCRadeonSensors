//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

import Cocoa

enum RadeonSensorSelector: UInt32 {
    case getVersion = 0
    case getCardCount = 1
    case getTemperatures = 2
}

class RadeonModel {
    static let shared = RadeonModel()
    private var connect: io_connect_t = 0

    init() {
       if !self.initDriver() {
           self.alert("Please download RadeonSensor from the release page", critical: true)
       }

       let gadgetVersion = (0, 4)
       let kextVersion = self.getKextVersion()
       if kextVersion.1 < gadgetVersion.1 || kextVersion.0 != gadgetVersion.0 {
           self.alert("Your RadeonSensor is incompatible with this version of RadeonGadget", critical: true)
       } else if kextVersion.1 > gadgetVersion.1 {
           self.alert("Update to the latest version for more features")
       }
    }

    func getKextVersion() -> (Int, Int) {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 1

        _ = IOConnectCallMethod(connect, RadeonSensorSelector.getVersion.rawValue, nil, 0, nil, 0,
                                      &scalarOut, &outputCount, nil, nil)

        var outputStrCount: Int = Int(scalarOut)
        var outputStr: [CChar] = [CChar](repeating: 0, count: outputStrCount)
        _ = IOConnectCallMethod(connect, RadeonSensorSelector.getVersion.rawValue, nil, 0, nil, 0,
                                      &scalarOut, &outputCount,
                                      &outputStr, &outputStrCount)

        let version = String(cString: Array(outputStr[0...outputStrCount - 1])).components(separatedBy: ".")
        if version.count <= 1 { self.alert("Invalid kext version", critical: true) }
        return (Int(version[0]) ?? 0, Int(version[1]) ?? 0)
    }

    func getGpuCount() -> Int {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 1

        var outputStr: [UInt64] = [UInt64]()
        var outputStrCount: Int = 0
        _ = IOConnectCallMethod(connect, RadeonSensorSelector.getCardCount.rawValue, nil, 0, nil, 0,
                                       &scalarOut, &outputCount, nil, nil)

        return Int(scalarOut)
    }

    func getTemps(_ gpuCount: Int) -> [Int] {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 0

        var outputStr: [UInt16] = [UInt16](repeating: 0, count: gpuCount)
        var outputStrCount: Int = MemoryLayout<UInt16>.size * gpuCount
        _ = IOConnectCallMethod(connect, RadeonSensorSelector.getTemperatures.rawValue, nil, 0, nil, 0,
                                      &scalarOut, &outputCount,
                                      &outputStr, &outputStrCount)

        return outputStr.map { Int($0) }
    }

    private func initDriver() -> Bool {
        let serviceObject = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("RadeonSensor"))
        if serviceObject == 0 {
            return false
        }

        return IOServiceOpen(serviceObject, mach_task_self_, 0, &connect) == KERN_SUCCESS
    }

    private func alert(_ message: String, critical: Bool = false) {
        let alert = NSAlert()
        alert.messageText = critical ? "RadeonSensor communication failure" : "Update Available"
        alert.informativeText = message
        alert.alertStyle = .critical
        alert.addButton(withTitle: critical ? "Quit" : "Dismiss")
        alert.addButton(withTitle: "\(critical ? "Quit and " : "")Download")
        if alert.runModal() == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/NootInc/RadeonSensor/releases")!)
        }
        if critical { NSApplication.shared.terminate(self) }
    }
}
