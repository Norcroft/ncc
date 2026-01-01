#!/usr/bin/env python3
# Lightweight C/C++ test runner for Norcroft.
# - Discovers tests under tests/**/*.c,*.cpp by default
# - Reads inline directives from // comments
# - Executes RUN lines with %s (source), %t (any temp file), %cc (C), %cxx (C++)
#
# Examples:
#   ./runtests.py --root tests --features explicit,namespaces -j8 --junit out.xml
#   ./runtests.py --echo -v
#
# Directives (examples):
#   // RUN: %cxx %s -c              (optional if default isn't suitable)
#   // EXPECT-ERROR                 (check return code != 0)
#   // KNOWN-FAIL: fails because... (don't highlight as a FAIL)

#   // CHECK: __ct__Fi  (ordered check against stdout)
#   // CHECK-NO: foo    (must not appear in stdout between surrounding CHECK matches)
#
#   // CHECK-ERR: Warning... (ordered check against stderr)
#   // CHECK-ERR-NOT: no-more (must not appear in stderr between surrounding CHECK-ERR matches)
#
#   // REQUIRES: explicit, namespaces (skip test if not in --features list)

import argparse
import os
import re
import shlex
import sys
import tempfile
import traceback
from dataclasses import dataclass, field
from multiprocessing import Pool, cpu_count
from subprocess import run, PIPE, CalledProcessError

# ----------------------------- parsing -----------------------------

RUN_RE           = re.compile(r'^\s*//\s*RUN:\s*(.+)$')
EXPECT_ERROR_RE  = re.compile(r'^\s*//\s*EXPECT-ERROR', re.I)
EXPECT_FAIL_RE   = re.compile(r'^\s*//\s*EXPECT-FAIL', re.I)  # legacy alias
CHECK_ERR_RE     = re.compile(r'^\s*//\s*CHECK-ERR:\s*(.+)$')
CHECK_ERR_NOT_RE = re.compile(r'^\s*//\s*CHECK-ERR-NOT:\s*(.+)$')
CHECK_RE         = re.compile(r'^\s*//\s*CHECK:\s*(.+)$')
CHECK_NO_RE      = re.compile(r'^\s*//\s*CHECK-NO:\s*(.+)$')
REQUIRES_RE      = re.compile(r'^\s*//\s*REQUIRES:\s*(.+)$')
KNOWN_FAIL_RE    = re.compile(r'^\s*//\s*KNOWN-FAIL(?:\s*:\s*(.*))?\s*$', re.I)

SRC_EXTS = ('.c', '.cc', '.cpp')

@dataclass
class TestSpec:
    path:           str
    runs:           list[str] = field(default_factory=list)
    expect_success: bool = True

    check_ops:      list[tuple[str, str]] = field(default_factory=list)  # ('CHECK'|'CHECK-NO', pattern) in source order
    check_err_ops:  list[tuple[str, str]] = field(default_factory=list)  # ('CHECK-ERR'|'CHECK-ERR-NOT', pattern) in source order


    requires:       set[str] = field(default_factory=set)
    force_asm:      bool = False
    xfail_reason:   str | None = None

def parse_test(path: str) -> TestSpec:
    spec = TestSpec(path=path)

    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            if m := RUN_RE.match(line):
                spec.runs.append(m.group(1).strip())
                continue
            if EXPECT_ERROR_RE.match(line) or EXPECT_FAIL_RE.match(line):
                spec.expect_success = False
                continue
            if m := CHECK_ERR_RE.match(line):
                spec.check_err_ops.append(('CHECK-ERR', m.group(1)))
                continue
            if m := CHECK_ERR_NOT_RE.match(line):
                spec.check_err_ops.append(('CHECK-ERR-NOT', m.group(1)))
                continue
            if m := CHECK_RE.match(line):
                spec.check_ops.append(('CHECK', m.group(1)))
                continue
            if m := CHECK_NO_RE.match(line):
                spec.check_ops.append(('CHECK-NO', m.group(1)))
                continue
            if m := REQUIRES_RE.match(line):
                toks = [t.strip() for t in re.split(r'[,\s]+', m.group(1)) if t.strip()]
                spec.requires.update(toks)
                continue
            if m := KNOWN_FAIL_RE.match(line):
                reason = (m.group(1) or '').strip()
                spec.xfail_reason = reason or " "
                continue
    return spec

