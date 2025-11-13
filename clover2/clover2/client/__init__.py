from clover2.client.client_base import ClientBase, ClientType
from clover2.client.client_mavros import ClientMavros


def Client(client_type: ClientType, *args, **kwargs) -> ClientBase:
    for cls in ClientBase.__subclasses__():
        if cls._type == client_type:
            return cls(*args, **kwargs)
    raise ValueError
