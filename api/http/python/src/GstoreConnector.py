import requests
from urllib import parse

defaultServerIP = "127.0.0.1"
defaultServerPort = "9000"


class GstoreConnector:
    def __init__(self, ip, port, username, password, http_type='ghttp'):
        if ip == "localhost":
            self.serverIP = defaultServerIP
        else:
            self.serverIP = ip
        self.serverPort = port
        self.base_url = "http://" + self.serverIP + ":" + str(self.serverPort) + "/"
        self.http_type = http_type
        if self.http_type == 'grpc':
            self.base_url += 'grpc/api'
        self.auth_params = {
            'username': username,
            'password': password
        }
        self.request = {"GET": self.get, "POST": self.post}

    def get(self, params, stream=False):
        if stream:
            return requests.get(self.base_url, params=params, stream=stream)
        else:
            return requests.get(self.base_url, params=parse.urlencode(params, quote_via=parse.quote), stream=stream).text

    def post(self, params, stream=False):
        if stream:
            return requests.post(self.base_url, json=params, stream=stream)
        else:
            return requests.post(self.base_url, json=params, stream=stream).text

    def save(self, filename, res_iter):
        with open(filename, 'wb') as fd:
            for chunk in res_iter.iter_content(4096):
                fd.write(chunk)
        return

    def build(self, db_name, db_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'build',
            'db_name': db_name,
            'db_path': db_path
        }
        return self.request[request_type](query_params)

    def check(self, request_type='GET'):
        query_params = {
            'operation': 'check'
        }
        return self.request[request_type](query_params)

    def load(self, db_name, csr, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'load',
            'db_name': db_name,
            'csr': csr
        }
        return self.request[request_type](query_params)

    def monitor(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'monitor',
            'db_name': db_name
        }
        return self.request[request_type](query_params)

    def unload(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'unload',
            'db_name': db_name
        }
        return self.request[request_type](query_params)

    def drop(self, db_name, is_backup, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'drop',
            'db_name': db_name,
            'is_backup': is_backup
        }
        return self.request[request_type](query_params)

    def show(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'show'
        }
        return self.request[request_type](query_params)

    def usermanage(self, type, op_username, op_password, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'usermanage',
            'type': type,
            'op_username': op_username,
            'op_password': op_password
        }
        return self.request[request_type](query_params)

    def showuser(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'showuser'
        }
        return self.request[request_type](query_params)

    def userprivilegemanage(self, type, op_username, privileges, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'userprivilegemanage',
            'type': type,
            'op_username': op_username,
            'privileges': privileges,
            'db_name': db_name
        }
        return self.request[request_type](query_params)

    def backup(self, db_name, backup_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'backup',
            'db_name': db_name,
            'backup_path': backup_path
        }
        return self.request[request_type](query_params)

    def restore(self, db_name, backup_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'restore',
            'db_name': db_name,
            'backup_path': backup_path
        }
        return self.request[request_type](query_params)

    def query(self, db_name, format, sparql, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'query',
            'db_name': db_name,
            'format': format,
            'sparql': sparql
        }
        return self.request[request_type](query_params)

    def fquery(self, db_name, format, sparql, filename, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'fquery',
            'db_name': db_name,
            'format': format,
            'sparql': sparql
        }
        r = self.request[request_type](query_params, stream=True)
        self.save(filename, r)
        return

    def exportDB(self, db_name, db_path, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'export',
            'db_name': db_name,
            'db_path': db_path
        }
        return self.request[request_type](query_params)

    def login(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'login'
        }
        return self.request[request_type](query_params)

    def begin(self, db_name, isolevel, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'begin',
            'db_name': db_name,
            'isolevel': isolevel
        }
        return self.request[request_type](query_params)

    def tquery(self, db_name, tid, sparql, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'tquery',
            'db_name': db_name,
            'tid': tid,
            'sparql': sparql
        }
        return self.request[request_type](query_params)

    def commit(self, db_name, tid, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'commit',
            'db_name': db_name,
            'tid': tid
        }
        return self.request[request_type](query_params)

    def rollback(self, db_name, tid, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'rollback',
            'db_name': db_name,
            'tid': tid
        }
        return self.request[request_type](query_params)

    def getTransLog(self, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'txnlog'
        }
        return self.request[request_type](query_params)

    def checkpoint(self, db_name, request_type='GET'):
        query_params = {
            **self.auth_params,
            'operation': 'checkpoint',
            'db_name': db_name
        }
        return self.request[request_type](query_params)