# --------------------------- execution ----------------------------

@dataclass
class TestResult:
    path: str
    status: str      # PASS/FAIL/SKIP/KNOWN-FAIL[/XPASS - internal for unexpected pass]
    details: str = ""
    stdout: str = ""
    stderr: str = ""

def substitute(cmd: str, src: str, tempstem: str, c_compiler: str, cxx_compiler: str) -> str:
    abs_src = os.path.abspath(src)
    cmd = cmd.replace('%s', shlex.quote(abs_src))
    cmd = cmd.replace('%t', tempstem)
    cmd = cmd.replace('%cxx', shlex.quote(cxx_compiler))
    cmd = cmd.replace('%cc', shlex.quote(c_compiler))
    return cmd

def run_one(spec: TestSpec, features: set[str], echo: bool, c_compiler: str, cxx_compiler: str, dump_checked: bool) -> TestResult:
    # gating
    if spec.requires and not spec.requires.issubset(features):
        return TestResult(spec.path, 'SKIP',
                          details=f"Missing required features: {', '.join(sorted(spec.requires - features))}")

    # temp dir and stem
    tmpdir = tempfile.mkdtemp(prefix='rt-')
    tempstem = os.path.join(tmpdir, 't')
    transcript = os.path.join(tmpdir, 'transcript.txt')

    all_stdout = []
    all_stderr = []
    last_rc = 0

    try:
        if not spec.runs:
            ext = os.path.splitext(spec.path)[1].lower()
            if ext == '.c':
                cmd = "%cc %s -c -o %t.o"
            else:
                cmd = "%cxx %s -c -o %t.o"
            spec.runs = [cmd]

        for raw in spec.runs:
            cmd = substitute(raw, spec.path, tempstem, c_compiler, cxx_compiler)

            # Auto-append Norcroft VFP ABI for tests under /vfp/
            try:
                from pathlib import PurePath
                parts = set(PurePath(spec.path).parts)
                in_vfp_suite = ('vfp' in parts)
            except Exception:
                # Fallback: simple substring check
                in_vfp_suite = (f"{os.sep}vfp{os.sep}" in spec.path)

            # Only append if not already present in the command
            if in_vfp_suite and ('-apcs' not in cmd):
                cmd += ' -apcs 3/32bit/fpregargs/narrow -fpu vfp'

            # If compile-only (-c) and no explicit -o is present, add one targeting the temp stem
            if re.search(r'(?:(?<=\s)|^)\-c(?=\s|$)', cmd) and not re.search(r'(?:(?<=\s)|^)\-o(\s|[^\s])', cmd):
                cmd += f" -o {tempstem}.o"

            if echo:
                print(f"[ECHO] cwd={tmpdir}\n[ECHO] RUN: {cmd}")
            try:
                with open(transcript, 'a', encoding='utf-8') as tf:
                    tf.write(f"RUN: {cmd}\n")
            except Exception:
                pass

            # Use /bin/bash -lc to allow pipes, redirects, &&, etc in RUN:..
            completed = run(cmd, shell=True, cwd=tmpdir, stdout=PIPE, stderr=PIPE, text=True)
            all_stdout.append(completed.stdout)
            all_stderr.append(completed.stderr)
            last_rc = completed.returncode

            # Keep running any subsequent RUN lines regardless, so we can collect more outputs

        stdout = ''.join(all_stdout)
        stderr = ''.join(all_stderr)

        def finalise(status, details):
            nonlocal stdout, stderr
            # KNOWN-FAIL handling: if we FAILED but test is KNOWN-FAIL, mark KNOWN-FAIL
            if status == 'FAIL' and spec.xfail_reason:
                return TestResult(spec.path, 'KNOWN-FAIL', details=spec.xfail_reason, stdout=stdout, stderr=stderr)
            return TestResult(spec.path, status, details=details, stdout=stdout, stderr=stderr)

        def _attach_checked(tag: str, text: str, limit: int = 20000):
            """Append checked text to stdout to make --verbose useful (ie. move stderr to stdout)."""
            nonlocal stdout
            if not (dump_checked or os.environ.get('RT_DUMP_CHECKED')):
                return
            if text is None:
                return
            t = text
            if len(t) > limit:
                t = t[:limit] + "\n... <truncated> ...\n"
            stdout += f"\n[checked:{tag}]\n" + t + "\n"

        # EXPECTation on exit codes (based on the *last* RUN line only).
        # NOTE: If a test has multiple RUN lines, an earlier failure followed by
        # a later success will currently be treated as success.
        if spec.expect_success and last_rc != 0:
            return finalise('FAIL', "Expected success (0) but last command failed with rc=%d" % last_rc)
        elif (not spec.expect_success) and last_rc == 0:
            return finalise('FAIL', "Expected non-zero exit status but last command succeeded (rc=0)")

        # Ordered substring helpers shared by CHECK and CHECK-ERR
        import re as _re
        def _normalize_ws(s: str) -> str:
            # Collapse runs of spaces/tabs to a single space, but preserve newlines,
            # then trim leading/trailing whitespace to make matching robust against
            # stray spaces at line ends.
            return _re.sub(r'[ \t]+', ' ', s).strip()

        def _ordered_match(hay: str, patterns, normalize: bool):
            """
            Ordered substring search.
            If normalize is True, both haystack and patterns have runs of spaces/tabs
            collapsed and leading/trailing whitespace stripped. This matches the
            semantics used for CHECK.

            Returns (end_pos, missing_pattern):
              - end_pos: index just after the last match in the processed haystack,
                         or 0 if there were no patterns.
              - missing_pattern: the first pattern that failed to match, or None on success.
            """
            hay_proc = _normalize_ws(hay) if normalize else hay
            pos = 0
            if not patterns:
                return 0, None
            for p in patterns:
                pat_proc = _normalize_ws(p) if normalize else p
                idx = hay_proc.find(pat_proc, pos)
                if idx < 0:
                    return 0, p
                pos = idx + len(pat_proc)
            return pos, None

        def _check_ops_windowed(hay: str,
                                ops: list[tuple[str, str]],
                                attach_tag: str,
                                check_kind: str,
                                no_kind: str):
            """Validate CHECK/CHECK-NO style ops against `hay`.

            Semantics (FileCheck-style):
              - `check_kind` ops are matched in order
              - `no_kind` ops are forbidden between the end of the previous match and the start of the next match
              - `no_kind` does not advance the match position
              - Trailing `no_kind` applies to the remainder after the last match

            Returns (ok: bool, fail_details: str). On failure it will also attach the checked text
            to stdout when --verbose/RT_DUMP_CHECKED is enabled.
            """
            if not ops:
                return True, ''

            hay_proc = _normalize_ws(hay)
            pos = 0
            pending_no: list[str] = []

            for kind, pat in ops:
                if kind == no_kind:
                    pending_no.append(_normalize_ws(pat))
                    continue

                # kind == check_kind
                want = _normalize_ws(pat)
                idx = hay_proc.find(want, pos)
                if idx < 0:
                    _attach_checked(attach_tag, hay)
                    return False, f"{check_kind} not found: {pat!r}"

                # Enforce any pending NO patterns between pos..idx
                if pending_no:
                    window = hay_proc[pos:idx]
                    for pno in pending_no:
                        if window.find(pno) >= 0:
                            _attach_checked(attach_tag, hay)
                            return False, f"{no_kind} violated before {pat!r}: {pno!r}"
                    pending_no.clear()

                pos = idx + len(want)

            # Trailing NO patterns apply from the last CHECK to end
            if pending_no:
                window = hay_proc[pos:]
                for pno in pending_no:
                    if window.find(pno) >= 0:
                        _attach_checked(attach_tag, hay)
                        return False, f"{no_kind} violated after final {check_kind}: {pno!r}"

            return True, ''

        # CHECK-ERR / CHECK-ERR-NOT on stderr (LLVM FileCheck style)
        ok, details = _check_ops_windowed(stderr,
                                          spec.check_err_ops,
                                          attach_tag='<stderr>',
                                          check_kind='CHECK-ERR',
                                          no_kind='CHECK-ERR-NOT')
        if not ok:
            return finalise('FAIL', details)

        # CHECK / CHECK-NO on stdout (LLVM FileCheck style)
        ok, details = _check_ops_windowed(stdout,
                                          spec.check_ops,
                                          attach_tag='<stdout>',
                                          check_kind='CHECK',
                                          no_kind='CHECK-NO')
        if not ok:
            return finalise('FAIL', details)

        # If we got here, it's a nominal PASS — but check if it's a KNOWN-FAIL.
        status = 'PASS'
        details = ''
        if spec.xfail_reason:
            status = 'XPASS'
            details = f"Unexpected pass (was KNOWN-FAIL: {spec.xfail_reason})"

        return TestResult(spec.path, status, details, stdout, stderr)

    except Exception as e:
        return TestResult(spec.path, 'FAIL', details=f"Exception: {e}\n{traceback.format_exc()}")
    finally:
        # keep artifacts in tmpdir for debugging if the test failed; otherwise clean up
        pass  # Intentionally keep; easy to browse tmp dirs after failures

