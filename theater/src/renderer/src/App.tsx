import { useState } from 'react'
import React from 'react'

function App(): React.JSX.Element {
  const [result, setResult] = useState<number | null>(null)
  const [numA, setNumA] = useState<number>(5)
  const [numB, setNumB] = useState<number>(3)
  const [message, setMessage] = useState<string>('')

  const handleAdd = async (): Promise<void> => {
    try {
      const sum = await window.api.native.add(numA, numB)
      setResult(sum)
    } catch (error) {
      console.error('Error calling native add:', error)
      setMessage('Error: ' + (error as Error).message)
    }
  }

  const handlePrintMessage = async (): Promise<void> => {
    try {
      const msg = await window.api.native.printMessage()
      setMessage(msg)
    } catch (error) {
      console.error('Error calling native printMessage:', error)
      setMessage('Error: ' + (error as Error).message)
    }
  }

  return (
    <div className="container">
      <h1>Electron + React + Native C++ DLL</h1>

      <div className="section">
        <h2>Test Native Add Function</h2>
        <div className="inputs">
          <input
            type="number"
            value={numA}
            onChange={(e) => setNumA(Number(e.target.value))}
            placeholder="Number A"
          />
          <span>+</span>
          <input
            type="number"
            value={numB}
            onChange={(e) => setNumB(Number(e.target.value))}
            placeholder="Number B"
          />
          <button onClick={handleAdd}>Calculate</button>
        </div>
        {result !== null && (
          <div className="result">
            Result: {numA} + {numB} = {result}
          </div>
        )}
      </div>

      <div className="section">
        <h2>Test Print Message Function</h2>
        <button onClick={handlePrintMessage}>Print Message to Console</button>
        {message && <div className="message">{message}</div>}
      </div>

      <style>{`
        .container {
          padding: 20px;
          font-family: Arial, sans-serif;
        }
        
        .section {
          margin: 30px 0;
          padding: 20px;
          border: 1px solid #ccc;
          border-radius: 8px;
        }
        
        .inputs {
          display: flex;
          gap: 10px;
          align-items: center;
          margin: 15px 0;
        }
        
        input {
          padding: 8px;
          font-size: 16px;
          border: 1px solid #ccc;
          border-radius: 4px;
          width: 100px;
        }
        
        button {
          padding: 10px 20px;
          font-size: 16px;
          background-color: #007bff;
          color: white;
          border: none;
          border-radius: 4px;
          cursor: pointer;
        }
        
        button:hover {
          background-color: #0056b3;
        }
        
        .result, .message {
          margin-top: 15px;
          padding: 10px;
          background-color: #e7f3ff;
          border-radius: 4px;
          font-weight: bold;
        }
      `}</style>
    </div>
  )
}

export default App