import { ElectronAPI } from '@electron-toolkit/preload'

declare global {
  interface Window {
    electron: ElectronAPI
    api: {
      native: {
        add: (a: number, b: number) => Promise<number>
        printMessage: () => Promise<string>
      }
    }
  }
}

export {}
