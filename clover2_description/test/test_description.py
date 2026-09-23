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


def generate_sdf(config_file=None, values=None):
    mappings = {'description_share': str(SHARE)}
    if values is not None:
        mappings.update({'enable_' + m: str(v).lower()
                         for m, v in zip(MODULES, values)})
    if config_file is not None:
        mappings['config_file'] = str(config_file)
    return ET.fromstring(xacro.process_file(
        str(SHARE / 'gazebo/klever5/klever5.sdf.xacro'), mappings=mappings).toxml())


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


@pytest.mark.parametrize('values', itertools.product((False, True), repeat=len(MODULES)))
def test_physics(values):
    root = generate(values)
    _, camera, lidar, led = values
    config = yaml.safe_load((SHARE / 'config/robots/klever5.yaml').read_text())
    body_mass = config['body']['mass']
    propeller_mass = config['propeller']['mass']
    masses = [float(m.get('value')) for m in root.findall('link/inertial/mass')]
    expected_mass = body_mass + 4*propeller_mass + camera*.018 + lidar*.100 + led*.010
    assert sum(masses) == pytest.approx(expected_mass)
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


@pytest.mark.parametrize('values', itertools.product((False, True), repeat=len(MODULES)))
def test_sdf_modules_match_urdf(values):
    urdf = generate(values)
    sdf = generate_sdf(values=values)
    model = sdf.find('model')
    assert sum(float(v.get('value')) for v in urdf.findall('.//inertial/mass')) == \
        pytest.approx(sum(float(v.text) for v in sdf.findall('.//inertial/mass')))
    sensors = {v.get('name') for v in sdf.findall('.//sensor')}
    for enabled, name in zip(values[:3], ('main_camera', 'front_camera', 'lidar')):
        assert (name in sensors) == enabled
    for enabled, name in zip(values, LINKS):
        assert (model.find(f"link[@name='{name}']") is not None or
                model.find(f"frame[@name='{name}']") is not None) == enabled
    for mesh in sdf.findall('.//mesh/uri'):
        assert mesh.text.startswith('file://')
        assert Path(mesh.text.removeprefix('file://')).is_file()
    for joint in urdf.findall('joint'):
        assert joint.get('type') == 'fixed'
    for joint in model.findall("joint[@type='revolute']"):
        assert joint.findtext('axis/xyz') == '0 0 1'
    for enabled, name, urdf_joint in zip(
            values, MODULES, ('main_camera_mount_joint', 'front_camera_mount_joint',
                              'laser_joint', 'led_strip_link_joint')):
        if not enabled:
            continue
        link = {'main_camera': 'main_camera_body_link',
                'front_camera': 'front_camera_body_link', 'lidar': 'laser',
                'led_strip': 'led_strip_link'}[name]
        element = model.find(f"link[@name='{link}']")
        if element is None:
            element = model.find(f"frame[@name='{link}']")
        origin = urdf.find(f"joint[@name='{urdf_joint}']/origin")
        assert element.findtext('pose') == origin.get('xyz') + ' ' + origin.get('rpy')
