/*
# Filename: GstoreConnector.js
# Author: suxunbin
# Last Modified: 2019-5-20 14:10
# Description: http api for nodejs
*/

const request = require('request-promise');

class GstoreConnector {
    constructor(ip = '', port, username = '', password = '')
    {
        if (ip == 'localhost')
            this.serverIP = '127.0.0.1';
        else
            this.serverIP = ip;
        this.serverPort = port;
        this.Url = 'http://' + this.serverIP + ':' + this.serverPort.toString();
        this.username = username;
        this.password = password;
    }

    async Get(strUrl = '')
    {
        strUrl = this.Url + "/" + encodeURIComponent(strUrl);
        const res = await request({uri: strUrl, method: 'GET', json: true});
        return res;
    }

    async Post(strUrl = '', strPost = '')
    {
        strUrl = this.Url + "/" + encodeURIComponent(strUrl);
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

    async build(db_name = '', rdf_file_path = '', request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=build&db_name=" + db_name + "&ds_path=" + rdf_file_path + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "build";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"ds_path\": \"' + rdf_file_path + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async load(db_name = '', request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=load&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "load";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
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
            const strUrl = "unload";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async user(type = '', username2 = '' , addition = '' , request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=user&type=" + type + "&username1=" + this.username + "&password1=" + this.password + "&username2=" + username2 + "&addition=" +addition;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "user";
            const strPost = '{\"type\": \"' + type + '\", \"username1\": \"' + this.username + '\", \"password1\": \"' + this.password + '\", \"username2\": \"' + username2 + '\", \"addition\": \"' + addition + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async showUser(request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=showUser&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "showUser";
            const strPost = '{\"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
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
            const strUrl = "query";
            const strPost = '{\"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"db_name\": \"' + db_name + '\", \"format\": \"' + format + '\", \"sparql\": \"' + sparql + '\"}';
            const res = this.Post(strUrl, strPost);
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
            const strUrl = "drop";
            var strPost;
            if(is_backup)
                strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"true\"}';
            else
                strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\", \"is_backup\": \"false\"}';
            const res = this.Post(strUrl, strPost);
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
            const strUrl = "monitor";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async checkpoint(db_name = '', request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=checkpoint&db_name=" + db_name + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "checkpoint";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
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
            const strUrl = "show";
            const strPost = '{\"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async getCoreVersion(request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=getCoreVersion&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "getCoreVersion";
            const strPost = '{\"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async getAPIVersion(request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=getAPIVersion&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "getAPIVersion";
            const strPost = '{\"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }

    async exportDB(db_name = '', dir_path = '', request_type = 'GET')
    {
        if (request_type == 'GET')
        {
            const strUrl = "?operation=export&db_name=" + db_name + "&ds_path=" + dir_path + "&username=" + this.username + "&password=" + this.password;
            const res = this.Get(strUrl);
            return res;
        }
        else if (request_type == 'POST')
        {
            const strUrl = "export";
            const strPost = '{\"db_name\": \"' + db_name + '\", \"ds_path\": \"' + dir_path + '\", \"username\": \"' + this.username + '\", \"password\": \"' + this.password + '\"}';
            const res = this.Post(strUrl, strPost);
            return res;
        }
    }
}

module.exports = GstoreConnector;
