//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

import Cocoa

class AppDelegate: NSObject, NSApplicationDelegate {
    var statusBar: StatusBarController?

    static var shared: AppDelegate {
        guard let delegate = NSApp.delegate as? AppDelegate else {
            fatalError("App delegate not set")
        }
        return delegate
    }

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        NSApp.setActivationPolicy(NSApplication.ActivationPolicy.accessory)
        statusBar = StatusBarController.init()
    }

    func applicationWillTerminate(_ aNotification: Notification) {}

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
}
