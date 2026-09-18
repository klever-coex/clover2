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


def test_sdf_physics_matches_config():
    root = generate_sdf()
    config = yaml.safe_load((SHARE / 'config/robots/klever5.yaml').read_text())
    expected = {'base_link': config['body']}
    expected.update({f"rotor_{settings['gazebo_index']}": config['propeller']
                     for settings in config['rotors'].values()})
    for link_name, physics in expected.items():
        inertial = root.find(f".//link[@name='{link_name}']/inertial")
        assert float(inertial.findtext('mass')) == pytest.approx(physics['mass'])
        assert inertial.findtext('pose') == ' '.join(map(str, physics['com'])) + ' 0 0 0'
        size = physics['inertia_size']
        expected_inertia = (physics['mass'] * (size[1]**2 + size[2]**2) / 12,
                            physics['mass'] * (size[0]**2 + size[2]**2) / 12,
                            physics['mass'] * (size[0]**2 + size[1]**2) / 12)
        actual_inertia = tuple(float(inertial.findtext(f'inertia/{axis}'))
                               for axis in ('ixx', 'iyy', 'izz'))
        assert actual_inertia == pytest.approx(expected_inertia)


@pytest.mark.parametrize('module', MODULES)
def test_external_mount_config(module, tmp_path):
    config = yaml.safe_load((SHARE / 'config/robots/klever5.yaml').read_text())
    assert set(config) == {'body', 'propeller', 'rotors', 'modules', 'simulation'}
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


@pytest.mark.parametrize('module', ('camera', 'lidar', 'led_strip'))
def test_standalone_module_twice(module):
    document = xacro.parse(f"""<robot name="test" xmlns:xacro="http://www.ros.org/wiki/xacro">
      <xacro:include filename="{SHARE}/xacro/modules/{module}.xacro"/>
      <link name="parent"/>
      <xacro:description_{module} name="first" parent="parent" xyz="0 0 0" rpy="0 0 0"/>
      <xacro:description_{module} name="second" parent="parent" xyz="1 0 0" rpy="0 0 0"/>
    </robot>""")
    xacro.process_doc(document)
    root = ET.fromstring(document.toxml())
    names = [v.get('name') for v in root.findall('link')]
    assert len(names) == len(set(names))
    assert len(root.findall('joint')) == (4 if module == 'camera' else 2)


def test_sdf_custom_sensors_and_motors(tmp_path):
    config = yaml.safe_load((SHARE / 'config/robots/klever5.yaml').read_text())
    config['simulation']['camera']['width'] = 320
    config['simulation']['lidar']['samples'] = 360
    config['simulation']['motor']['motorConstant'] = 1.0e-5
    config['modules']['main_camera']['xyz'] = '0.1 0.2 0.3'
    path = tmp_path / 'custom config.yaml'
    path.write_text(yaml.safe_dump(config))
    root = generate_sdf(path, (True, True, True, True))
    assert {c.text for c in root.findall('.//camera/image/width')} == {'320'}
    assert root.findtext('.//lidar/scan/horizontal/samples') == '360'
    assert {float(c.text) for c in root.findall('.//plugin/motorConstant')} == {1.0e-5}
    camera = root.find(".//sensor[@name='main_camera']")
    assert camera.findtext('pose').startswith('0.1 0.2 0.3 ')


def test_body_and_rotors_explicit_parameters():
    document = xacro.parse(f"""<robot name="test" xmlns:xacro="http://www.ros.org/wiki/xacro">
      <xacro:include filename="{SHARE}/xacro/klever5/body.xacro"/>
      <xacro:include filename="{SHARE}/xacro/klever5/propellers.xacro"/>
      <xacro:property name="settings"
        value="${{xacro.load_yaml('{SHARE}/config/robots/klever5.yaml')}}"/>
      <link name="root"/>
      <xacro:klever5_body body="${{settings['body']}}" name="first"/>
      <xacro:klever5_body body="${{settings['body']}}" name="second"/>
      <xacro:mount name="first_mount" parent="root" child="first" xyz="0 0 0" rpy="0 0 0"/>
      <xacro:mount name="second_mount" parent="root" child="second" xyz="1 0 0" rpy="0 0 0"/>
      <xacro:klever5_propellers propeller="${{settings['propeller']}}"
        rotors="${{settings['rotors']}}" parent="first" prefix="first_"/>
      <xacro:klever5_propellers propeller="${{settings['propeller']}}"
        rotors="${{settings['rotors']}}" parent="second" prefix="second_"/>
    </robot>""")
    xacro.process_doc(document)
    root = ET.fromstring(document.toxml())
    names = [v.get('name') for v in root.findall('link')]
    assert len(names) == len(set(names)) == 11
    parents = [v.get('link') for v in root.findall('joint/parent')]
    assert parents.count('first') == parents.count('second') == 4
    assert len(root.findall('joint')) == 10
