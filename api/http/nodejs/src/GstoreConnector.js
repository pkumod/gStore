/*
# Filename: GstoreConnector.js
# Author: suxunbin
# Last Modified: 2019-5-20 14:10
# Description: http api for nodejs
*/

const request = require('request-promise');

class GstoreConnector {
    constructor(ip = '', port, httpType = 'ghttp', username = '', password = '') 
    {
        if (ip == 'localhost')
            this.serverIP = '127.0.0.1';
        else
            this.serverIP = ip;
        this.serverPort = port;
        this.Url = 'http://' + this.serverIP + ':' + this.serverPort.toString() + '/';
        if (httpType == 'grpc')
            this.Url = this.Url + 'grpc/api/';
        this.username = username;
        this.password = password;
    }

    async Get(strUrl = '') 
    {
        strUrl = this.Url + encodeURIComponent(strUrl);
        const res = await request({uri: strUrl, method: 'GET', json: true});
        return res;
    }

    async Post(strPost = '') 
    {
        strUrl = this.Url;
        const res = await request({
            uri: strUrl, 
            method: 'POST', 
            json: true, 
            body: strPost,  
            headers:{
                'Content-Type':'application/x-www-form-urlencoded',
                'Content-Length':strPost.length
            }
        });
        return res;
    }

    async build(db_name = '', db_path = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"build\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async check(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=check";
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"check\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async load(db_name = '', csr = '0', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"load\", \"db_name\": \"' + db_name + '\", \"csr\": \"' + csr + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async monitor(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=monitor&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"monitor\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async unload(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=unload&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"unload\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async drop(db_name = '', is_backup , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            var strUrl;
            if(is_backup)
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=true";
            else
                strUrl = "?operation=drop&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password + "&is_backup=false";
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            var strPost;
            if(is_backup)
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"true\"}';
            else
                strPost = '{\"operation\": \"drop\", \"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"false\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async show(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=show&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"show\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async usermanage(type = '', op_username = '' , op_password = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=usermanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&op_password=" + op_password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"usermanage\", \"type\": \"' + type + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"op_username\": \"' + op_username + '\", \"op_password\": \"' + op_password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async showuser(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=showuser&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"showuser\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async userprivilegemanage(type = '', op_username = '' , privileges = '' , db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=userprivilegemanage&type=" + type + "&username=" + this.username + "&password=" + this.password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"userprivilegemanage\", \"type\": \"' + type + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"op_username\": \"' + op_username + '\", \"privileges\": \"' + privileges + '\", \"db_name\": \"' + db_name + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async backup(db_name = '', backup_path = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=backup&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&backup_path=" + backup_path;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"backup\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async restore(db_name = '', backup_path = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=restore&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&backup_path=" + backup_path;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"restore\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"backup_path\": \"' + backup_path + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async query(db_name = '', format = '' , sparql = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=query&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"query\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async exportDB(db_name = '', db_path = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=export&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&db_path=" + db_path;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"export\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"db_path\": \"' + db_path + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async login(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=login&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"login\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async begin(db_name = '', isolevel = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=begin&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&isolevel=" + isolevel;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"begin\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"isolevel\": \"' + isolevel + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async tquery(db_name = '', tid = '' , sparql = '' , request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=tquery&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"tquery\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\", \"sparql\": \"' + sparql + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async commit(db_name = '', tid = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=commit&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"commit\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async rollback(db_name = '', tid = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=rollback&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name + "&tid=" + tid;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"rollback\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"tid\": \"' + tid + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async getTransLog(request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=txnlog&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"txnlog\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }

    async checkpoint(db_name = '', request_type = 'GET') 
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=checkpoint&username=" + this.username + "&password=" + this.password + "&db_name=" + db_name;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strPost = '{\"operation\": \"checkpoint\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\"}';
            const res = this.Post(strPost);
            return res;
        }
    }
}

module.exports = GstoreConnector;
