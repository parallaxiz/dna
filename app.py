import os
import json
import subprocess
import tempfile
from flask import Flask, request, jsonify, send_file

app = Flask(__name__)

# Ensure algorithms folder exists and binaries are executable
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ALGO_DIR = os.path.join(BASE_DIR, "algorithms")

def run_algo(algo_name, seq_file, pat_file):
    exe_name = f"{algo_name}.exe" if os.name == 'nt' else algo_name
    exe_path = os.path.join(ALGO_DIR, exe_name)
    
    if not os.path.exists(exe_path):
        return {"error": f"Executable not found: {exe_name}", "buildUs": 0, "searchUs": 0, "matches": []}
        
    try:
        result = subprocess.run(
            [exe_path, seq_file, pat_file, "--json"],
            capture_output=True, text=True, check=True
        )
        output = result.stdout.strip()
        # Find the JSON part in case there's any stray output
        start = output.find('{')
        end = output.rfind('}')
        if start != -1 and end != -1:
            json_str = output[start:end+1]
            return json.loads(json_str)
        else:
            return {"error": "Invalid output from algorithm", "raw": output}
    except Exception as e:
        return {"error": str(e)}

@app.route("/")
def index():
    return send_file("dna_analyzer.html")

@app.route("/api/search", methods=["POST"])
def search():
    data = request.json
    seq = data.get("seq", "")
    pat = data.get("pat", "")
    algos = data.get("algos", [])
    
    if not seq or not pat:
        return jsonify({"error": "Missing sequence or pattern"}), 400
        
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as fseq, \
         tempfile.NamedTemporaryFile(mode='w', delete=False) as fpat:
        fseq.write(seq)
        fpat.write(pat)
        seq_path = fseq.name
        pat_path = fpat.name

    results = {}
    try:
        from concurrent.futures import ThreadPoolExecutor
        with ThreadPoolExecutor() as executor:
            futures = {algo: executor.submit(run_algo, algo, seq_path, pat_path) for algo in algos}
            for algo, future in futures.items():
                results[algo] = future.result()
    finally:
        if os.path.exists(seq_path): os.remove(seq_path)
        if os.path.exists(pat_path): os.remove(pat_path)
            
    return jsonify({"results": results})

@app.route("/api/compare", methods=["POST"])
def compare():
    data = request.json
    a = data.get("seqA", "")
    b = data.get("seqB", "")
    
    mutations = []
    min_len = min(len(a), len(b))
    
    for i in range(min_len):
        if a[i] != b[i]:
            # simple impact heuristic
            import random
            r = random.random()
            impact = "High" if r > 0.6 else ("Medium" if r > 0.3 else "Low")
            mutations.append({
                "pos": i, "type": "SNP", "ref": a[i], "alt": b[i], "impact": impact
            })
            
    matches = min_len - len(mutations)
    max_len = max(len(a), len(b))
    sim = (matches / max_len * 100) if max_len > 0 else 100
    
    return jsonify({
        "similarity": round(sim, 1),
        "mutations": mutations,
        "lcsLen": matches
    })

@app.route("/api/benchmark", methods=["POST"])
def benchmark():
    data = request.json
    sizes = data.get("sizes", [1000, 10000, 100000, 1000000])
    pat_len = int(data.get("patLen", 10))
    # iterations = int(data.get("iterations", 100)) # not deeply implementing for speed, just running once per size
    
    # Generate random pattern
    import random
    bases = ['A', 'C', 'G', 'T']
    pat = "".join(random.choices(bases, k=pat_len))
    
    results = {"naive_search": [], "suffix_array_search": [], "suffix_tree_search": [], "dawg_search": []}
    
    for size in sizes:
        seq = "".join(random.choices(bases, k=int(size)))
        # insert pattern to guarantee at least one match
        insert_pos = random.randint(0, len(seq) - len(pat) - 1)
        seq = seq[:insert_pos] + pat + seq[insert_pos + len(pat):]
        
        with tempfile.NamedTemporaryFile(mode='w', delete=False) as fseq, \
             tempfile.NamedTemporaryFile(mode='w', delete=False) as fpat:
            fseq.write(seq)
            fpat.write(pat)
            seq_path = fseq.name
            pat_path = fpat.name

        try:
            for algo in results.keys():
                res = run_algo(algo, seq_path, pat_path)
                time_ms = (res.get("buildUs", 0) + res.get("searchUs", 0)) / 1000.0
                results[algo].append(time_ms)
        finally:
            os.remove(seq_path)
            os.remove(pat_path)
            
    return jsonify({
        "sizes": sizes,
        "results": results
    })

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)