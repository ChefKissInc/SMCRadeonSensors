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
    private var compactValue: [NSAttributedString.Key: NSObject]?

    var temps: [Int] = []

    func setup() {
        let compactLH: CGFloat = 6

        let pStyle = NSMutableParagraphStyle()
        pStyle.minimumLineHeight = compactLH
        pStyle.maximumLineHeight = compactLH

        compactLabel = [
            NSAttributedString.Key.font: NSFont.init(name: "Monaco", size: 7.2)!,
            NSAttributedString.Key.foregroundColor: NSColor.labelColor,
            NSAttributedString.Key.paragraphStyle: pStyle
        ]

        normalValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 14, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor
        ]

        compactValue = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 9, weight: NSFont.Weight.semibold),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor
        ]

        normalLabel = [
            NSAttributedString.Key.font: NSFont.systemFont(ofSize: 13, weight: NSFont.Weight.regular),
            NSAttributedString.Key.foregroundColor: NSColor.labelColor
        ]
    }

    func drawTitle(label: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: normalLabel)
        attributedString.draw(at: NSPoint(x: 0, y: 2.5))
    }

    func drawCompactSingle(label: String, value: String, x: CGFloat) {
        let attributedString = NSAttributedString(string: label, attributes: compactLabel)
        attributedString.draw(in: NSRect(x: x, y: -4.5, width: 7, height: frame.height))

        let value = NSAttributedString(string: value, attributes: normalValue)
        value.draw(at: NSPoint(x: x + 10, y: 2.5))
    }
}

private class SingleGpuStatusbarView: StatusbarView {
    override func draw(_ dirtyRect: NSRect) {
        guard (NSGraphicsContext.current?.cgContext) != nil else { return }

        let temp: String
        if temps.count == 0 {
            temp = "-"
        } else if temps[0] > 125 {
            temp = "INV"
            NSLog("Invalid temperature: %u", temps[0])
        } else {
            temp = "\(temps[0])º"
        }

        drawTitle(label: "GPU", x: 0)
        drawCompactSingle(label: "TEM", value: temp, x: 35)
    }
}

private class MultiGpuStatusbarView: StatusbarView {
    var gpuCount: Int = 0

    override func draw(_ dirtyRect: NSRect) {
        guard (NSGraphicsContext.current?.cgContext) != nil else { return }

        drawTitle(label: "GPU", x: 0)
        for i in 0...gpuCount-1 {
            let temp: String
            if i > temps.count || temps[i] == 255 {
                temp = "-"
            } else if temps[i] > 125 {
                temp = "INV"
                NSLog("Invalid temperature for GPU %u: %u", i, temps[0])
            } else {
                temp = "\(temps[i])º"
            }
            drawCompactSingle(label: String(format: "GP%d", i + 1), value: temp, x: CGFloat(35 + (i)*40))
        }
    }
}

private class NoGpuStatusbarView: StatusbarView {
    override func draw(_ dirtyRect: NSRect) {
        guard (NSGraphicsContext.current?.cgContext) != nil else { return }

        drawTitle(label: "GPU NOT FOUND", x: 0)
    }
}

class StatusBarController {
    private var statusItem: NSStatusItem!
    private var view: StatusbarView!
    private var popover: NSPopover
    private var updateTimer: Timer?
    private var gpuCount: Int

    init() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        statusItem.isVisible = true

        gpuCount = RadeonModel.shared.getGpuCount()
        if gpuCount < 1 {
            view = NoGpuStatusbarView()
            statusItem.length = 110
        } else if gpuCount == 1 {
            view = SingleGpuStatusbarView()
            statusItem.length = 70
        } else {
            let multiview = MultiGpuStatusbarView()
            multiview.gpuCount = gpuCount
            view = multiview
            statusItem.length = CGFloat((35 + (gpuCount * 40) - 5))
        }
        view.setup()

        popover = NSPopover.init()
        let popupView = PopupView()
        popover.contentSize = NSSize(width: 120, height: 32)
        popover.contentViewController = NSHostingController(rootView: popupView)

        if let statusBarButton = statusItem.button {
            view.frame = statusBarButton.bounds
            statusBarButton.wantsLayer = true
            statusBarButton.addSubview(view)
            statusBarButton.action = #selector(togglePopover(sender:))
            statusBarButton.target = self
        }

        if gpuCount > 0 {
            updateTimer = Timer.scheduledTimer(withTimeInterval: 2, repeats: true, block: { _ in
                self.update()
            })
        }
    }

    func update() {
        view.temps = RadeonModel.shared.getTemps(gpuCount)
        view.setNeedsDisplay(view.frame)
    }

    func dismiss() {
        updateTimer?.invalidate()
        NSStatusBar.system.removeStatusItem(statusItem!)
        statusItem = nil
    }

    @objc func togglePopover(sender: AnyObject) {
        if popover.isShown {
            popover.performClose(sender)
        } else if let statusBarButton = statusItem.button {
            popover.show(relativeTo: statusBarButton.bounds, of: statusBarButton, preferredEdge: NSRectEdge.maxY)
        }
    }
}

struct PopupView: View {
    var body: some View {
        Button(action: { exit(0) }) {
            Text("Exit").frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }
}
