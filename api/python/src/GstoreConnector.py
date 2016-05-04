# coding: utf-8
# zhangxiaoyang.hit#gmail.com
# github.com/zhangxiaoyang

import socket
import traceback

class GstoreConnector:

    def _connect(self):
        try:
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
            self._sock.connect((self.ip, self.port))  
            return True
        except Exception, e:
            print 'socket connection error. @GstoreConnector.connect'
            traceback.print_exc()
        return False

    def _disconnect(self):
        try:
            self._sock.close()  
            return True
        except Exception, e:
            print 'socket disconnection error. @GstoreConnector.disconnect'
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
            context_len |= (ord(head[i]) << i * 8)

        data = bytearray(context_len)
        recv_len = 0
        while recv_len < context_len:
            chunk = self._sock.recv(1024)
            data.extend(chunk)
            recv_len += len(chunk)
        return str(data)

    def _pack(self, msg):
        data_context = bytearray()
        data_context.extend(msg)
        context_len = len(data_context) + 1
        data_len = context_len + 4

        data = bytearray(data_len)
        for i in range(4):
            data[i] = chr((context_len >> i * 8) & 0xFF)
        for i, _ in enumerate(data_context):
            data[i + 4] = data_context[i]
        data[data_len - 1] = 0
        return data

    def _communicate(f):
        def wrapper(self, *args, **kwargs):
            if not self._connect():
                print 'connect to server error. @GstoreConnector.%s' % f.__name__
                return False

            if f.__name__ == 'build':
                cmd = 'import'
            elif f.__name__ == 'show':
                cmd = 'show databases'
            else:
                cmd = f.__name__
            params = ' '.join(map(lambda x:str(x), args))
            full_cmd = ' '.join([
                cmd,
                params
            ]).strip()

            if not self._send(full_cmd):
                print 'send %s command error. @GstoreConnector.build' % cmd
                return False

            recv_msg = self._recv()
            self._disconnect()

            succ = {
                'load': 'load database done.',
                'unload': 'unload database done.',
                'import': 'import RDF file to database done.',
                'query': None,
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
    def load(self, db_name):
        pass

    @_communicate
    def unload(self, db_name):
        pass

    @_communicate
    def build(self, db_name, rdf_file_path):
        pass

    @_communicate
    def query(self, sparql):
        pass

    @_communicate
    def show(self):
        pass
