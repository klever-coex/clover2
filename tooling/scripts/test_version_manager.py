#!/usr/bin/env python3

import json
import pathlib
import subprocess
import sys
import tempfile
import unittest

SCRIPT = pathlib.Path(__file__).parent / "version_manager"

PACKAGE_XML = """<?xml version="1.0"?>
<package format="3">
  <name>{name}</name>
  <version>{version}</version>
</package>
"""

NPM_PACKAGE_JSON = """{
  "name": "clover2",
  "private": true,
  "version": "__VERSION__",
  "scripts": {
    "build": "vite build"
  }
}
"""


class VersionManagerTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = pathlib.Path(self._tmp.name)
        for name, version in [("clover2", "0.1.9"), ("clover2_extra", "0.1.9")]:
            pkg_dir = self.root / name
            pkg_dir.mkdir()
            (pkg_dir / "package.xml").write_text(
                PACKAGE_XML.format(name=name, version=version))
        (self.root / "frontend").mkdir()
        (self.root / "frontend" / "package.json").write_text(
            NPM_PACKAGE_JSON.replace("__VERSION__", "0.1.9+12513254"))

    def tearDown(self):
        self._tmp.cleanup()

    # // helpers

    def run_manager(self, *args: str) -> subprocess.CompletedProcess:
        return subprocess.run(
            [sys.executable, str(SCRIPT), "-d", str(self.root), *args],
            capture_output=True, text=True,
        )

    def run_checked(self, *args: str) -> str:
        result = self.run_manager(*args)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result.stdout.strip()

    def bump_json(self, *args: str) -> dict:
        return json.loads(self.run_checked("bump", *args, "--json"))

    def compose_json(self, *args: str) -> dict:
        return json.loads(self.run_checked("compose", *args, "--json"))

    def git(self, *args: str) -> None:
        result = subprocess.run(["git", "-C", str(self.root), *args],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def init_git_with_stable_tag(self):
        self.git("init", "-q", "-b", "main")
        self.git("config", "user.email", "test@example.com")
        self.git("config", "user.name", "test")
        self.git("add", "-A")
        self.git("commit", "-qm", "init")
        self.git("tag", "-a", "v0.1.9", "-m", "release")

    def assert_lockstep(self, expected: str) -> None:
        printed = self.run_checked("print")
        # print output is grouped by store type: header lines have no ": ",
        # version lines are "\t<name>: <version>"
        values = [line.split(": ")[1] for line in printed.splitlines() if ": " in line]
        self.assertTrue(values, f"No versions in print output:\n{printed}")
        self.assertEqual(set(values), {expected})
        npm = json.loads((self.root / "frontend" / "package.json").read_text())
        self.assertEqual(npm["version"], expected)

    # // stores & plain bumps

    def test_npm_store_bumped_with_formatting_kept(self):
        self.bump_json("patch")
        text = (self.root / "frontend" / "package.json").read_text()
        self.assertIn('"version": "0.1.10"', text)
        self.assertIn('"build": "vite build"', text)
        self.assertNotIn("+12513254", text)
        self.assert_lockstep("0.1.10")

    def test_npm_suffix_normalized(self):
        # frontend ships 0.1.9+12513254; stores must normalize it to bare
        self.bump_json("patch")
        self.assert_lockstep("0.1.10")

    def test_regular_bumps(self):
        self.bump_json("patch")
        self.assert_lockstep("0.1.10")
        self.bump_json("minor")
        self.assert_lockstep("0.2.0")
        self.bump_json("major")
        self.assert_lockstep("1.0.0")

    def test_update_strips_suffix(self):
        self.run_checked("update", "0.2.0-rc.1")
        self.assert_lockstep("0.2.0")

    def test_invalid_version_fails(self):
        result = self.run_manager("update", "not-semver")
        self.assertNotEqual(result.returncode, 0)
        npm = json.loads((self.root / "frontend" / "package.json").read_text())
        self.assertEqual(npm["version"], "0.1.9+12513254")
        for name in ("clover2", "clover2_extra"):
            text = (self.root / name / "package.xml").read_text()
            self.assertIn("<version>0.1.9</version>", text)

    # // rc series driven by git tags

    def test_rc_cut_advances_and_increments(self):
        self.init_git_with_stable_tag()

        # cut: writes bare base into stores, rc.1 is tag-only
        payload = self.bump_json("rc")
        self.assertEqual(payload, {"base": "0.2.0", "tag": "v0.2.0-rc.1",
                                   "stores_changed": True})
        self.assert_lockstep("0.2.0")

        # idempotent before the tag is created
        payload = self.bump_json("rc")
        self.assertEqual(payload, {"base": "0.2.0", "tag": "v0.2.0-rc.1",
                                   "stores_changed": False})

        self.git("add", "-A")
        self.git("commit", "-qm", "release: v0.2.0-rc.1")
        self.git("tag", "-a", "v0.2.0-rc.1", "-m", "rc")

        # next rc: tag-only operation, stores untouched
        payload = self.bump_json("rc")
        self.assertEqual(payload, {"base": "0.2.0", "tag": "v0.2.0-rc.2",
                                   "stores_changed": False})
        self.assert_lockstep("0.2.0")

    def test_rc_base_field(self):
        self.init_git_with_stable_tag()
        payload = self.bump_json("rc", "--base", "major")
        self.assertEqual(payload["base"], "1.0.0")

    def test_full_release_cycle(self):
        self.init_git_with_stable_tag()

        self.bump_json("rc")
        self.git("add", "-A")
        self.git("commit", "-qm", "release: v0.2.0-rc.1")
        self.git("tag", "-a", "v0.2.0-rc.1", "-m", "rc")
        self.git("commit", "--allow-empty", "-qm", "fix during rc")
        self.git("tag", "-a", "v0.2.0-rc.2", "-m", "rc")

        # promote: stable tag goes on the rc commit, stores untouched
        rc_info = self.compose_json("--latest-rc")
        self.assertEqual(rc_info["tag"], "v0.2.0-rc.2")
        self.assertEqual(rc_info["version"], "0.2.0")
        self.git("tag", "-a", "v0.2.0", "-m", "release", rc_info["commit"])

        self.assertEqual(self.compose_json("--latest-stable")["tag"], "v0.2.0")
        self.assertEqual(self.compose_json("--ref", "refs/tags/v0.2.0")["version"],
                         "0.2.0")

        # next regular release bumps from the promoted base
        payload = self.bump_json("patch")
        self.assertEqual(payload["base"], "0.2.1")
        self.assert_lockstep("0.2.1")

    # // compose

    def test_compose_from_tag_ref(self):
        self.init_git_with_stable_tag()
        payload = self.compose_json("--ref", "refs/tags/v0.1.9")
        self.assertEqual(payload["build_mode"], "release")
        self.assertEqual(payload["version"], "0.1.9")
        self.assertEqual(payload["base_version"], "0.1.9")

    def test_compose_from_rc_tag_ref(self):
        self.init_git_with_stable_tag()
        self.bump_json("rc")
        self.git("add", "-A")
        self.git("commit", "-qm", "release: v0.2.0-rc.1")
        self.git("tag", "-a", "v0.2.0-rc.1", "-m", "rc")

        payload = self.compose_json("--ref", "refs/tags/v0.2.0-rc.1")
        self.assertEqual(payload["build_mode"], "pre-release")
        self.assertEqual(payload["version"], "0.2.0-rc.1")

    def test_compose_mode_develop_appends_hash(self):
        self.init_git_with_stable_tag()
        out = self.run_checked("compose", "--mode", "develop", "--field", "version")
        self.assertRegex(out, r"^0\.1\.9\+[0-9a-f]{7,}$")

    def test_compose_mode_release_uses_head_tag(self):
        self.init_git_with_stable_tag()
        out = self.run_checked("compose", "--mode", "release", "--field", "version")
        self.assertEqual(out, "0.1.9")

    def test_compose_mode_pre_release_without_tag_falls_back_to_bare(self):
        self.init_git_with_stable_tag()
        out = self.run_checked("compose", "--mode", "pre-release", "--field", "version")
        self.assertEqual(out, "0.1.9")

    def test_compose_unknown_tag_ref_fails(self):
        self.init_git_with_stable_tag()
        result = self.run_manager("compose", "--ref", "refs/tags/v9.9.9")
        self.assertNotEqual(result.returncode, 0)

    def test_compose_latest_stable_ignores_rc_tags(self):
        self.init_git_with_stable_tag()
        self.bump_json("rc")
        self.git("add", "-A")
        self.git("commit", "-qm", "release: v0.2.0-rc.1")
        self.git("tag", "-a", "v0.2.0-rc.1", "-m", "rc")
        self.assertEqual(self.compose_json("--latest-stable")["tag"], "v0.1.9")


if __name__ == "__main__":
    unittest.main()
