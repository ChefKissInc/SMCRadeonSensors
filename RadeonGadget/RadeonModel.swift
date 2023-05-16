//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

import Cocoa

class RadeonModel {
    static let shared = RadeonModel()
    private var connect: io_connect_t = 0

    init() {
        if !initDriver() {
            alertAndQuit("Please download RadeonSensor from the release page.")
        }

        let gadgetVersion = (0, 4)
        let kextVersion = getKextVersion()
        if kextVersion.1 > gadgetVersion.1 {
            alert("There are updates available for RadeonSensor. Update to the latest version for more features.")
        } else if kextVersion.1 < gadgetVersion.1 || kextVersion.0 != gadgetVersion.0 {
            alertAndQuit("Your RadeonSensor.kext is incompatible with this version of RadeonGadget.")
        }
    }

    func getKextVersion() -> (Int, Int) {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 0

        let maxStrLength = 16
        var outputStr: [CChar] = [CChar](repeating: 0, count: maxStrLength)
        var outputStrCount: Int = maxStrLength
        _ = IOConnectCallMethod(connect, 0, nil, 0, nil, 0,
                                      &scalarOut, &outputCount,
                                      &outputStr, &outputStrCount)

        let kextVersion = String(cString: Array(outputStr[0...outputStrCount-1]))
        if kextVersion.contains(".") {
            let versionArr = kextVersion.components(separatedBy: ".")
            return (Int(versionArr[0]) ?? 0, Int(versionArr[1]) ?? 0)
        } else {
            return (0, 0)
        }
    }

    func getNrOfGpus() -> Int {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 1

        var outputStr: [UInt64] = [UInt64]()
        var outputStrCount: Int = 0
        _ = IOConnectCallMethod(connect, 1, nil, 0, nil, 0,
                                       &scalarOut, &outputCount,
                                       &outputStr, &outputStrCount)

        return Int(scalarOut)
    }

    func getTemps(_ nrOfGpus: Int) -> [Int] {
        var scalarOut: UInt64 = 0
        var outputCount: UInt32 = 0

        var outputStr: [UInt16] = [UInt16](repeating: 0, count: nrOfGpus)
        var outputStrCount: Int = 2 /* sizeof(UInt16) */ * nrOfGpus
        _ = IOConnectCallMethod(connect, 2, nil, 0, nil, 0,
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

    private func alertAndQuit(_ message: String) {
        let alert = NSAlert()
        alert.messageText = "RadeonSensor not found"
        alert.informativeText = message
        alert.alertStyle = .critical
        alert.addButton(withTitle: "Quit")
        alert.addButton(withTitle: "Quit and download")

        if alert.runModal() == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/NootInc/RadeonSensor/releases")!)
        }

        NSApplication.shared.terminate(self)
    }

    private func alert(_ message: String) {
        let alert = NSAlert()
        alert.messageText = "RadeonSensor update available"
        alert.informativeText = message
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Dismiss")
        alert.addButton(withTitle: "Visit releases page")

        if alert.runModal() == .alertSecondButtonReturn {
            NSWorkspace.shared.open(URL(string: "https://github.com/NootInc/RadeonSensor/releases")!)
        }
    }
}
