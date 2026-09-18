from typing import TYPE_CHECKING, Any, Mapping

import click

from clover2_tooling.ros import exec_ros

if TYPE_CHECKING:
    from clover2_dev.plugins import PluginContext

DEFAULT_TOPIC = "/diagnostics_agg"


def build_argv(topic: str, sim_time: bool,
               node_args: tuple[str, ...]) -> list[str]:
    argv = ["run", "clover2_ui", "diagnostics", "--topic", topic]
    if sim_time:
        argv += ["--ros-args", "-p", "use_sim_time:=true"]
    return argv + list(node_args)


class DiagnosticsPlugin:
    name = "diagnostics"

    def create_commands(self, ctx: "PluginContext") -> list[click.Command]:
        plugin_config: Mapping[str, Any] = ctx.config

        @click.command(
            name="diagnostics",
            context_settings={"ignore_unknown_options": True},
            help="Run the clover2_ui diagnostics TUI node")
        @click.option("--topic", default=plugin_config.get(
            "topic", DEFAULT_TOPIC), show_default=True,
            help="Diagnostics topic to monitor")
        @click.option("--sim-time", is_flag=True,
                      help="Pass use_sim_time:=true to the node")
        @click.argument("node_args", nargs=-1, type=click.UNPROCESSED)
        def diagnostics(topic: str, sim_time: bool,
                        node_args: tuple[str, ...]) -> None:
            exec_ros(build_argv(topic, sim_time, node_args))

        return [diagnostics]


plugin = DiagnosticsPlugin()
