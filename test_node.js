const path = require("path");

// Try to load the native addon
let theaterSoundApi;
try {
  // Adjust the path based on where your .node file is located
  theaterSoundApi = require("./theater/native/build/Release/theater_sound_api.node");
  console.log("✅ Native addon loaded successfully");
} catch (error) {
  console.error("❌ Failed to load native addon:", error.message);
  process.exit(1);
}

// Helper function to run tests
function runTest(testName, testFn) {
  try {
    console.log(`\n🧪 Running test: ${testName}`);
    testFn();
    console.log(`✅ ${testName} - PASSED`);
  } catch (error) {
    console.error(`❌ ${testName} - FAILED:`, error.message);
  }
}

// Test 1: Basic API functions
runTest("Basic Add Function", () => {
  const result = theaterSoundApi.add(5, 10);
  if (result !== 15) {
    throw new Error(`Expected 15, got ${result}`);
  }
  console.log(`   add(5, 10) = ${result}`);
});

runTest("Print Message Function", () => {
  theaterSoundApi.printMessage();
  console.log("   printMessage() executed (check console for output)");
});

// Test 2: Process Management API
runTest("Process Map Creation and Destruction", () => {
  const processMap = theaterSoundApi.createProcessMap();
  if (!processMap) {
    throw new Error("Failed to create process map");
  }
  console.log("   Process map created successfully");

  theaterSoundApi.destroyProcessMap(processMap);
  console.log("   Process map destroyed successfully");
});

runTest("Get Process List", () => {
  const processMap = theaterSoundApi.createProcessMap();
  const processList = theaterSoundApi.getProcessList(processMap);

  if (!Array.isArray(processList)) {
    throw new Error("Process list should be an array");
  }

  console.log(`   Found ${processList.length} processes`);
  if (processList.length > 0) {
    console.log(
      `   First few process IDs: ${processList.slice(0, 3).join(", ")}`
    );
  }

  theaterSoundApi.destroyProcessMap(processMap);
});

runTest("Process Name Retrieval", () => {
  const processMap = theaterSoundApi.createProcessMap();
  const processList = theaterSoundApi.getProcessList(processMap);

  if (processList.length > 0) {
    const firstProcessId = processList[0];
    const processName = theaterSoundApi.getProcessName(firstProcessId);
    console.log(`   Process ID ${firstProcessId} name: ${processName}`);
  } else {
    console.log("   No processes found to test name retrieval");
  }

  theaterSoundApi.destroyProcessMap(processMap);
});

runTest("Process Running Check", () => {
  const processMap = theaterSoundApi.createProcessMap();
  const processList = theaterSoundApi.getProcessList(processMap);

  if (processList.length > 0) {
    const firstProcessId = processList[0];
    const isRunning = theaterSoundApi.isProcessRunning(firstProcessId);
    console.log(`   Process ID ${firstProcessId} is running: ${isRunning}`);
  } else {
    console.log("   No processes found to test running check");
  }

  theaterSoundApi.destroyProcessMap(processMap);
});

// Test 3: Audio Capture API
runTest("Audio Capture Creation and Destruction", () => {
  const audioCapture = theaterSoundApi.createAudioCapture();
  if (!audioCapture) {
    throw new Error("Failed to create audio capture");
  }
  console.log("   Audio capture created successfully");

  theaterSoundApi.destroyAudioCapture(audioCapture);
  console.log("   Audio capture destroyed successfully");
});

runTest("Audio Capture Lifecycle", () => {
  const audioCapture = theaterSoundApi.createAudioCapture();

  // Check initial recording state
  let isRecording = theaterSoundApi.isAudioRecording(audioCapture);
  console.log(`   Initial recording state: ${isRecording}`);

  // Try to start recording
  const startResult = theaterSoundApi.startAudioCapture(audioCapture);
  console.log(`   Start audio capture result: ${startResult}`);

  // Check recording state after start
  isRecording = theaterSoundApi.isAudioRecording(audioCapture);
  console.log(`   Recording state after start: ${isRecording}`);

  // Try to stop recording
  const stopResult = theaterSoundApi.stopAudioCapture(audioCapture);
  console.log(`   Stop audio capture result: ${stopResult}`);

  // Check final recording state
  isRecording = theaterSoundApi.isAudioRecording(audioCapture);
  console.log(`   Final recording state: ${isRecording}`);

  theaterSoundApi.destroyAudioCapture(audioCapture);
});

runTest("Audio Queue Operations", () => {
  const audioCapture = theaterSoundApi.createAudioCapture();

  // Check initial queue size
  let queueSize = theaterSoundApi.getAudioQueueSize(audioCapture);
  console.log(`   Initial queue size: ${queueSize}`);

  // Clear the queue
  theaterSoundApi.clearAudioQueue(audioCapture);
  console.log("   Audio queue cleared");

  // Check queue size after clear
  queueSize = theaterSoundApi.getAudioQueueSize(audioCapture);
  console.log(`   Queue size after clear: ${queueSize}`);

  // Try to get next audio chunk
  const audioChunk = theaterSoundApi.getNextAudioChunk(audioCapture);
  console.log(
    `   Audio chunk retrieved: ${
      audioChunk ? `${audioChunk.length} bytes` : "null"
    }`
  );

  theaterSoundApi.destroyAudioCapture(audioCapture);
});

runTest("Set Target Process List", () => {
  const audioCapture = theaterSoundApi.createAudioCapture();
  const processMap = theaterSoundApi.createProcessMap();
  const processList = theaterSoundApi.getProcessList(processMap);

  if (processList.length > 0) {
    // Set the first few processes as targets
    const targetProcesses = processList.slice(
      0,
      Math.min(3, processList.length)
    );
    theaterSoundApi.setTargetProcessList(audioCapture, targetProcesses);
    console.log(
      `   Set ${
        targetProcesses.length
      } target processes: ${targetProcesses.join(", ")}`
    );
  } else {
    console.log("   No processes available to set as targets");
  }

  theaterSoundApi.destroyAudioCapture(audioCapture);
  theaterSoundApi.destroyProcessMap(processMap);
});

// Test 4: Error handling with invalid inputs
runTest("Error Handling - Invalid Process ID", () => {
  try {
    const isRunning = theaterSoundApi.isProcessRunning(999999);
    console.log(`   Invalid process ID check result: ${isRunning}`);
  } catch (error) {
    console.log(`   Expected error for invalid process ID: ${error.message}`);
  }
});

console.log("\n🎉 All tests completed!");
console.log(
  "\nNote: Some tests may show expected errors or warnings depending on system state."
);
console.log(
  "Audio capture functionality requires proper audio devices and permissions."
);
