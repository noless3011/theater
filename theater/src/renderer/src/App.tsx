function App(): React.JSX.Element {
  const ipcHandle = (): void => window.electron.ipcRenderer.send('ping')

  return (
    <div className="bg-amber-300 w-full h-full flex items-center justify-center">
      <button
        onClick={ipcHandle}
        className="bg-blue-500 w-36 h-12 rounded-4xl flex justify-center items-center"
      >
        Ping
      </button>
    </div>
  )
}

export default App
