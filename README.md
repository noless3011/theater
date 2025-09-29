# Theater 🎭

A hybrid desktop application built with Electron (React + TypeScript) frontend and C++ backend library.

## Overview

Theater is a cross-platform desktop application that combines the power of modern web technologies with native C++ performance. The project consists of two main components:

- **Frontend (`theater/`)**: An Electron application built with React, TypeScript, and Vite
- **Backend (`theater_be/`)**: A C++ shared library (DLL/SO/DYLIB) for high-performance operations

## 🚀 Features

- Cross-platform desktop application (Windows, macOS, Linux)
- Modern React UI with TypeScript support
- TailwindCSS for styling
- Native C++ backend for performance-critical operations
- Comprehensive testing setup with CTest
- Hot reload development environment

## 📋 Prerequisites

Before you begin, ensure you have the following installed:

- **Node.js** (v16 or higher)
- **npm** or **yarn**
- **CMake** (v3.10 or higher)
- **C++ compiler** (GCC, Clang, or MSVC)
- **Git**

## 🛠️ Installation

1. **Clone the repository**

   ```bash
   git clone https://github.com/noless3011/theater.git
   cd theater
   ```

2. **Install frontend dependencies**

   ```bash
   cd theater
   npm install
   ```

3. **Build the C++ backend**
   ```bash
   cd ../theater_be
   cmake --preset gcc
   cmake --build out/build/gcc
   ```

## 🏃‍♂️ Running the Application

### Development Mode

1. **Start the Electron app in development mode**

   ```bash
   cd theater
   npm run dev
   ```

2. **Run tests for the C++ backend**
   ```bash
   cd theater_be
   ctest --preset gcc-test --verbose
   ```

### Production Build

1. **Build the frontend**

   ```bash
   cd theater
   npm run build
   ```

2. **Create distributable packages**

   ```bash
   # Windows
   npm run build:win

   # macOS
   npm run build:mac

   # Linux
   npm run build:linux
   ```

## 🏗️ Project Structure

```
theater/
├── LICENSE                 # MIT License
├── README.md              # This file
├── theater/               # Electron frontend
│   ├── package.json       # Node.js dependencies and scripts
│   ├── electron.vite.config.ts  # Vite configuration
│   ├── src/
│   │   ├── main/          # Electron main process
│   │   ├── preload/       # Preload scripts
│   │   └── renderer/      # React application
│   └── build/             # Build assets and icons
└── theater_be/            # C++ backend
    ├── CMakeLists.txt     # CMake configuration
    ├── CMakePresets.json  # CMake presets
    ├── src/               # C++ source files
    ├── include/           # Header files
    └── tests/             # Unit tests
```

## 🧪 Testing

### Frontend Tests

```bash
cd theater
npm run lint          # ESLint
npm run typecheck     # TypeScript type checking
```

### Backend Tests

```bash
cd theater_be
ctest --preset gcc-test --verbose
```

## 🔧 Development

### Frontend Development

- Uses **Electron Vite** for fast development builds
- **React 19** with TypeScript for the UI
- **TailwindCSS** for styling
- Hot module replacement (HMR) enabled

### Backend Development

- Written in **C++17**
- Uses **CMake** for build configuration
- Supports cross-platform compilation
- Includes comprehensive test suite with **CTest**

### Available Scripts (Frontend)

| Script              | Description                 |
| ------------------- | --------------------------- |
| `npm run dev`       | Start development server    |
| `npm run build`     | Build for production        |
| `npm run lint`      | Run ESLint                  |
| `npm run format`    | Format code with Prettier   |
| `npm run typecheck` | Type check TypeScript files |

### Available Tasks (Backend)

Use VS Code tasks or run manually:

| Task            | Command                             |
| --------------- | ----------------------------------- |
| Configure CMake | `cmake --preset gcc`                |
| Build           | `cmake --build out/build/gcc`       |
| Test            | `ctest --preset gcc-test --verbose` |

### VS Code Configuration

The project includes pre-configured VS Code settings for optimal development experience:

#### Recommended Extensions

The `.vscode/extensions.json` file includes recommended extensions:

- **ESLint** (`dbaeumer.vscode-eslint`) - JavaScript/TypeScript linting
- **C/C++** (`ms-vscode.cpptools`) - C++ language support and debugging

<details>
<summary>View extensions.json content</summary>

```json
{
  "recommendations": ["dbaeumer.vscode-eslint", "ms-vscode.cpptools"]
}
```

</details>

#### Tasks Configuration

Pre-configured tasks in `.vscode/tasks.json`:

- **Configure CMake** - Sets up the CMake build system
- **Build DLL with CMake** - Compiles the C++ backend
- **Run CTest** - Executes the test suite
- **Build and Test** - Combined build and test sequence
- **C/C++: g++.exe build active file** - Quick compile for active C++ file

