import socket
import traceback


class GstoreConnector:
    def _connect(self):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._sock.connect((self.ip, self.port))
            return True
        except Exception as e:
            print('socket connection error. @GstoreConnector.connect')
            traceback.print_exc()
        return False

    def _disconnect(self):
        try:
            self._sock.close()
            return True
        except Exception as e:
            print('socket disconnection error. @GstoreConnector.disconnect')
            traceback.print_exc()
        return False

    def _send(self, msg):
        data = self._pack(msg)
        self._sock.send(data)
        return True

    def _recv(self):
        head = self._sock.recv(4)
        context_len = 0
        for i in range(4):
            # context_len |= (ord(head[i].decode('utf-8')) & 0xFF) << i * 8
            context_len |= (head[i] & 0xFF) << i * 8

        data = bytearray()
        recv_len = 0
        while recv_len < context_len:
            chunk = self._sock.recv(context_len - recv_len)
            data.extend(chunk)
            recv_len += len(chunk)
            # .rstrip('\x00')
        return data.decode('utf-8')

    def _pack(self, msg):
        data_context = bytearray()
        data_context.extend(msg.encode(encoding='utf-8'))
        context_len = len(data_context) + 1
        data_len = context_len + 4

        data = bytearray(data_len)
        for i in range(4):
            # data[i] = chr((context_len >> i * 8) & 0xFF).encode('utf-8')
            data[i] = (context_len >> i * 8) & 0xFF
        for i, _ in enumerate(data_context):
            data[i + 4] = data_context[i]
        data[data_len - 1] = 0
        return data

    def _communicate(f):
        def wrapper(self, *args, **kwargs):
            if not self._connect():
                print('connect to server error. @GstoreConnector.%s' % f.__name__)
                return False

            if f.__name__ == 'build':
                cmd = 'import'
            elif f.__name__ == 'show':
                if args[0]:
                    cmd = 'show all'
                else:
                    cmd = 'show databases'
            else:
                cmd = f.__name__
            if f.__name__ == 'query':
                # DEBUG:only sparql query should be used, output should not be included in parameters
                print("the first argument:")
                print(args[0])
                # print args[1]
                params = ' '.join([args[0]])
            else:
                params = ' '.join(map(lambda x: str(x), args))
            full_cmd = ' '.join([
                cmd,
                params
            ]).strip()
            print("command to send to server:")
            print(full_cmd)

            if not self._send(full_cmd):
                print('send %s command error. @GstoreConnector.build' % cmd)
                return False

            recv_msg = self._recv()
            self._disconnect()

            succ = {
                'test': 'OK',
                'load': 'load database done.',
                'unload': 'unload database done.',
                'import': 'import RDF file to database done.',
                'drop': 'drop database done.',
                'stop': 'server stopped.',
                'query': None,
                'show all': None,
                'show databases': None,
            }
            if cmd in succ:
                if succ[cmd] == recv_msg:
                    return True
                else:
                    return recv_msg
            return False

        return wrapper

    def __init__(self, ip='127.0.0.1', port=3305):
        self.ip = ip
        self.port = port

    @_communicate
    def test(self):
        pass

    @_communicate
    def load(self, db_name):
        pass

    @_communicate
    def unload(self, db_name):
        pass

    @_communicate
    def build(self, db_name, rdf_file_path):
        pass

    @_communicate
    def drop(self, db_name):
        pass

    @_communicate
    def query(self, sparql, output='/'):
        pass

    @_communicate
    def show(self, _type=False):
        pass
