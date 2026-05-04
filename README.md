# 🧬 DNA Intelligence Platform

A production-ready, high-performance genomic analysis suite. This platform integrates a **Flask** backend with high-speed **C++** pattern-searching algorithms and a premium, cyberpunk-inspired **JavaScript** dashboard.

## 🚀 Key Features

*   **Native C++ Engine**: High-performance implementations of Suffix Trees (Ukkonen's), Suffix Arrays, and Directed Acyclic Word Graphs (DAWG).
*   **Real-time Dashboard**: Interactive visualization of GC content, nucleotide distribution, and algorithm performance benchmarking.
*   **Genomic File Handling**: Dynamic file upload support with automatic sequence extraction and session-based persistence.
*   **Sequence Comparison**: SNP detection and similarity scoring between genomic sequences (e.g., SARS-CoV-2, BRCA1).
*   **Cyberpunk Aesthetics**: State-of-the-art UI with glassmorphism, dynamic gradients, and responsive data visualizations using Chart.js.

## 🛠️ Technology Stack

*   **Backend**: Python (Flask)
*   **Computation**: C++17 (Optimized binaries)
*   **Frontend**: HTML5, Vanilla CSS, JavaScript (ES6+)
*   **Visualization**: Chart.js
*   **Typography**: Orbitron, Rajdhani, and Share Tech Mono

## 📂 Project Structure

```text
├── app.py                  # Flask REST API server
├── dna_analyzer.html       # Single-page Application (UI & JS Logic)
├── algorithms/             # C++ Source and Optimized Binaries
│   ├── naive_search.cpp
│   ├── suffix_array_search.cpp
│   ├── suffix_tree_search.cpp
│   └── dawg_search.cpp
└── README.md
```

## ⚙️ Setup & Execution

### 1. Compile Algorithms
Ensure you have `g++` installed. From the root directory:
```bash
g++ -O3 -o algorithms/naive_search.exe algorithms/naive_search.cpp
g++ -O3 -o algorithms/suffix_array_search.exe algorithms/suffix_array_search.cpp
g++ -O3 -o algorithms/suffix_tree_search.exe algorithms/suffix_tree_search.cpp
g++ -O3 -o algorithms/dawg_search.exe algorithms/dawg_search.cpp
```

### 2. Run the Application
Install dependencies and start the Flask server:
```bash
pip install flask
python app.py
```
Visit `http://127.0.0.1:5000` in your browser.

## 📊 Algorithmic Performance

The platform compares O(n) preprocessing structures (Suffix Tree/DAWG) against baseline searching to demonstrate efficiency gains on large genomic datasets.