# --------------------------- discovery ----------------------------

def discover(root: str) -> list[str]:
    files = []
    for dirpath, _dirs, filenames in os.walk(root):
        for name in filenames:
            if name.endswith(SRC_EXTS):
                files.append(os.path.join(dirpath, name))
    files.sort()
    return files

# --------------------------- reporting ----------------------------

def colour(s, code):
    if not sys.stdout.isatty():
        return s
    return f"\033[{code}m{s}\033[0m"

def summarize(results: list[TestResult]):
    counts = {'PASS':0,'FAIL':0,'SKIP':0,'KNOWN-FAIL':0,'XPASS':0}
    for r in results:
        counts[r.status] = counts.get(r.status,0)+1
    line = f"{counts['PASS']} passed, {counts['KNOWN-FAIL']} known-fail, {counts['XPASS']} unexpected-pass, {counts['SKIP']} skipped, {counts['FAIL']} failed"
    if counts['FAIL'] or counts['XPASS']:
        print(colour(line, "31"))  # red
    else:
        print(colour(line, "32"))  # green
    return counts

def print_result(r: TestResult, verbose: bool):
    status_colour = {"PASS":"32","FAIL":"31","SKIP":"33","KNOWN-FAIL":"36","XPASS":"35"}[r.status]
    rel = r.path
    print(f"{colour(r.status,status_colour)} {rel}{' - '+r.details if r.details else ''}")
    if verbose and r.status in ('FAIL','XPASS'):
        if r.stdout.strip():
            print(colour("  --- stdout ---","90"))
            print(indent(r.stdout, "  "))
        if r.stderr.strip():
            print(colour("  --- stderr ---","90"))
            print(indent(r.stderr, "  "))

