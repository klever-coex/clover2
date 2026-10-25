"""Random string generator."""

import random
import string


def generate_random_string(length: int) -> str:
    """Generate a random alphanumeric string."""
    characters = string.ascii_letters + string.digits
    random_string = "".join(random.choices(characters, k=length))
    return random_string