Access these tasks via:

- Command Palette: `Ctrl+Shift+P` → "Tasks: Run Task"
- Terminal menu: `Terminal` → `Run Task...`

<details>
<summary>View tasks.json content</summary>

```json
{
  "tasks": [
    {
      "label": "Build DLL with CMake",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", "out/build/gcc"],
      "group": "build",
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": true,
        "clear": false
      },
      "options": {
        "cwd": "${workspaceFolder}/theater_be"
      },
      "dependsOn": "Configure CMake",
      "problemMatcher": ["$gcc"]
    },
    {
      "label": "Configure CMake",
      "type": "shell",
      "command": "cmake",
      "args": ["--preset", "gcc"],
      "group": "build",
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": true,
        "clear": false
      },
      "options": {
        "cwd": "${workspaceFolder}/theater_be"
      },
      "problemMatcher": ["$gcc"]
    },
    {
      "label": "Run CTest",
      "type": "shell",
      "command": "ctest",
      "args": ["--preset", "gcc-test", "--verbose"],
      "group": "test",
      "presentation": {
        "echo": true,
        "reveal": "always",
        "focus": false,
        "panel": "shared",
        "showReuseMessage": true,
        "clear": false
      },
      "options": {
        "cwd": "${workspaceFolder}/theater_be"
      },
      "dependsOn": "Build DLL with CMake",
      "problemMatcher": ["$gcc"]
    },
    {
      "label": "Build and Test",
      "dependsOrder": "sequence",
      "dependsOn": ["Build DLL with CMake", "Run CTest"],
      "group": "test"
    },
    {
      "type": "cppbuild",
      "label": "C/C++: g++.exe build active file",
      "command": "C:\\ProgramData\\mingw64\\mingw64\\bin\\g++.exe",
      "args": [
        "-fdiagnostics-color=always",
        "-g",
        "${file}",
        "-o",
        "${fileDirname}\\${fileBasenameNoExtension}.exe"
      ],
      "options": {
        "cwd": "${fileDirname}"
      },
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "detail": "Task generated by Debugger."
    }
  ]
}
```

</details>

#### Debug Configuration

Launch configurations in `.vscode/launch.json`:

- **Debug Main Process** - Debug the Electron main process
- **Debug Renderer Process** - Debug the React renderer process
- **Debug All** - Debug both processes simultaneously

Start debugging with `F5` or via the Run and Debug panel (`Ctrl+Shift+D`).

<details>
<summary>View launch.json content</summary>

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Debug Main Process",
      "type": "node",
      "request": "launch",
      "cwd": "${workspaceRoot}/theater",
      "runtimeExecutable": "${workspaceRoot}/theater/node_modules/.bin/electron-vite",
      "windows": {
        "runtimeExecutable": "${workspaceRoot}/theater/node_modules/.bin/electron-vite.cmd"
      },
      "runtimeArgs": ["--sourcemap"],
      "env": {
        "REMOTE_DEBUGGING_PORT": "9222"
      }
    },
    {
      "name": "Debug Renderer Process",
      "port": 9222,
      "request": "attach",
      "type": "chrome",
      "webRoot": "${workspaceFolder}/theater/src/renderer",
      "timeout": 60000,
      "presentation": {
        "hidden": true
      }
    }
  ],
  "compounds": [
    {
      "name": "Debug All",
      "configurations": ["Debug Main Process", "Debug Renderer Process"],
      "presentation": {
        "order": 1
      }
    }
  ]
}
```

</details>

#### Editor Settings

Configured in `.vscode/settings.json`:

- **Prettier** as default formatter for TypeScript, JavaScript, and JSON files
- Consistent code formatting across the project

<details>
<summary>View settings.json content</summary>

```json
{
  "[typescript]": {
    "editor.defaultFormatter": "esbenp.prettier-vscode"
  },
  "[javascript]": {
    "editor.defaultFormatter": "esbenp.prettier-vscode"
  },
  "[json]": {
    "editor.defaultFormatter": "esbenp.prettier-vscode"
  }
}
```

</details>

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow the existing code style
- Write tests for new features
- Update documentation as needed
- Ensure all tests pass before submitting PR

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Nguyễn Quang Thịnh** - [noless3011](https://github.com/noless3011)

## 🙏 Acknowledgments

- Built with [Electron](https://electronjs.org/)
- UI powered by [React](https://reactjs.org/)
- Styling with [TailwindCSS](https://tailwindcss.com/)
- Build system using [Vite](https://vitejs.dev/) and [CMake](https://cmake.org/)

---

_For more detailed information about specific components, check the documentation in the respective directories._