def indent(s: str, pre: str) -> str:
    return ''.join(pre + line for line in s.splitlines(True))

def write_junit(results: list[TestResult], path: str):
    # Minimal JUnit XML
    total = len(results)
    failures = sum(1 for r in results if r.status in ('FAIL','XPASS'))
    skips = sum(1 for r in results if r.status == 'SKIP')
    cases = []
    for r in results:
        classname = os.path.dirname(r.path).replace(os.sep,'.') or 'tests'
        name = os.path.basename(r.path)
        case = f'<testcase classname="{xml_escape(classname)}" name="{xml_escape(name)}">'
        if r.status == 'SKIP':
            case += f'<skipped message="{xml_escape(r.details)}"/>'
        elif r.status in ('FAIL','XPASS'):
            msg = r.details or r.status
            body = ''
            if r.stdout:
                body += "\n[stdout]\n" + r.stdout
            if r.stderr:
                body += "\n[stderr]\n" + r.stderr
            case += f'<failure message="{xml_escape(msg)}">{xml_escape(body)}</failure>'
        case += '</testcase>'
        cases.append(case)
    xml = f'<?xml version="1.0" encoding="UTF-8"?><testsuite name="runtests" tests="{total}" failures="{failures}" skipped="{skips}">' + ''.join(cases) + '</testsuite>'
    with open(path, 'w', encoding='utf-8') as f:
        f.write(xml)

