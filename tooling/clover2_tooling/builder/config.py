from dataclasses import dataclass


@dataclass(frozen=True)
class ImageConfiguration:
    name: str
    base_image_url: str
    size: str = "14G"
    arch: str = "arm64"
    compression: str = "zip"
    docker_images: tuple[str, ...] = ()


image_configurations: dict[str, ImageConfiguration] = {
    "klever5": ImageConfiguration(
        name="klever5",
        base_image_url="https://cdimage.ubuntu.com/releases/24.04/release/ubuntu-24.04.4-preinstalled-server-arm64+raspi.img.xz",
        docker_images=("clover2-docs", "clover2-frontend"),
    ),
}


def get_configuration(name: str) -> ImageConfiguration:
    try:
        return image_configurations[name]
    except KeyError:
        raise RuntimeError(
            f"Unknown configuration '{name}'; available: {', '.join(image_configurations)}"
        ) from None
