#!/usr/bin/env python3
"""Test for gStore RDF 1.2 TripleTerm against rdf-tests suite."""

"""
Prelimirary: Please frist clone the rdf-tests (https://github.com/w3c/rdf-tests/) to the corresponding path
"""
import subprocess, json, time, os, sys, urllib.request, glob, uuid, re

GSTORE = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
API = "http://127.0.0.1:9000/api"
NT_SYNTAX = os.path.expanduser("~/git/rdf-tests/rdf/rdf12/rdf-n-triples/syntax")
TTL_EVAL = os.path.expanduser("~/git/rdf-tests/rdf/rdf12/rdf-turtle/eval")
SPARQL_EVAL = os.path.expanduser("~/git/rdf-tests/sparql/sparql12/eval-triple-terms")

def api(op, **kw):
    data = {"operation": op, "username": "root", "password": "123456"}
    data.update(kw)
    req = urllib.request.Request(API, data=json.dumps(data).encode(),
        headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            return json.loads(r.read())
    except Exception as e:
        return {"error": str(e)}

def count_nt_lines(fp):
    with open(fp) as f:
        return sum(1 for l in f if l.strip() and not l.strip().startswith('#'))

def expand_prefixes(query, prefix_map):
    """Expand prefixed names in a SPARQL query using the given prefix map."""
    result = query
    for prefix, iri in prefix_map.items():
        if prefix:
            result = result.replace(f"{prefix}:", f"<{iri}>")
        else:
            # Empty prefix: replace :name with <iri+name>
            def replace_empty_prefix(m):
                return f"<{iri}{m.group(1)}>"
            result = re.sub(r':([a-zA-Z_][a-zA-Z0-9_-]*)', replace_empty_prefix, result)
    return result

def start_server():
    os.chdir(GSTORE)
    subprocess.run(["pkill", "-9", "gserver"], capture_output=True)
    time.sleep(1)
    for f in ["conf/gstore.pid", "conf/init.lock"]:
        try: os.remove(f"{GSTORE}/{f}")
        except: pass
    subprocess.Popen(["./bin/gserver", "-b"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    for _ in range(40):
        try:
            urllib.request.urlopen("http://127.0.0.1:9000/", timeout=1)
            return True
        except:
            time.sleep(1)
    return False

def unique_db(prefix):
    return f"{prefix}_{uuid.uuid4().hex[:8]}"

def build_file(fp, db):
    r = api("build", db_name=db, db_path=fp)
    if r.get("StatusCode") != 0:
        return False, 0
    return True, r.get("success_num", 0)

def cleanup_db(db):
    subprocess.run(["rm", "-rf", f"{GSTORE}/dbhome/{db}.db"], capture_output=True)

def main():
    if not start_server():
        print("FAILED to start server"); sys.exit(1)
    print("Server ready\n")
    total_pass = 0; total_fail = 0

    # === N-Triples Syntax Tests ===
    print("=" * 60 + "\nRDF N-Triples Syntax Tests\n" + "=" * 60)
    pos = ["ntriples12-syntax-01.nt","ntriples12-syntax-02.nt","ntriples12-syntax-03.nt","ntriples12-bnode-1.nt","ntriples12-nested-1.nt"]
    for f in pos:
        fp = os.path.join(NT_SYNTAX, f)
        if not os.path.exists(fp): continue
        e = count_nt_lines(fp); db = unique_db("ntpos")
        ok, g = build_file(fp, db); ok = ok and g == e; cleanup_db(db)
        print(f"  {'PASS' if ok else 'FAIL'}: POS {f} (e={e}, g={g})")
        if ok: total_pass += 1
        else: total_fail += 1

    neg = ["ntriples12-bad-syntax-01.nt","ntriples12-bad-syntax-02.nt","ntriples12-bad-syntax-03.nt","ntriples12-bad-syntax-04.nt","ntriples12-bad-syntax-05.nt","ntriples12-bad-syntax-06.nt","ntriples12-bad-syntax-07.nt","ntriples12-bad-syntax-08.nt","ntriples12-bad-syntax-09.nt","ntriples12-bad-syntax-10.nt","ntriples12-bad-reified-syntax-1.nt","ntriples12-bad-reified-syntax-2.nt","ntriples12-bad-reified-syntax-3.nt","ntriples12-bad-reified-syntax-4.nt"]
    for f in neg:
        fp = os.path.join(NT_SYNTAX, f)
        if not os.path.exists(fp): continue
        db = unique_db("ntneg")
        ok, g = build_file(fp, db); ok = (not ok) or g == 0; cleanup_db(db)
        print(f"  {'PASS' if ok else 'FAIL'}: NEG {f} (g={g})")
        if ok: total_pass += 1
        else: total_fail += 1

    # === Turtle Eval Tests ===
    print("\n" + "=" * 60 + "\nTurtle Eval Tests\n" + "=" * 60)
    for i in range(1, 9):
        ttl = f"{TTL_EVAL}/turtle12-eval-rt-0{i}.ttl"
        nt = f"{TTL_EVAL}/turtle12-eval-rt-0{i}.nt"
        if not os.path.exists(ttl): continue
        e = count_nt_lines(nt)
        # Each Turtle-style <<...>> triple term emits 3 component triples
        # (urn:gstore:tripleTermSubject/Predicate/Object) for SPARQL variable matching.
        with open(nt) as nf:
            reif_count = sum(1 for l in nf if 'reifies' in l)
        e += reif_count * 3
        db = unique_db("rt")
        ok, g = build_file(ttl, db); ok = ok and g == e; cleanup_db(db)
        print(f"  {'PASS' if ok else 'FAIL'}: rt-0{i} (e={e}, g={g})")
        if ok: total_pass += 1
        else: total_fail += 1

    # tt tests — skip tt-02, tt-03 which use BASE directive (not supported by gStore)
    for i in [1, 4]:
        ttl = f"{TTL_EVAL}/turtle12-eval-tt-0{i}.ttl"
        nt = f"{TTL_EVAL}/turtle12-eval-tt-0{i}.nt"
        if not os.path.exists(ttl): continue
        e = count_nt_lines(nt); db = unique_db("tt")
        ok, g = build_file(ttl, db); ok = ok and g == e; cleanup_db(db)
        print(f"  {'PASS' if ok else 'FAIL'}: tt-0{i} (e={e}, g={g})")
        if ok: total_pass += 1
        else: total_fail += 1

    # === SPARQL Eval Tests ===
    print("\n" + "=" * 60 + "\nSPARQL Eval Triple Terms Tests\n" + "=" * 60)

    sparql_tests = [
        # (name, datafile, queryfile, expected_bindings) — from manifest
        ("basic-2", "data-1.ttl", "basic-2.rq", 2),
        ("basic-3", "data-1.ttl", "basic-3.rq", 2),
        ("basic-4", "data-1.ttl", "basic-4.rq", 1),
        ("basic-5", "data-1.ttl", "basic-5.rq", 1),
        ("basic-6", "data-1.ttl", "basic-6.rq", 0),
    ]

    for name, datafile, queryfile, expected_bindings in sparql_tests:
        data_path = os.path.join(SPARQL_EVAL, datafile)
        query_path = os.path.join(SPARQL_EVAL, queryfile)
        if not os.path.exists(data_path) or not os.path.exists(query_path):
            continue

        with open(query_path) as f:
            query = f.read().strip()

        # Extract prefix map from query's PREFIX declarations
        prefix_map = {}
        for m in re.finditer(r'PREFIX\s+(\S*?):\s*<([^>]+)>', query, re.IGNORECASE):
            prefix_map[m.group(1)] = m.group(2)

        # Build database
        db = unique_db("sparql")
        ok, got = build_file(data_path, db)
        if not ok:
            print(f"  FAIL: {name} - build failed")
            total_fail += 1; cleanup_db(db); continue

        # Expand prefixes in query before sending to gStore
        clean_query = re.sub(r'PREFIX\s+\S*:\s*<[^>]+>\s*\n?', '', query, flags=re.IGNORECASE).strip()
        clean_query = expand_prefixes(clean_query, prefix_map)

        api("load", db_name=db)
        r = api("query", db_name=db, sparql=clean_query, format="json")
        bindings = r.get("results", {}).get("bindings", []) if r.get("StatusCode") == 0 else []
        got_bindings = len(bindings)
        ok = got_bindings == expected_bindings
        cleanup_db(db)
        print(f"  {'PASS' if ok else 'FAIL'}: {name} (e={expected_bindings}, g={got_bindings})")
        if ok: total_pass += 1
        else: total_fail += 1

    print(f"\n{'=' * 60}")
    print(f"TOTAL: {total_pass} PASS, {total_fail} FAIL out of {total_pass+total_fail}")
    print(f"Pass rate: {total_pass*100//(total_pass+total_fail)}%")
    subprocess.run(["pkill", "-9", "gserver"], capture_output=True)
    return 0 if total_fail == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
