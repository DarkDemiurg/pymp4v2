"""PEP 561 stubs in the installed package must match the live pybind11 surface."""

import ast
from pathlib import Path

import pymp4v2
import pymp4v2.raw as raw

_TYPING_MODULES = {"typing", "types", "enum", "__future__"}


def _public_names(obj):
    names = {name for name in dir(obj) if not name.startswith("_")}
    if getattr(obj, "__version__", None) is not None:
        names.add("__version__")
    return names


def _stub_names(path: Path):
    tree = ast.parse(path.read_text(encoding="utf-8"))
    names = set()
    for node in tree.body:
        if isinstance(node, (ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            names.add(node.name)
        elif isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id != "__all__":
                    names.add(target.id)
        elif (
            isinstance(node, ast.AnnAssign)
            and isinstance(node.target, ast.Name)
            and node.target.id != "__all__"
        ):
            names.add(node.target.id)
        elif isinstance(node, ast.ImportFrom):
            root = (node.module or "").split(".", 1)[0]
            if node.level == 0 and root in _TYPING_MODULES:
                continue
            for alias in node.names:
                names.add(alias.asname or alias.name)
    names.discard("*")
    return names


def test_pep561_marker_and_stubs_ship_in_package():
    pkg = Path(pymp4v2.__file__).resolve().parent
    assert (pkg / "py.typed").is_file()
    assert (pkg / "__init__.pyi").is_file()
    assert (pkg / "raw.pyi").is_file()
    assert (pkg / "py.typed").stat().st_size >= 0


def test_toplevel_stub_covers_live_exports():
    pkg = Path(pymp4v2.__file__).resolve().parent
    live = _public_names(pymp4v2)
    stub = _stub_names(pkg / "__init__.pyi")
    missing = live - stub
    extra = stub - live
    assert not missing, f"live names missing from __init__.pyi: {sorted(missing)}"
    assert not extra, f"__init__.pyi names not on live module: {sorted(extra)}"
    assert "Sample" in live
    assert "Sample" in stub


def test_raw_stub_covers_live_exports():
    pkg = Path(pymp4v2.__file__).resolve().parent
    live = _public_names(raw)
    stub = _stub_names(pkg / "raw.pyi")
    missing = live - stub
    extra = stub - live
    assert not missing, f"live names missing from raw.pyi: {sorted(missing)}"
    assert not extra, f"raw.pyi names not on live module: {sorted(extra)}"
