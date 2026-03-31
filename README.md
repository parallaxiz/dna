# DNA Sequence Analyzer

A high-performance, web-based genomic intelligence platform designed for real-time DNA sequence analysis, pattern matching, and mutation tracking. This application combines a Python Streamlit backend with a sophisticated, custom-engineered HTML5/JavaScript frontend to provide a cyberpunk-inspired, interactive diagnostic experience.

## 🧬 Key Features

* **Multi-Algorithm Pattern Search**: Compare the performance of various string-searching algorithms including Naive Search, Suffix Array, Suffix Tree (Ukkonen's), and Directed Acyclic Word Graph (DAWG).
* **Real-time Sequence Statistics**: Instant calculation of GC content, sequence length, and nucleotide composition (A, T, G, C distribution).
* **Sequence Comparison & Alignment**: Align two sequences to identify mutations (SNPs), calculate similarity percentages, and determine the Longest Common Subsequence (LCS).
* **Interactive Benchmarking**: Visualize search time vs. sequence length and memory usage across different data structures.
* **Version Control & Persistent Storage**: Track analysis history and view storage efficiency gains when using persistent data structures (delta-based storage).
* **Genomic File Support**: Upload and process `.fasta`, `.fa`, and `.fna` files directly in the browser.

## 🛠️ Technology Stack

* **Backend**: Streamlit (Python)
* **Frontend**: HTML5, CSS3 (Custom Cyberpunk UI), JavaScript (ES6+)
* **Data Visualization**: Chart.js
* **Typography**: Orbitron, Share Tech Mono, and Rajdhani Google Fonts

## 📂 Project Structure

```text
├── app.py              # Streamlit entry point; handles page config and UI injection
├── dna_analyzer.html   # Core application logic, styles, and interactive UI
└── pyrightconfig.json  # Python type-checking configuration
```

## 🚀 Getting Started

### Prerequisites

* Python 3.11+
* Streamlit

### Installation & Execution

1.  Clone the repository to your local machine.
2.  Ensure your environment matches the configuration in `pyrightconfig.json` if you require type-checking.
3.  Run the application using Streamlit:
    ```bash
    streamlit run app.py
    ```

## 📊 Algorithmic Overview

The platform is designed to showcase the efficiency of different genomic data structures:

| Algorithm | Complexity | Best For |
| :--- | :--- | :--- |
| **Suffix Tree** | O(m + k) | Multiple pattern searches on the same sequence |
| **Suffix Array** | O(m log n) | Memory-constrained environments (4n bytes) |
| **DAWG** | O(m) | Space-efficient pattern repetition |
| **Naive Search** | O(n × m) | Baseline comparison with no preprocessing |
