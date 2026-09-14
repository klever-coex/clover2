import logging
import pathlib

from minio import Minio
from minio.commonconfig import Tags

from clover2_tooling.builder.config import ImageConfiguration
from clover2_tooling.builder.settings import BuilderSettings

logger = logging.getLogger(__name__)

BUCKET = "clover2"


def _client(settings: BuilderSettings) -> Minio:
    endpoint = settings.minio_endpoint
    if "://" in endpoint:
        secure, endpoint = endpoint.startswith(
            "https://"), endpoint.split("://", 1)[1]
    else:
        secure = settings.minio_secure
    return Minio(endpoint, access_key=settings.minio_access_key,
                 secret_key=settings.minio_secret_key, secure=secure)


def upload(artifact: pathlib.Path, config: ImageConfiguration,
           channel: str, settings: BuilderSettings) -> str:
    if not settings.minio_endpoint:
        raise RuntimeError("MinIO endpoint is not set (MINIO_ENDPOINT)")

    client = _client(settings)

    key = f"{config.name}/{channel}/{artifact.name}"

    image_tags = Tags(for_object=True)
    image_tags["platform"] = config.name
    image_tags["build-type"] = settings.build_mode
    image_tags["git-hash"] = settings.composed_version()["git_hash"]

    logger.info("Uploading '%s' -> %s/%s", artifact, BUCKET, key)
    client.fput_object(BUCKET, key, artifact, tags=image_tags)
    logger.info("Uploaded: %s/%s", BUCKET, key)

    return key
