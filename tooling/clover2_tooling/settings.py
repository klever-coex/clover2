from typing import TYPE_CHECKING, Any, Mapping

import click

from clover2_tooling.ros import exec_ros, package_share

if TYPE_CHECKING:
    from clover2_dev.plugins import PluginContext

DEFAULT_SCHEMA = "src/clover2/clover2_bringup/schemas/klever5.yaml"
SCHEMA_PACKAGE = "clover2_bringup"
DEFAULT_CONFIG = "config.yaml"


def resolve(project_root, explicit, configured: str):
    """Subcommand flag > [plugins.<id>] value; both relative to project root."""
    import pathlib

    if explicit is not None:
        return pathlib.Path(explicit)
    base = project_root if project_root is not None else pathlib.Path.cwd()
    return base / configured


def resolve_schema(project_root, explicit, configured, find_share=package_share):
    """Schema resolution: flag > the schema installed with clover2_bringup
    (primary source — it is guaranteed to match the built TUI node, and works
    from any directory in a sourced ROS environment) > [plugins.settings]
    schema (project-relative) > project default."""
    import pathlib

    if explicit is not None:
        return pathlib.Path(explicit)

    share = find_share(SCHEMA_PACKAGE)
    if share is not None:
        installed = share / "schemas" / "klever5.yaml"
        if installed.is_file():
            return installed

    base = project_root if project_root is not None else pathlib.Path.cwd()
    project_default = base / (configured or DEFAULT_SCHEMA)
    if project_default.is_file():
        return project_default

    return project_default


def build_argv(schema, config) -> list[str]:
    return ["run", "clover2_ui", "settings", str(schema), str(config)]


class SettingsPlugin:
    name = "settings"

    def create_commands(self, ctx: "PluginContext") -> list[click.Command]:
        plugin_config: Mapping[str, Any] = ctx.config
        app_root = ctx.root

        def resolve_opt(explicit, key: str, default: str):
            import pathlib

            if explicit is not None:
                return pathlib.Path(explicit)
            base = app_root if app_root is not None else pathlib.Path.cwd()
            return base / plugin_config.get(key, default)

        @click.command(
            name="settings",
            context_settings={"ignore_unknown_options": True},
            help="Run the clover2_ui settings TUI node")
        @click.option("--schema", "schema_",
                      type=click.Path(exists=True, dir_okay=False,
                                      path_type=str),
                      help="Settings schema YAML (default: project checkout, "
                           "or the schema installed with clover2_bringup)")
        @click.option("--config", "config_",
                      type=click.Path(dir_okay=False, path_type=str),
                      help="Config YAML edited by the TUI "
                           f"(default: {DEFAULT_CONFIG} in the project)")
        @click.argument("node_args", nargs=-1, type=click.UNPROCESSED)
        def settings(schema_, config_, node_args: tuple[str, ...]) -> None:
            schema_path = resolve_schema(app_root, schema_,
                                         plugin_config.get("schema"))
            config_path = resolve_opt(config_, "config", DEFAULT_CONFIG)
            if not schema_path.exists():
                raise click.ClickException(
                    f"Schema not found: {schema_path} (pass --schema, run from "
                    "the clover2 project, or source the ROS environment with "
                    "clover2_bringup installed)")

            exec_ros(build_argv(schema_path, config_path) + list(node_args))

        return [settings]


plugin = SettingsPlugin()
