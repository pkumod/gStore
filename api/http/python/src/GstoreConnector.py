"""
# Filename: GStoreConnector.py
# Author: yangchaofan suxunbin
# Last Modified: 2019-5-15 18:10
# Description: http api for python
"""

import sys
import requests

version = sys.version[0]
if version == '3':
    from urllib import parse

defaultServerIP = "127.0.0.1"
defaultServerPort = "9000"

class GstoreConnector:
    def __init__(self, ip, port, httpType, username, password):
        if (ip == "localhost"):
            self.serverIP = defaultServerIP
        else:
            self.serverIP = ip
        self.serverPort = port
        self.Url = "http://" + self.serverIP + ":" + str(self.serverPort) + "/"
        if (httpType == "grpc"):
            self.Url += "grpc/api/"
        self.username = username
        self.password = password
   
    def UrlEncode(self, s):
        ret = ""
        if version == '2':
            for i in range(len(s)):
                c = s[i]
                if ((ord(c)==42) or (ord(c)==45) or (ord(c)==46) or (ord(c)==47) or (ord(c)==58) or (ord(c)==95)):
                    ret += c
                elif ((ord(c)>=48) and (ord(c)<=57)):
                    ret += c
                elif ((ord(c)>=65) and (ord(c)<=90)):
                    ret += c
                elif ((ord(c)>=97) and (ord(c)<=122)):
                    ret += c
                elif (ord(c)==32):
                    ret += '+'
                elif ((ord(c)!=9) and (ord(c)!=10) and (ord(c)!=13)):
                    ret += "{}{:X}".format("%", ord(c))
        elif version == '3':
            ret = parse.quote(s)
        return ret

    def Get(self, strUrl):
        r = requests.get(self.Url + self.UrlEncode(strUrl))
        return r.text

    def Post(self, strPost):
        r = requests.post(self.Url, strPost)
        return r.text

    def fGet(self, strUrl, filename):
        r = requests.get(self.Url + self.UrlEncode(strUrl), stream=True)
        with open(filename, 'wb') as fd:
            for chunk in r.iter_content(4096):
                fd.write(chunk)
        return

    def fPost(self, strPost, filename):
        r = requests.post(self.Url + strPost, stream=True)
        with open(filename, 'wb') as fd:
            for chunk in r.iter_content(4096):
                fd.write(chunk)
        return

    def build(self, db_name, db_path, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"build\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res

    def check(self, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=check"
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"check\"}'
            res = self.Post(strPost)
        return res

    def load(self, db_name, csr, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"load\", \"db_name\": \"' + db_name + '\", \"csr\": \"' + csr + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res
  
    def monitor(self, db_name, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=monitor&db_name=" + db_name + "&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"monitor\", \"db_name\": \"' + db_name + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res
  
    def unload(self, db_name, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=unload&db_name=" + db_name + "&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"unload\", \"db_name\": \"' + db_name + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res

    def drop(self, db_name, is_backup, request_type='GET'):
        if request_type == 'GET':      
            if is_backup:  
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + self.username + "&password=" + self.password + "&is_backup=true"
            else:  
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + self.username + "&password=" + self.password + "&is_backup=false"
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            if is_backup: 
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"is_backup\": \"true\"}'
            else: 
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"is_backup\": \"false\"}'
            res = self.Post(strPost)
        return res

    def show(self, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=show&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"show\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res

    def usermanage(self, type, op_username, op_password, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=usermanage&type=" + type + "&username=" + self.username + "&password=" + self.password + "&op_username=" + op_username + "&op_password=" + op_password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"usermanage\", \"type\": \"' + type + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"op_username\": \"' + op_username + '\", \"op_password\": \"' + op_password + '\"}'
            res = self.Post(strPost)
        return res

    def showuser(self, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=showuser&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"showuser\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res

    def userprivilegemanage(self, type, op_username, privileges, db_name, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=userprivilegemanage&type=" + type + "&username=" + self.username + "&password=" + self.password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"userprivilegemanage\", \"type\": \"' + type + '\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"op_username\": \"' + op_username + '\", \"privileges\": \"' + privileges + '\", \"db_name\": \"' + db_name + '\"}'
            res = self.Post(strPost)
        return res

    def backup(self, db_name, backup_path, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=backup&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&backup_path=" + backup_path
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"backup\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + '\"}'
            res = self.Post(strPost)
        return res

    def restore(self, db_name, backup_path, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=restore&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&backup_path=" + backup_path
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"restore\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + '\"}'
            res = self.Post(strPost)
        return res

    def query(self, db_name, format, sparql, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=query&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"query\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\"}'
            res = self.Post(strPost)
        return res

    def fquery(self, db_name, format, sparql, filename, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=query&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql
            self.fGet(strUrl, filename)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"query\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\"}'
            self.fPost(strPost, filename)
        return

    def exportDB(self, db_name, db_path, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=export&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&db_path=" + db_path
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"export\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + '\"}'
            res = self.Post(strPost)
        return res

    def login(self, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=login&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"login\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res

    def begin(self, db_name, isolevel, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=begin&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&isolevel=" + isolevel
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"begin\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"isolevel\": \"' + isolevel + '\"}'
            res = self.Post(strPost)
        return res

    def tquery(self, db_name, tid, sparql, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=tquery&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"tquery\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\", \"sparql\": \"' + sparql + '\"}'
            res = self.Post(strPost)
        return res

    def commit(self, db_name, tid, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=commit&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&tid=" + tid
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"commit\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}'
            res = self.Post(strPost)
        return res

    def rollback(self, db_name, tid, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=rollback&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name + "&tid=" + tid
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"rollback\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}'
            res = self.Post(strPost)
        return res

    def getTransLog(self, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=txnlog&username=" + self.username + "&password=" + self.password
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"txnlog\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\"}'
            res = self.Post(strPost)
        return res
        
    def checkpoint(self, db_name, request_type='GET'):
        if request_type == 'GET':        
            strUrl = "?operation=checkpoint&username=" + self.username + "&password=" + self.password + "&db_name=" + db_name
            res = self.Get(strUrl)
        elif request_type == 'POST':        
            strPost = '{\"operation\": \"checkpoint\", \"username\": \"' + self.username + '\", \"password\": \"' + self.password + '\", \"db_name\": \"' + db_name + '\"}'
            res = self.Post(strPost)
        return res