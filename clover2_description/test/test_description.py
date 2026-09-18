"""Validate the installed description across all module combinations."""
import itertools
import math
from pathlib import Path
import xml.etree.ElementTree as ET

from ament_index_python.packages import get_package_share_directory
import pytest
import xacro
import yaml


SHARE = Path(get_package_share_directory('clover2_description'))
MODULES = ('main_camera', 'front_camera', 'lidar', 'led_strip')
LINKS = ('main_camera_link', 'front_camera_link', 'laser', 'led_strip_link')


def generate(values, config_file=None):
    mappings = {
        'enable_' + module: str(value).lower()
        for module, value in zip(MODULES, values)
    }
    if config_file is not None:
        mappings['config_file'] = str(config_file)
    return ET.fromstring(xacro.process_file(
        str(SHARE / 'xacro/klever5/klever5.xacro'), mappings=mappings).toxml())


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


def test_camera_geometry_and_names():
    root = generate((True, True, False, False))
    assert root.find("link[@name='main_camera_body_link']/visual") is None
    assert root.find("link[@name='front_camera_body_link']/visual/geometry/mesh") is not None
    for name in ('main_camera', 'front_camera'):
        assert root.find(f"link[@name='{name}_optical_frame']") is None
        joint = root.find(f"joint[@name='{name}_optical_joint']")
        assert joint.find('parent').get('link') == name + '_body_link'
        assert joint.find('child').get('link') == name + '_link'


@pytest.mark.parametrize('values', itertools.product((False, True), repeat=len(MODULES)))
def test_physics(values):
    root = generate(values)
    _, camera, lidar, led = values
    masses = [float(m.get('value')) for m in root.findall('link/inertial/mass')]
    assert sum(masses) == pytest.approx(.720 + 4*.003 + camera*.018 + lidar*.100 + led*.010)
    for link in root.findall('link'):
        inertial = link.find('inertial')
        if inertial is None:
            assert link.get('name') in ('main_camera_body_link', 'main_camera_link',
                                        'front_camera_link')
            continue
        assert float(inertial.find('mass').get('value')) > 0
        inertia = inertial.find('inertia')
        diagonal = [float(inertia.get(k)) for k in ('ixx', 'iyy', 'izz')]
        assert all(math.isfinite(v) and v > 0 for v in diagonal)
        assert 2*max(diagonal) <= sum(diagonal) + 1e-12
        size = list(map(float, link.find('collision/geometry/box').get('size').split()))
        assert all(v > 0 for v in size)
    # Internal camera and optical frames never add mass already counted in the body.
    assert root.find("link[@name='main_camera_link']/inertial") is None


@pytest.mark.parametrize('module', MODULES)
def test_external_mount_config(module, tmp_path):
    config = yaml.safe_load((SHARE / 'config/robots/klever5.yaml').read_text())
    assert set(config) == {'modules'}
    assert all(set(settings) == {'xyz', 'rpy'} for settings in config['modules'].values())
    config['modules'][module] = {'xyz': '0.2 -0.1 0.3', 'rpy': '0.1 0.2 -0.3'}
    path = tmp_path / 'custom mounts.yaml'
    path.write_text(yaml.safe_dump(config))
    baseline = generate((True, True, True, True))
    modified = generate((True, True, True, True), config_file=path)
    names = {'main_camera': 'main_camera_mount_joint',
             'front_camera': 'front_camera_mount_joint',
             'lidar': 'laser_joint', 'led_strip': 'led_strip_link_joint'}
    original = baseline.find(f"joint[@name='{names[module]}']/origin")
    changed = modified.find(f"joint[@name='{names[module]}']/origin")
    assert changed.attrib == config['modules'][module]
    changed.attrib.clear()
    changed.attrib.update(original.attrib)
    # Every other transform, visual, collision and physical property stays identical.
    assert ET.tostring(modified) == ET.tostring(baseline)
