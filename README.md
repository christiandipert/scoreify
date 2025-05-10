### 1. Overview

This application is designed to recognize classical music in real-time using Fast Fourier Transform (FFT) and frequency peak matching. It will process live violin audio input from a MacBook microphone and match it against a precomputed dataset of classical pieces. Additionally, users can switch to file-based audio input for recognition. The system will provide a live spectrogram, a ranked match list, and automatic playback of the recognized piece.

## 2. Core Features

- **Real-time recognition** with progressive narrowing of candidate matches.
- **Live spectrogram visualization** updating smoothly.
- **Two input modes:**
  - Microphone input (live violin playing).
  - File input (treat an audio file as a mic input).
- **Precomputed fingerprinting:**
  - MP3 dataset processed in advance and stored in a SQLite database.
  - Fingerprints stored in binary format for fast lookups.
- **Confidence-based match ranking:**
  - Display multiple possible matches if uncertain.
  - Use a 500ms confidence smoothing window before locking onto a match.
- **Automatic audio playback:**
  - Play the matched piece from the corresponding section.
  - Stop playback if the match changes.
- **Session history** (GUI only, resets on app close).
- **Manual import of new MP3s** with user-defined metadata.
- **Logs and debugging mode** for raw frequency analysis.

## 3. Architecture & Technologies

- **Language:** C++17
- **GUI Framework:** Dear ImGui (for real-time visualization)
- **Audio Processing:**
  - CoreAudio for capture and playback
  - LAME for MP3 encoding/decoding
  - FFTW3 for FFT analysis
- **Database:** SQLite3 (stores fingerprints & metadata)
- **File Handling:** MP3 files are referenced by path (not copied)
- **Build System:** CMake
- **Dependencies:**
  - libmp3lame
  - FFTW3
  - SQLite3
  - Dear ImGui
  - GLFW (for window management)

## 4. Data Handling & Storage

### Fingerprint Database

- Precomputed fingerprints are stored in SQLite3.
- Each entry consists of:
  - **Piece Metadata:** {title, composer, movement}
  - **Fingerprint Data** (binary, indexed for fast search)
  - Time-stamped frequency peaks for matching against live input.

### Session History

- Stored in-memory only (clears on app restart).
- Includes:
  - List of detected pieces & confidence scores.
  - Matched frequency peaks.
  - Export option (JSON).

### Logging & Error Handling

- Errors are logged to *app.log* in the same directory.
- GUI displays brief error messages, full details in logs.
- If a missing MP3 file is detected, the user must manually remove it.

## 5. Recognition Pipeline

1. Audio Capture (CoreAudio Input).
2. Apply FFT (FFTW3) & Extract Frequency Peaks.
3. Compare Peaks to Precomputed Database (Allowing for Pitch/Tempo Variations).
4. Smooth Confidence Over 500ms Window.
5. Update GUI with Top Matches.
6. Trigger Playback When a Stable Match is Found.
7. Stop Playback if Silence is Detected for 15-20 Seconds.

## 6. User Interface (GUI Layout)

- **Live Spectrogram Panel** (real-time frequency visualization using Dear ImGui).
- **Match List Panel** (ranked results + confidence bars).
- **Status Bar** (shows current mode, processing status, last action).
- **Audio Playback Controls** (play/pause, seek, volume, stop if new match appears).
- **Buttons:**
  - Start (begin recognition)
  - Stop (manual stop)
  - Reset (clear session history)
  - Import MP3 (add new pieces to database)
  - Export Session (save JSON history)

## 7. Error Handling & Edge Cases

- **Low Confidence:** Display "No Match Found."
- **Short Inputs:** Start matching immediately, refine over time.
- **Silent Period (15-20s):** Automatically stop recognition.
- **File Missing:** Warn user, allow manual deletion.
- **Multiple Matching Pieces:** Show all candidates, refine over time.
- **Application Exit:** Closes immediately, no warning.
- **No Auto Updates:** Users must manually download new versions.

## 8. Testing Plan

### Unit Tests

- FFT computation & peak extraction.
- Database storage & retrieval performance.
- Confidence smoothing algorithm.
- Audio input handling (mic & file).

### Integration Tests

- End-to-end recognition flow.
- GUI responsiveness under load.
- Handling of incomplete/missing dataset entries.

### User Testing

- Verify recognition speed & accuracy across different pieces.
- Ensure UI elements update correctly in real-time.
- Evaluate performance under different input qualities.
