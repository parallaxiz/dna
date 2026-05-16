import os
import json
import subprocess
import tempfile
from flask import Flask, request, jsonify, send_file

app = Flask(__name__)

# Ensure algorithms folder exists and binaries are executable
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ALGO_DIR = os.path.join(BASE_DIR, "algorithms")
import hashlib

BUILDS_DIR = os.path.join(BASE_DIR, "builds")
if not os.path.exists(BUILDS_DIR): os.makedirs(BUILDS_DIR)

def run_algo(algo_name, seq_file, pat_file, seq_hash=None):
    exe_name = f"{algo_name}.exe" if os.name == 'nt' else algo_name
    exe_path = os.path.join(ALGO_DIR, exe_name)
    
    if not os.path.exists(exe_path):
        return {"error": f"Executable not found: {exe_name}", "buildUs": 0, "searchUs": 0, "matches": []}
        
    cmd = [exe_path, seq_file, pat_file, "--json"]
    
    # Persistent Build Logic
    if seq_hash and algo_name != "naive_search":
        cache_path = os.path.join(BUILDS_DIR, f"{algo_name}_{seq_hash}.bin")
        if os.path.exists(cache_path):
            cmd += ["--load", cache_path]
        else:
            cmd += ["--save", cache_path]
            
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        output = result.stdout.strip()
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
        
    # Hash the sequence to identify it uniquely
    seq_hash = hashlib.sha256(seq.encode()).hexdigest()[:16]
        
    fseq = tempfile.NamedTemporaryFile(mode='w', delete=False)
    fpat = tempfile.NamedTemporaryFile(mode='w', delete=False)
    try:
        fseq.write(seq)
        fpat.write(pat)
        fseq.close()
        fpat.close()
        
        seq_path = fseq.name
        pat_path = fpat.name

        results = {}
        from concurrent.futures import ThreadPoolExecutor
        with ThreadPoolExecutor() as executor:
            futures = {algo: executor.submit(run_algo, algo, seq_path, pat_path, seq_hash) for algo in algos}
            for algo, future in futures.items():
                results[algo] = future.result()
    finally:
        if 'seq_path' in locals() and os.path.exists(seq_path): os.remove(seq_path)
        if 'pat_path' in locals() and os.path.exists(pat_path): os.remove(pat_path)
            
    return jsonify({"results": results})

@app.route("/api/compare", methods=["POST"])
def compare():
    data = request.json
    a = data.get("seqA", "")
    b = data.get("seqB", "")
    
    if not a or not b:
        return jsonify({"similarity": 0, "mutations": [], "lcsLen": 0})

    import tempfile
    import json
    
    with tempfile.NamedTemporaryFile(mode='w', delete=False) as fa, \
         tempfile.NamedTemporaryFile(mode='w', delete=False) as fb:
        fa.write(a)
        fb.write(b)
        a_path = fa.name
        b_path = fb.name

    exe_name = "sequence_compare.exe" if os.name == 'nt' else "sequence_compare"
    exe_path = os.path.join(ALGO_DIR, exe_name)

    try:
        if not os.path.exists(exe_path):
            return jsonify({"error": "Executable not found", "similarity": 0, "mutations": [], "lcsLen": 0})
            
        result = subprocess.run(
            [exe_path, a_path, b_path, "--json"],
            capture_output=True, text=True, check=True
        )
        output = result.stdout.strip()
        start = output.find('{')
        end = output.rfind('}')
        if start != -1 and end != -1:
            json_str = output[start:end+1]
            res_data = json.loads(json_str)
            res_data["similarity"] = round(res_data.get("similarity", 0), 1)
            return jsonify(res_data)
        else:
            return jsonify({"error": "Invalid output", "similarity": 0, "mutations": [], "lcsLen": 0})
    except Exception as e:
        return jsonify({"error": str(e), "similarity": 0, "mutations": [], "lcsLen": 0})
    finally:
        if os.path.exists(a_path): os.remove(a_path)
        if os.path.exists(b_path): os.remove(b_path)

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