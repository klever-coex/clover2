"""Validate the installed description across all module combinations."""
import itertools
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from ament_index_python.packages import get_package_share_directory
import pytest
import xacro


SHARE = Path(get_package_share_directory('clover2_description'))
MODULES = ('main_camera', 'front_camera', 'lidar', 'led_strip')
LINKS = ('main_camera_link', 'front_camera_link', 'laser', 'led_strip_link')


def generate(values):
    return ET.fromstring(xacro.process_file(str(SHARE / 'xacro/klever5/klever5.xacro'), mappings={
        'enable_' + module: str(value).lower()
        for module, value in zip(MODULES, values)
    }).toxml())


@pytest.mark.parametrize('values', itertools.product((False, True), repeat=len(MODULES)))
def test_configurations(values):
    root = generate(values)
    names = [link.attrib['name'] for link in root.findall('link')]
    assert len(names) == len(set(names))
    joints = root.findall('joint')
    assert len({j.attrib['name'] for j in joints}) == len(joints)
    children = [j.find('child').attrib['link'] for j in joints]
    assert len(children) == len(set(children))
    assert set(names) - set(children) == {'base_link'}
    reached = {'base_link'}
    while True:
        expanded = reached | {j.find('child').attrib['link'] for j in joints
                              if j.find('parent').attrib['link'] in reached}
        if expanded == reached:
            break
        reached = expanded
    assert reached == set(names)
    for enabled, link in zip(values, LINKS):
        assert (link in names) == enabled
    assert len(names) == 5 + sum(n for n, enabled in zip((2, 2, 1, 1), values) if enabled)
    for mesh in root.findall('.//mesh'):
        uri = mesh.attrib['filename']
        assert uri.startswith('package://clover2_description/')
        assert (SHARE / uri.removeprefix('package://clover2_description/')).is_file()


def rotation(rpy):
    r, p, y = map(float, rpy.split())
    cr, sr = math.cos(r), math.sin(r)
    cp, sp = math.cos(p), math.sin(p)
    cy, sy = math.cos(y), math.sin(y)
    return [[cy*cp, cy*sp*sr-sy*cr, cy*sp*cr+sy*sr],
            [sy*cp, sy*sp*sr+cy*cr, sy*sp*cr-cy*sr], [-sp, cp*sr, cp*cr]]


def test_camera_frames():
    root = generate((True, True, False, False))
    config = xacro.load_yaml(str(SHARE / 'config/robots/klever5.yaml'))
    for name in ('main_camera', 'front_camera'):
        mount = root.find(f"joint[@name='{name}_mount_joint']/origin")
        optical = root.find(f"joint[@name='{name}_optical_joint']/origin")
        settings = config['modules'][name]
        assert mount.attrib['xyz'] == settings['xyz']
        assert mount.attrib['rpy'] == settings['rpy']
        # Optical Z points along housing X, optical X along -Y, optical Y along -Z.
        actual = rotation(optical.attrib['rpy'])
        expected = [[0, 0, 1], [-1, 0, 0], [0, -1, 0]]
        for row, target in zip(actual, expected):
            assert row == pytest.approx(target, abs=1e-12)
