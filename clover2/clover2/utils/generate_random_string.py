import random
import string


def generate_random_string(length: int) -> str:

    characters = string.ascii_letters + string.digits
    random_string = "".join(random.choices(characters, k=length))
    return random_string
