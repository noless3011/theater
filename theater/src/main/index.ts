import { app, shell, BrowserWindow, ipcMain } from 'electron'
import { join } from 'path'
import { electronApp, optimizer, is } from '@electron-toolkit/utils'
import icon from '../../resources/icon.png?asset'
import { createRequire } from 'module'

const require = createRequire(import.meta.url)

// Import the native addon
// eslint-disable-next-line @typescript-eslint/no-explicit-any
let nativeAddon: any
try {
  // In development
  if (is.dev) {
    nativeAddon = require('../../native/build/Release/native_addon.node')
  } else {
    // In production, the .node file should be in resources
    nativeAddon = require(join(process.resourcesPath, 'native_addon.node'))
  }
} catch (error) {
  console.error('Failed to load native addon:', error)
}

// Type definitions for the native addon
interface NativeAddon {
  add(a: number, b: number): number
  printMessage(): void
}

const native: NativeAddon = nativeAddon

function createWindow(): void {
  const mainWindow = new BrowserWindow({
    width: 900,
    height: 670,
    show: false,
    autoHideMenuBar: true,
    ...(process.platform === 'linux' ? { icon } : {}),
    webPreferences: {
      preload: join(__dirname, '../preload/index.js'),
      sandbox: false,
      contextIsolation: true,
      nodeIntegration: false
    }
  })

  mainWindow.on('ready-to-show', () => {
    mainWindow.show()
  })

  mainWindow.webContents.setWindowOpenHandler((details) => {
    shell.openExternal(details.url)
    return { action: 'deny' }
  })

  if (is.dev && process.env['ELECTRON_RENDERER_URL']) {
    mainWindow.loadURL(process.env['ELECTRON_RENDERER_URL'])
  } else {
    mainWindow.loadFile(join(__dirname, '../renderer/index.html'))
  }

  // Test the native addon
  if (native) {
    console.log('Testing native addon...')
    const result = native.add(10, 20)
    console.log(`10 + 20 = ${result}`)
    native.printMessage()
  }
}

// IPC handlers to expose native functions to renderer
ipcMain.handle('native:add', (_event, a: number, b: number) => {
  if (!native) {
    throw new Error('Native addon not loaded')
  }
  return native.add(a, b)
})

ipcMain.handle('native:printMessage', () => {
  if (!native) {
    throw new Error('Native addon not loaded')
  }
  native.printMessage()
  return 'Message printed'
})

app.whenReady().then(() => {
  electronApp.setAppUserModelId('com.electron')

  app.on('browser-window-created', (_, window) => {
    optimizer.watchWindowShortcuts(window)
  })

  createWindow()

  app.on('activate', function () {
    if (BrowserWindow.getAllWindows().length === 0) createWindow()
  })
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})
