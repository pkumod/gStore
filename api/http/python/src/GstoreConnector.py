"""
# Filename: GStoreConnector.py
# Author: yangchaofan
# Last Modified: 2018-7-18 14:44
# Description: http api for python
"""

import requests

defaultServerIP = "127.0.0.1"
defaultServerPort = "3305"

class GstoreConnector:
    def __init__(self, ip, port):
        if (ip == "localhost"):
            self.serverIP = defaultServerIP
        else:
            self.serverIP = ip
        self.serverPort = port
        self.Url = "http://" + self.serverIP + ":" + str(self.serverPort)
   
    def UrlEncode(self, s):
        ret = ""
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
            elif (ord(c)>=256):
                ret += chr(ord(c))
            elif ((ord(c)!=9) and (ord(c)!=10) and (ord(c)!=13)):
                ret += "{}{:X}".format("%", ord(c))
        return ret

    def Get(self, strUrl):
        r = requests.get(self.UrlEncode(strUrl))
        return r.text

    def fGet(self, strUrl, filename):
        r = requests.get(self.UrlEncode(strUrl), stream=True)
        with open(filename, 'wb') as fd:
            for chunk in r.iter_content(4096):
                fd.write(chunk)
        return

    def load(self, db_name, username, password):
        cmd = self.Url + "/?operation=load&db_name=" + db_name + "&username=" + username + "&password=" + password
        res = self.Get(cmd)
        print(res)
        if res == "load database done.":
            return True
        return False
    
    def unload(self, db_name, username, password):
        cmd = self.Url + "/?operation=unload&db_name=" + db_name + "&username=" + username + "&password=" + password
        res = self.Get(cmd)
        print(res)
        if res == "unload database done.":
            return True
        return False

    def build(self, db_name, rdf_file_path, username, password):
        cmd = self.Url + "/?operation=build&db_name=" + db_name + "&ds_path=" + rdf_file_path + "&username=" + username + "&password=" + password
        res = self.Get(cmd)
        print(res)
        if res == "import RDF file to database done.":
            return True
        return False

    def query(self, username, password, db_name, sparql):
        cmd = self.Url + "/?operation=query&username=" + username + "&password=" + password + "&db_name=" + db_name + "&format=json&sparql=" + sparql
        return self.Get(cmd)

    def fquery(self, username, password, db_name, sparql, filename):
        cmd = self.Url + "/?operation=query&username=" + username + "&password=" + password + "&db_name=" + db_name + "&format=json&sparql=" + sparql
        self.fGet(cmd, filename)
        return

    def show(self):
        cmd = self.Url + "/?operation=show"
        return self.Get(cmd)

    def user(self, type, username1, password1, username2, addition):
        cmd = self.Url + "/?operation=user&type=" + type + "&username1=" + username1+ "&password1=" + password1 + "&username2=" + username2 + "&addition=" +addition
        return self.Get(cmd)

    def showUser(self):
        cmd = self.Url + "/?operation=showUser"
        return self.Get(cmd)

    def monitor(self, db_name):
        cmd = self.Url + "/?operation=monitor&db_name=" + db_name;
        return self.Get(cmd)
    
    def checkpoint(self, db_name):
        cmd = self.Url + "/?operation=checkpoint&db_name=" + db_name
        return self.Get(cmd)
