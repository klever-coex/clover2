import logging
import pathlib
import tarfile
import zipfile

logger = logging.getLogger(__name__)

CHUNK_SIZE = 8 * 1024 * 1024
PROGRESS_STEP = 1024 * 1024 * 1024


def _compress_zip(src: pathlib.Path, dst: pathlib.Path) -> None:
    with zipfile.ZipFile(dst, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=6, allowZip64=True) as zf:
        zinfo = zipfile.ZipInfo(src.name)
        zinfo.file_size = src.stat().st_size
        zinfo.compress_type = zipfile.ZIP_DEFLATED

        with zf.open(zinfo, "w") as target, open(src, "rb") as f:
            total = 0
            while chunk := f.read(CHUNK_SIZE):
                target.write(chunk)
                total += len(chunk)
                if total // PROGRESS_STEP != (total - len(chunk)) // PROGRESS_STEP:
                    logger.info("Compressed %d GiB...", total // PROGRESS_STEP)


def _compress_tgz(src: pathlib.Path, dst: pathlib.Path) -> None:
    with tarfile.open(dst, "w:gz") as tf:
        tf.add(src, arcname=src.name)


def _compress_txz(src: pathlib.Path, dst: pathlib.Path) -> None:
    with tarfile.open(dst, "w:xz") as tf:
        tf.add(src, arcname=src.name)


COMPRESSORS: dict[str, tuple[str, object]] = {
    "zip": (".zip", _compress_zip),
    "tar.gz": (".tar.gz", _compress_tgz),
    "tar.xz": (".tar.xz", _compress_txz),
}


def compress(src: pathlib.Path, fmt: str, dest_dir: pathlib.Path) -> pathlib.Path:
    try:
        ext, compressor = COMPRESSORS[fmt]
    except KeyError:
        raise RuntimeError(
            f"Unknown compression '{fmt}'; available: {', '.join(COMPRESSORS)}"
        ) from None

    dest_dir.mkdir(parents=True, exist_ok=True)
    dst = dest_dir / f"{src.stem}{ext}"
    logger.info("Compressing '%s' -> '%s' (%s)", src, dst, fmt)
    compressor(src, dst)
    logger.info("Compressed: %s", dst)
    return dst