def xml_escape(s: str) -> str:
    return (s.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")
             .replace('"',"&quot;").replace("'","&apos;"))

# ----------------------------- main ------------------------------

def main():
    p = argparse.ArgumentParser(description="Norcroft test runner.")
    p.add_argument('--root', default='tests', help='Root directory to discover tests (default: tests)')
    p.add_argument('--features', default='', help='Comma-separated feature flags (e.g., explicit,namespaces)')
    p.add_argument('-j','--jobs', type=int, default=max(1, min(8, cpu_count()//2)), help='Parallel jobs (default: ~half cores)')
    p.add_argument('-v','--verbose', action='store_true', help='Show stdout/stderr for failing/unexp pass tests')
    p.add_argument('--junit', default='', help='Write JUnit XML to this file')
    p.add_argument('--cc', default='bin/ncc-riscos', help='C compiler to invoke for %%cc')
    p.add_argument('--cxx', default='bin/n++-riscos', help='C++ compiler to invoke for %%cxx')
    p.add_argument('--echo', action='store_true', help='Echo each expanded RUN command and its temp dir before executing')
    p.add_argument('paths', nargs='*', help='Optional list of test files or directories. If omitted, uses --root.')
    p.add_argument('--match', default='', help='Only run tests whose path contains this substring (case-sensitive).')
    args = p.parse_args()

    # Resolve non-absolute compiler paths against this script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if not os.path.isabs(args.cc):
        args.cc = os.path.join(script_dir, args.cc)
    if not os.path.isabs(args.cxx):
        args.cxx = os.path.join(script_dir, args.cxx)

    features = set([t.strip() for t in args.features.split(',') if t.strip()])

    selected: list[str] = []
    if args.paths:
        for pth in args.paths:
            pth = os.path.normpath(pth)
            if os.path.isdir(pth):
                selected.extend(discover(pth))
            elif os.path.isfile(pth):
                selected.append(pth)
            else:
                print(f"No such file or directory: {pth}", file=sys.stderr)
                return 2
    else:
        selected = discover(args.root)

    if args.match:
        selected = [t for t in selected if args.match in t]

    # De-dup and sort for stable output
    tests = sorted(set(selected))

    if not tests:
        where = ' '.join(args.paths) if args.paths else args.root
        msg = f"No tests matched under {where}"
        if args.match:
            msg += f" (match={args.match!r})"
        print(msg, file=sys.stderr)
        return 2

    # Pre-parse all specs first, so parse errors show early
    specs = [parse_test(t) for t in tests]

    work = [(s, features, args.echo, args.cc, args.cxx, args.verbose) for s in specs]
    if args.jobs > 1:
        with Pool(processes=args.jobs) as pool:
            results = pool.starmap(run_one, work)
    else:
        results = [run_one(*w) for w in work]

    # Print per-test results
    for r in results:
        print_result(r, args.verbose)

    # Summary and exit code
    counts = summarize(results)
    if args.junit:
        write_junit(results, args.junit)

    # Fail the run if any FAIL or XPASS
    exit_code = 0 if (counts.get('FAIL',0) == 0 and counts.get('XPASS',0) == 0) else 1
    sys.exit(exit_code)

if __name__ == '__main__':
    main()
