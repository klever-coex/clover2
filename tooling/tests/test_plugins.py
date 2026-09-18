import pathlib

import click
import pytest

from clover2_tooling.diagnostics import build_argv as diag_argv
from clover2_tooling.settings import DEFAULT_SCHEMA, resolve, resolve_schema
from clover2_tooling.settings import build_argv as settings_argv


def test_settings_argv():
    argv = settings_argv(pathlib.Path("/s.yaml"), pathlib.Path("/c.yaml"))
    assert argv == ["run", "clover2_ui", "settings", "/s.yaml", "/c.yaml"]


def test_diagnostics_argv_defaults():
    assert diag_argv("/diagnostics_agg", False, ()) == [
        "run", "clover2_ui", "diagnostics", "--topic", "/diagnostics_agg"]


def test_diagnostics_argv_sim_time_and_passthrough():
    argv = diag_argv("/t", True, ("--ros-args", "--log-level", "debug"))
    assert argv == ["run", "clover2_ui", "diagnostics", "--topic", "/t",
                    "--ros-args", "-p", "use_sim_time:=true",
                    "--ros-args", "--log-level", "debug"]


def test_resolve_explicit_flag(tmp_path: pathlib.Path):
    assert resolve(None, tmp_path / "x.yaml", "ignored.yaml") == tmp_path / "x.yaml"


def test_resolve_configured_relative_to_root(tmp_path: pathlib.Path):
    assert resolve(tmp_path, None, "src/schema.yaml") == tmp_path / "src" / "schema.yaml"


def test_resolve_no_root_uses_cwd(tmp_path: pathlib.Path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    assert resolve(None, None, "config.yaml") == tmp_path / "config.yaml"


def test_resolve_schema_explicit_flag(tmp_path: pathlib.Path):
    got = resolve_schema(tmp_path, tmp_path / "s.yaml", None)
    assert got == tmp_path / "s.yaml"


def test_resolve_schema_prefers_installed_share(tmp_path: pathlib.Path):
    installed = tmp_path / "share" / "clover2_bringup" / "schemas" / "klever5.yaml"
    installed.parent.mkdir(parents=True)
    installed.write_text("")
    checkout = tmp_path / "clover2_bringup" / "schemas" / "klever5.yaml"
    checkout.parent.mkdir(parents=True)
    checkout.write_text("")

    def fake_share(package):
        return tmp_path / "share" / "clover2_bringup"

    got = resolve_schema(tmp_path, None, "clover2_bringup/schemas/klever5.yaml",
                         find_share=fake_share)

    assert got == installed


def test_resolve_schema_without_ros2_uses_project_checkout(tmp_path: pathlib.Path):
    checkout = tmp_path / "clover2_bringup" / "schemas" / "klever5.yaml"
    checkout.parent.mkdir(parents=True)
    checkout.write_text("")

    got = resolve_schema(tmp_path, None, "clover2_bringup/schemas/klever5.yaml",
                         find_share=lambda pkg: None)
    assert got == checkout


def test_resolve_schema_no_checkout_defaults_to_project_path(
        tmp_path: pathlib.Path):
    got = resolve_schema(tmp_path, None, None, find_share=lambda pkg: None)
    assert got == tmp_path / DEFAULT_SCHEMA


def test_exec_ros_missing_ros2(monkeypatch):
    import clover2_tooling.ros as ros

    monkeypatch.setattr(ros.shutil, "which", lambda name: None)
    with pytest.raises(click.ClickException, match="ros2.*not found"):
        ros.exec_ros(["run", "clover2_ui", "diagnostics"])


def test_exec_ros_replaces_process(monkeypatch):
    import clover2_tooling.ros as ros

    calls = {}
    monkeypatch.setattr(ros.shutil, "which", lambda name: "/usr/bin/ros2")

    def fake_execvp(prog, argv):
        calls["prog"], calls["argv"] = prog, argv

    monkeypatch.setattr(ros.os, "execvp", fake_execvp)
    ros.exec_ros(["run", "clover2_ui", "diagnostics", "--topic", "/t"])
    assert calls == {"prog": "/usr/bin/ros2",
                     "argv": ["ros2", "run", "clover2_ui", "diagnostics",
                              "--topic", "/t"]}


def test_package_share_reads_ament_index(tmp_path, monkeypatch):
    import os

    import clover2_tooling.ros as ros

    prefix = tmp_path / "install" / "clover2_bringup"
    marker = (prefix / "share" / "ament_index" / "resource_index"
              / "packages" / "clover2_bringup")
    marker.parent.mkdir(parents=True)
    marker.write_text("")

    monkeypatch.setenv("AMENT_PREFIX_PATH", os.pathsep.join([
        str(tmp_path / "install" / "other"), str(prefix)]))

    assert ros.package_share("clover2_bringup") == prefix / "share" / "clover2_bringup"


def test_package_share_none_when_not_indexed(tmp_path, monkeypatch):
    import clover2_tooling.ros as ros

    monkeypatch.setenv("AMENT_PREFIX_PATH", str(tmp_path / "install"))
    assert ros.package_share("clover2_bringup") is None

    monkeypatch.delenv("AMENT_PREFIX_PATH")
    assert ros.package_share("clover2_bringup") is None


def test_plugins_expose_protocol():
    from clover2_tooling.diagnostics import plugin as diagnostics
    from clover2_tooling.settings import plugin as settings

    assert settings.name == "settings"
    assert diagnostics.name == "diagnostics"

    commands = settings.create_commands(
        type("C", (), {"config": {}, "root": None})())
    assert len(commands) == 1 and commands[0].name == "settings"
