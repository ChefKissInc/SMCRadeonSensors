//  Copyright © 2023 ChefKiss Inc. Licensed under the Thou Shalt Not Profit License version 1.0. See LICENSE for
//  details.

import Foundation
import AppKit
import Cocoa
import SwiftUI

private class StatusbarView: NSView {
    private var normalLabel: [NSAttributedString.Key: NSObject]?
    private var compactLabel: [NSAttributedString.Key: NSObject]?
    private var normalValue: [NSAttributedString.Key: NSObject]?

    var gpuCount: Int = 0
    var temps: [Int] = []

    func setup() {
        let compactLH: CGFloat = 6

        let pStyle = NSMutableParagraphStyle()
        pStyle.minimumLineHeight = compactLH
        pStyle.maximumLineHeight = compactLH

        self.compactLabel = [
            .font: NSFont.monospacedSystemFont(ofSize: 7.2, weight: .regular),
            .foregroundColor: NSColor.labelColor,
            .paragraphStyle: pStyle
        ]
        self.normalValue = [
            .font: NSFont.systemFont(ofSize: 14, weight: .regular),
            .foregroundColor: NSColor.labelColor
        ]
        self.normalLabel = [
            .font: NSFont.systemFont(ofSize: 13, weight: .regular),
            .foregroundColor: NSColor.labelColor
        ]
    }

    func drawTitle(_ label: String, x: CGFloat) {
        NSAttributedString(string: label, attributes: self.normalLabel).draw(at: NSPoint(x: x, y: 2.5))
    }

    func drawCompactSingle(_ label: String, _ value: String, x: CGFloat) {
        NSAttributedString(string: label, attributes: self.compactLabel)
            .draw(in: NSRect(x: x, y: -4.5, width: 7.2, height: self.frame.height))
        NSAttributedString(string: value, attributes: self.normalValue).draw(at: NSPoint(x: x + 7.2, y: 2.5))
    }

    override func draw(_ dirtyRect: NSRect) {
        guard (NSGraphicsContext.current?.cgContext) != nil else { return }
        self.drawTitle("GPU", x: 0.0)
        if self.gpuCount == 0 {
            self.drawTitle("NONE", x: 35.0)
            return
        }
        for i in 0...gpuCount - 1 {
            var temp: String
            if i >= self.temps.count || self.temps[i] == 255 {
                temp = "-  "
            } else if self.temps[i] > 125 {
                temp = "INV"
                NSLog("Invalid temperature for GPU %u: %u", i, self.temps[i])
            } else {
                temp = "\(self.temps[i])ºC".padding(toLength: 5, withPad: " ", startingAt: 0)
            }
            self.drawCompactSingle("GP\(i)", temp, x: 35.0 + CGFloat(i) * 45.0)
        }
    }
}

class StatusBarController {
    private var statusItem: NSStatusItem!
    private var view: StatusbarView!
    private var popover: NSPopover
    private var updateTimer: Timer?
    private var gpuCount: Int

    init() {
        self.statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        self.statusItem.isVisible = true

        self.gpuCount = RadeonModel.shared.getGpuCount()
        self.statusItem.length = (self.gpuCount == 0 ? 35.0 * 2.0 : 35.0) + CGFloat(self.gpuCount) * 45.0
        self.view = StatusbarView()
        self.view.gpuCount = self.gpuCount
        self.view.setup()

        self.popover = NSPopover()
        self.popover.contentViewController = NSHostingController(rootView: PopupView())

        if let statusBarButton = self.statusItem.button {
            self.view.frame = statusBarButton.bounds
            statusBarButton.wantsLayer = true
            statusBarButton.addSubview(view)
            statusBarButton.action = #selector(togglePopover(sender:))
            statusBarButton.target = self
        }

        if self.gpuCount > 0 {
            self.updateTimer = Timer.scheduledTimer(withTimeInterval: 2, repeats: true, block: { _ in
                self.update()
            })
        }
    }

    func update() {
        self.view.temps = RadeonModel.shared.getTemps(self.gpuCount)
        self.view.setNeedsDisplay(self.view.frame)
    }

    func dismiss() {
        self.updateTimer?.invalidate()
        NSStatusBar.system.removeStatusItem(self.statusItem!)
        self.statusItem = nil
    }

    @objc func togglePopover(sender: AnyObject) {
        if self.popover.isShown {
            self.popover.performClose(sender)
        } else if let statusBarButton = self.statusItem.button {
            self.popover.show(relativeTo: statusBarButton.bounds, of: statusBarButton, preferredEdge: NSRectEdge.minY)
        }
    }
}

struct PopupView: View {
    var body: some View {
        VStack {
            HStack {
                Image(nsImage: NSImage(named: "AppIcon")!)
                    .resizable()
                    .frame(width: 32.0, height: 32.0)
                Text("RadeonSensor")
                    .font(.title)
                    .padding(.bottom, 5.0)
            }
            Text("More coming soon™")
                .font(.caption)
                .padding(.bottom, 5.0)
            ZStack {
                Button("Exit", action: { exit(0) })
                Spacer()
                Text("Noot")
                    .font(.caption.italic())
                    .frame(maxWidth: .infinity, alignment: .trailing)
            }
            .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0)
        }
        .frame(minWidth: 0, maxWidth: .infinity, minHeight: 0)
        .padding(12.0)
    }
}
