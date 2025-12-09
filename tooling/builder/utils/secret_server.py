#!/usr/bin/env python3

import os
from flask import Flask

class SecretServer:
    def __init__(self, name):
        self.app = Flask(name)
        self._setup_routes()

    def _setup_routes(self):
        @self.app.route('/secret/<key>')
        def index(key):
            return os.environ.get(key)

    def run(self, host='0.0.0.0', port=3000, debug=True):
        self.app.run(host=host, port=port, debug=debug)

if __name__ == '__main__':
    server = SecretServer(__name__)
    server.run()
